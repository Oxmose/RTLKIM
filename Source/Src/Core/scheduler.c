/***************************************************************************//**
 * @file scheduler.c
 * 
 * @see scheduler.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Kernel's thread scheduler.
 * 
 * @details Kernel's thread scheduler. Thread creation and management functions 
 * are located in this file.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/


#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Lib/string.h>           /* strncpy */
#include <Lib/stdlib.h>           /* uitoa */
#include <Memory/kheap.h>         /* kmalloc, kfree */
#include <Cpu/cpu.h>              /* cpu_hlt */
#include <Cpu/cpu_settings.h>     /* cpu_settings_set_tss_int_esp() */
#include <Interrupt/panic.h>      /* kernel_panic */
#include <Interrupt/interrupts.h> /* register_interrupt_handler,
                                   * set_IRQ_EOI, update_tick */
#include <IO/kernel_output.h>     /* kernel_success, kernel_error */
#include <IO/graphic.h>           /* save_color_scheme, $1set_color_scheme */
#include <Core/kernel_queue.h>    /* kernel_queue_t, kernel_queue_node_t */
#include <Time/time_management.h> /* time_get_current_uptime(), 
                                   * time_register_scheduler() */
#include <Sync/critical.h>        /* ENTER_CRITICAL, EXIT_CRITICAL */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Core/scheduler.h>

#if TEST_MODE_ENABLED == 1
#include <Tests/test_bank.h>
#endif

 /*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/** @brief The last TID given by the kernel. */
static volatile uint32_t last_given_tid;
/** @brief The number of thread in the system (dead threads are not accounted).
 */
static volatile uint32_t thread_count;
/** @brief First scheduling flag, tells if it's the first time we schedule a 
 * thread.
 */
static volatile uint32_t first_sched;

/** @brief Idle thread handle. */
static kernel_thread_t*     idle_thread;
/** @brief Idle thread queue node. */
static kernel_queue_node_t* idle_thread_node;

/** @brief Init thread handle. */
static kernel_thread_t*     init_thread;
/** @brief Init thread queue node. */
static kernel_queue_node_t* init_thread_node;

/** @brief Current active thread handle. */
static kernel_thread_t*     active_thread;
/** @brief Current active thread queue node. */
static kernel_queue_node_t* active_thread_node;
/** @brief Previously active thread handle. */
static kernel_thread_t*     prev_thread;
/** @brief Previously active thread queue node. */
static kernel_queue_node_t* prev_thread_node;

/** @brief Current system state. */
static volatile SYSTEM_STATE_E system_state;

/*******************************************************
 * THREAD TABLES
 * Sorted by priority:
 *     - sleeping_threads: thread wakeup time
 *
 * Global thread table used to browse the threads, even those
 * kept in a nutex / semaphore or other structure and that do
 * not appear in the three other tables.
 *
 *******************************************************/
/** @brief Active threads tables. The array is sorted by priority. */
static kernel_queue_t* active_threads_table[KERNEL_LOWEST_PRIORITY + 1];

/** @brief Sleeping threads table. The threads are sorted by their wakeup time
 * value.
 */
static kernel_queue_t* sleeping_threads_table;

/** @brief Zonbie threads table. */
static kernel_queue_t* zombie_threads_table;

/** @brief Global thread table. */
static kernel_queue_t* global_threads_table;

/** @brief Extern user programm entry point. */
extern int main(int, char**);

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Thread's exit point.
 * 
 * @details Exit point of a thread. The function will release the resources of 
 * the thread and manage its children (INIT will inherit them). Put the thread 
 * in a THREAD_STATE_ZOMBIE state. If an other thread is already joining the 
 * active thread, then the joining thread will switch from blocked to ready 
 * state.
 */
static void thread_exit(void)
{
    OS_RETURN_E          err;
    kernel_thread_t*     joining_thread = NULL;
    kernel_queue_node_t* node;
    uint32_t             word;

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Exit thread %d\n", active_thread->tid);
    #endif

    /* Cannot exit idle thread */
    if(active_thread == idle_thread)
    {
        kernel_error("Cannot exit idle thread[%d]\n",
                     OS_ERR_UNAUTHORIZED_ACTION);
        kernel_panic(err);
    }

    /* Set new thread state */
    active_thread->state =  THREAD_STATE_ZOMBIE;

    if(active_thread == init_thread)
    {
        /* Schedule thread, should never return since the state is zombie */
        sched_schedule();

        return;
    }

    ENTER_CRITICAL(word);

    /* Enqueue thread in zombie list. */
    err = kernel_queue_push(active_thread_node, zombie_threads_table);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not enqueue zombie thread[%d]\n", err);
        kernel_panic(err);
    }

    /* All the children of the thread are inherited by init */
    node = kernel_queue_pop(active_thread->children, &err);
    while(node != NULL && err == OS_NO_ERR)
    {
        kernel_thread_t* thread = (kernel_thread_t*)node->data;

        thread->ptid = init_thread->tid;

        if(thread->joining_thread->data == active_thread)
        {
            thread->joining_thread->data = NULL;
        }

        err = kernel_queue_push(node, init_thread->children);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could not enqueue thread to init[%d]\n", err);
            kernel_panic(err);
        }
        node = kernel_queue_pop(active_thread->children, &err);
    }
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not dequeue thread from children[%d]\n", err);
        kernel_panic(err);
    }

    /* Delete list */
    err = kernel_queue_delete_queue(&active_thread->children);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not delete lsit of children[%d]\n", err);
        kernel_panic(err);
    }

    /* Search for joining thread */
    if(active_thread->joining_thread != NULL)
    {
        joining_thread = (kernel_thread_t*)active_thread->joining_thread->data;
    }

    /* Wake up joining thread */
    if(joining_thread != NULL)
    {
        if(joining_thread->state == THREAD_STATE_JOINING)
        {
            #if SCHED_KERNEL_DEBUG == 1
            kernel_serial_debug("Woke up joining thread %d\n",
                joining_thread->tid);
            #endif

            joining_thread->state = THREAD_STATE_READY;

            err = kernel_queue_push(active_thread->joining_thread,
                                active_threads_table[joining_thread->priority]);
            if(err != OS_NO_ERR)
            {
                EXIT_CRITICAL(word);
                kernel_error("Could not enqueue joining thread[%d]\n", err);
                kernel_panic(err);
            }
        }
    }

    EXIT_CRITICAL(word);

    /* Schedule thread */
    sched_schedule();
}

/**
 * @brief Cleans a joined thread footprint in the system.
 * 
 * @details Cleans a thread that is currently being joined by the curent active 
 * thread. Removes the thread from all lists and cleans the lists nodes. 
 *
 * @param[in] thread The thread to clean.
 */
static void sched_clean_joined_thread(kernel_thread_t* thread)
{
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t             word;

    ENTER_CRITICAL(word);

    /* Remove node from children table */
    node = kernel_queue_find(active_thread->children, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not find joined thread in chlidren table[%d]\n",
                     err);
        kernel_panic(err);
    }

    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(active_thread->children, node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node in children table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Remove node from zombie table */
    node = kernel_queue_find(zombie_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not find joined thread in zombie table[%d]\n",
                     err);
        kernel_panic(err);
    }

    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(zombie_threads_table, node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node in zombie table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Remove node from general table */
    node = kernel_queue_find(global_threads_table, thread, &err);
    if(err != OS_NO_ERR && err != OS_ERR_NO_SUCH_ID)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not find joined thread in general table[%d]\n",
                     err);
        kernel_panic(err);
    }
    if(node != NULL && err == OS_NO_ERR)
    {
        err = kernel_queue_remove(global_threads_table, node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node in general table[%d]\n",
                         err);
            kernel_panic(err);
        }

        err = kernel_queue_delete_node(&node);
        if(err != OS_NO_ERR)
        {
            EXIT_CRITICAL(word);
            kernel_error("Could delete thread node[%d]\n", err);
            kernel_panic(err);
        }
    }

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d joined thread %d\n",
                         active_thread->tid,
                         thread->tid);
    #endif

    kfree(thread->stack);
    kfree(thread);
    --thread_count;

    EXIT_CRITICAL(word);
}

/**
 * @brief Thread routine wrapper.
 * 
 * @details Thread launch routine. Wrapper for the actual thread routine. The
 * wrapper will call the thread routine, pass its arguments and gather the 
 * return value of the thread function to allow the joining thread to retreive 
 * it. Some statistics about the thread might be added in this function.
 */
static void thread_wrapper(void)
{
    active_thread->start_time = time_get_current_uptime();

    if(active_thread->function == NULL)
    {
        kernel_error("Thread routine cannot be NULL\n");
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    active_thread->ret_val = active_thread->function(active_thread->args);

    active_thread->return_state = THREAD_RETURN_STATE_RETURNED;

    active_thread->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

/*******************************************************************************
 * System's specific functions.
 * 
 * These are the thread routine used by the kernel to manage the system such as
 * the IDLE or INIT threads.
 ******************************************************************************/

/**
 * @brief Scheduler's main function kickstarter.
 * 
 * @details Main kickstarter, just call the main. This is used to start main in 
 * its own thread.
 *
 * @param[in] args The arguments for the main thread. Unsued at the moment.
 * 
 * @return The main return state.
 */
static void* main_kickstart(void* args)
{
    (void)args;
    char* argv[2] = {"main", NULL};

    /* Call main */
    int32_t ret = main(1, argv);

    return (void*)ret;
}

/**
 * @brief IDLE thread routine.
 * 
 * @details IDLE thread routine. This thread should always be ready, it is the 
 * only thread running when no other trhread are ready. It allows better power
 * consumption management and CPU usage computation.
 *
 * @param[in] args The argument to send to the IDLE thread, usualy null.
 * 
 * @warning The IDLE thread routine should never return.
 * 
 * @return NULL always, should never return.
 */
static void* idle_sys(void* args)
{
    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("IDLE Started\n");
    #endif

    (void)args;

    /* Halt forever, cpu_hlt for energy consumption */
    while(1)
    {
        kernel_interrupt_restore(1);

        if(system_state == SYSTEM_STATE_HALTED)
        {
            kernel_printf("\n");
            kernel_info(" -- System HALTED -- ");
            kernel_interrupt_disable();
        }
        cpu_hlt();
    }

    /* If we return better go away and cry in a corner */
    return NULL;
}

/* INIT thread routine. In addition to the IDLE thread, the INIT thread is the
 * last thread to run. The thread will gather all orphan thread and wait for
 * their death before halting the system. The INIT thread routine is also
 * responsible for calling the main function.
 *
 * @param args The argument to send to the INIT thread, usualy null.
 * @return NULL always
 */
static void* init_func(void* args)
{
    colorscheme_t        new_scheme;
    colorscheme_t        buffer;
    kernel_thread_t*     thread;
    kernel_thread_t*     main_thread;
    kernel_queue_node_t* thread_node;
    uint32_t             sys_thread;
    OS_RETURN_E          err;
    uint32_t             word;

    (void)args;

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("INIT Started\n");
    #endif

    new_scheme.foreground = FG_CYAN;
    new_scheme.background = BG_BLACK;
    new_scheme.vga_color  = 1;

    /* No need to test return value */
    graphic_save_color_scheme(&buffer);

    /* Set REG on BLACK color scheme */
    graphic_set_color_scheme(new_scheme);

    /* Print tag */
    kernel_printf("\n -- RTLK Started -- \n\n");

    /* Restore original screen color scheme */
    graphic_set_color_scheme(buffer);

    #if TEST_MODE_ENABLED == 1
    scheduler_load_test();
    scheduler_preemt_test();
    scheduler_sleep_test();
    critical_test();
    div_by_zero_test();
    mutex_test();
    semaphore_test();
    while(1)
    {
        sched_sleep(10000000);
    }
    #endif

    err = sched_create_thread(&main_thread, KERNEL_HIGHEST_PRIORITY, "main", 
                              SCHEDULER_MAIN_STACK_SIZE, main_kickstart, (void*)1);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot kickstart main, aborting [%d]\n", err);
        kernel_panic(err);
    }

    err = sched_wait_thread(main_thread, NULL, NULL);
    if(err != OS_NO_ERR)
    {
        kernel_error("Cannot waint main, aborting [%d]\n", err);
        kernel_panic(err);
    }

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Main returned, INIT waiting for children\n");
    #endif

    /* System thread are idle threads and INIT */
    sys_thread = 2;

    ENTER_CRITICAL(word);

    /* Wait all children */
    while(thread_count > sys_thread)
    {
        
        thread_node = kernel_queue_pop(active_thread->children, &err);

        while(thread_node != NULL && err == OS_NO_ERR)
        {

            thread = (kernel_thread_t*)thread_node->data;

            EXIT_CRITICAL(word);

            err = sched_wait_thread(thread, NULL, NULL);
            if(err != OS_NO_ERR)
            {
                EXIT_CRITICAL(word);
                kernel_error("Error while waiting thread in INIT [%d]\n", err);
                kernel_panic(err);
            }

            ENTER_CRITICAL(word);

            err = kernel_queue_delete_node(&thread_node);
            if(err != OS_NO_ERR)
            {
                EXIT_CRITICAL(word);
                kernel_error("Error while deleting thread node in INIT [%d]\n", 
                             err);
                kernel_panic(err);
            }
            thread_node = kernel_queue_pop(active_thread->children, &err);
        }
    }

    EXIT_CRITICAL(word);

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("INIT Ended\n");
    #endif

    /* If here, the system is halted */
    system_state = SYSTEM_STATE_HALTED;



    return NULL;
}

/*******************************************************************************
 * System's specific functions END.
 ******************************************************************************/

static OS_RETURN_E create_idle(const uint32_t idle_stack_size)
{
    OS_RETURN_E          err;
    kernel_queue_node_t* second_idle_thread_node;
    char                 idle_name[5] = "Idle\0";
    uint32_t             stack_index;


    idle_thread = kmalloc(sizeof(kernel_thread_t));
    idle_thread_node = kernel_queue_create_node(idle_thread, &err);

    if(err != OS_NO_ERR ||
       idle_thread == NULL || 
       idle_thread_node == NULL)
    {
        if(idle_thread != NULL)
        {
            kfree(idle_thread);
        }
        else 
        {
            err = OS_ERR_MALLOC;
        }
        return err;
    }

    memset(idle_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    idle_thread->tid            = last_given_tid;
    idle_thread->ptid           = last_given_tid;
    idle_thread->priority       = IDLE_THREAD_PRIORITY;
    idle_thread->init_prio      = IDLE_THREAD_PRIORITY;
    idle_thread->args           = 0;
    idle_thread->function       = idle_sys;
    idle_thread->joining_thread = NULL;
    idle_thread->state          = THREAD_STATE_RUNNING;

    idle_thread->children = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kfree(idle_thread);
        return err;
    }

    /* Init thread stack */
    stack_index = (idle_stack_size + ALIGN - 1) & (~(ALIGN - 1));
    stack_index /= sizeof(uint32_t);
    idle_thread->stack = kmalloc(stack_index * sizeof(uint32_t));
    if(idle_thread->stack == NULL)
    {
        kfree(idle_thread);
        return OS_ERR_MALLOC;
    }

    /* Init thread context */
    idle_thread->eip = (uint32_t) thread_wrapper;
    idle_thread->esp =
        (uint32_t)&idle_thread->stack[stack_index - 17];
    idle_thread->ebp =
        (uint32_t)&idle_thread->stack[stack_index - 1];
    idle_thread->tss_esp = 
        (uint32_t)&idle_thread->kernel_stack + THREAD_KERNEL_STACK_SIZE;

    /* Init thread stack */
    idle_thread->stack[stack_index - 1]  = THREAD_INIT_EFLAGS;
    idle_thread->stack[stack_index - 2]  = THREAD_INIT_CS;
    idle_thread->stack[stack_index - 3]  = idle_thread->eip;
    idle_thread->stack[stack_index - 4]  = 0; /* UNUSED (error) */
    idle_thread->stack[stack_index - 5]  = 0; /* UNUSED (int id) */
    idle_thread->stack[stack_index - 6]  = THREAD_INIT_DS;
    idle_thread->stack[stack_index - 7]  = THREAD_INIT_ES;
    idle_thread->stack[stack_index - 8]  = THREAD_INIT_FS;
    idle_thread->stack[stack_index - 9]  = THREAD_INIT_GS;
    idle_thread->stack[stack_index - 10] = THREAD_INIT_SS;
    idle_thread->stack[stack_index - 11] = THREAD_INIT_EAX;
    idle_thread->stack[stack_index - 12] = THREAD_INIT_EBX;
    idle_thread->stack[stack_index - 13] = THREAD_INIT_ECX;
    idle_thread->stack[stack_index - 14] = THREAD_INIT_EDX;
    idle_thread->stack[stack_index - 15] = THREAD_INIT_ESI;
    idle_thread->stack[stack_index - 16] = THREAD_INIT_EDI;
    idle_thread->stack[stack_index - 17] = idle_thread->ebp;
    idle_thread->stack[stack_index - 18] = idle_thread->esp;

    strncpy(idle_thread->name, idle_name, 5);

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("IDLE thread created\n");
    #endif

    second_idle_thread_node = kernel_queue_create_node(idle_thread, &err);
    if(err != OS_NO_ERR || second_idle_thread_node == NULL)
    {
        kfree(idle_thread->stack);
        kfree(idle_thread);
        return err;
    }

    err = kernel_queue_push(second_idle_thread_node, global_threads_table);
    if(err != OS_NO_ERR)
    {
        kfree(idle_thread->stack);
        kfree(idle_thread);
        return err;
    }

    ++thread_count;
    ++last_given_tid;

    /* Initializes the scheduler active thread */
    active_thread = idle_thread;
    active_thread_node = idle_thread_node;
    prev_thread = active_thread;
    prev_thread_node = active_thread_node;

    return OS_NO_ERR;
}

/**
 * @brief Selects the next thread to be scheduled.
 * 
 * @details Selects the next thread to be scheduled. Sets the prev_thread and 
 * active_thread pointers. The function will select the next most prioritary 
 * thread to be executed. This function also wake up sleeping thread which 
 * wake-up time has been reached
 */
static void select_thread(void)
{
    OS_RETURN_E          err;
    kernel_queue_node_t* sleeping_node;
    uint32_t             i;
    uint64_t             current_time = time_get_current_uptime();

    /* Switch running thread */
    prev_thread = active_thread;
    prev_thread_node = active_thread_node;

    /* If the thread was not locked */
    if(prev_thread->state == THREAD_STATE_RUNNING)
    {
        prev_thread->state = THREAD_STATE_READY;
        err = kernel_queue_push(prev_thread_node,
                                active_threads_table[prev_thread->priority]);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic(err);
        }
    }
    else if(prev_thread->state == THREAD_STATE_SLEEPING)
    {
        err = kernel_queue_push_prio(prev_thread_node,
                                     sleeping_threads_table,
                                     prev_thread->wakeup_time);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not enqueue old thread[%d]\n", err);
            kernel_panic(err);
        }
    }

    /* Wake up the sleeping threads */
    do
    {
        kernel_thread_t* sleeping;

        sleeping_node = kernel_queue_pop(sleeping_threads_table, &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue sleeping thread[%d]\n", err);
            kernel_panic(err);
        }

        /* If nothing to wakeup */
        if(sleeping_node == NULL)
        {
            break;
        }

        sleeping = (kernel_thread_t*)sleeping_node->data;

        /* If we should wakeup the thread */
        if(sleeping != NULL && sleeping->wakeup_time < current_time)
        {
            sleeping->state = THREAD_STATE_READY;

            err = kernel_queue_push(sleeping_node,
                                    active_threads_table[sleeping->priority]);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic(err);
            }
        }
        else if(sleeping != NULL)
        {
            err = kernel_queue_push_prio(sleeping_node,
                                         sleeping_threads_table,
                                         sleeping->wakeup_time);
            if(err != OS_NO_ERR)
            {
                kernel_error("Could not enqueue sleeping thread[%d]\n", err);
                kernel_panic(err);
            }
            break;
        }
    } while(sleeping_node != NULL);

    /* Get the new thread */
    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_thread_node = kernel_queue_pop(active_threads_table[i], &err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not dequeue next thread[%d]\n", err);
            kernel_panic(err);
        }
        if(active_thread_node != NULL)
        {
            break;
        }
    }

    if(active_thread_node == NULL || err != OS_NO_ERR)
    {
        kernel_error("Could not dequeue next thread[%d]\n", err);
        kernel_panic(err);
    }

    active_thread = (kernel_thread_t*)active_thread_node->data;

    if(active_thread == NULL)
    {
        kernel_error("Next thread to schedule should not be NULL\n");
        kernel_panic(err);
    }
    active_thread->state = THREAD_STATE_RUNNING;
}

/**
 * @brief Scheduler interrupt handler, executes the conetxt switch.
 * 
 * @details Scheduling function, set a new ESP to the pre interrupt cpu context
 * and save the old ESP to the current thread stack. The function will call the 
 * select_thread function and then set the CPU registers with the values on the 
 * new active_thread stack.
 * 
 * @warning THIS FUNCTION SHOULD NEVER BE CALLED OUTSIDE OF AN INTERRUPT.
 * 
 * @param[in, out] cpu_state The pre interrupt CPU state.
 * @param[in] int_id The interrupt id when calling this function.
 * @param[in] stack_state The pre interrupt stack state.
 */
static void schedule_int(cpu_state_t *cpu_state, uint32_t int_id,
                         stack_state_t *stack_state)
{
    (void) int_id;
    (void) stack_state;

    /* Save the actual ESP (not the fist time since the first schedule should
     * dissociate the boot sequence (pointed by the current esp) and the IDLE
     * thread.*/
    if(first_sched == 1)
    {
        active_thread->esp = cpu_state->esp;
    }
    else 
    {
        first_sched = 1;
    }

    /* Search for next thread */
    select_thread();

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("CPU Sched %d -> %d\n",
                        prev_thread->tid, active_thread->tid);
    #endif

    /* Restore thread esp */
    cpu_state->esp = active_thread->esp;
}

SYSTEM_STATE_E get_system_state(void)
{
    return system_state;
}

OS_RETURN_E sched_init(void)
{
    OS_RETURN_E err;
    uint32_t    i;

    /* Init scheduler settings */
    last_given_tid  = 0;
    thread_count    = 0;

    init_thread      = NULL;
    init_thread_node = NULL;

    first_sched = 0;

    /* Init thread tables */
    global_threads_table = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create global_threads_table[%d]\n", err);
        kernel_panic(err);
    }

    for(i = 0; i < KERNEL_LOWEST_PRIORITY + 1; ++i)
    {
        active_threads_table[i] = kernel_queue_create_queue(&err);
        if(err != OS_NO_ERR)
        {
            kernel_error("Could not create active_threads_table %d [%d]\n", 
                         i, err);
            kernel_panic(err);
        }
    }

    zombie_threads_table = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create zombie_threads_table[%d]\n", err);
        kernel_panic(err);
    }

    sleeping_threads_table = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create sleeping_threads_table %d [%d]\n", i, err);
        kernel_panic(err);
    }

    /* Create idle thread */
    err = create_idle(SCHEDULER_IDLE_STACK_SIZE);
    if(err != OS_NO_ERR)
    {
        kernel_error("Could not create IDLE thread[%d]\n", err);
        kernel_panic(err);
    }

    /* Register SW interrupt scheduling */
    err = kernel_interrupt_register_int_handler(SCHEDULER_SW_INT_LINE, 
                                                schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Register the scheduler on the main system timer. */
    err = time_register_scheduler(schedule_int);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Create INIT thread */
    err = sched_create_thread(&init_thread, KERNEL_HIGHEST_PRIORITY, "init", 
                              SCHEDULER_INIT_STACK_SIZE, init_func, (void*)0);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    system_state = SYSTEM_STATE_RUNNING;
    kernel_success("SCHEDULER Initialized\n");

    /* First schedule, we should never return from here */
    sched_schedule();

    /* We should never return fron this function */
    return OS_ERR_UNAUTHORIZED_ACTION;
}

void sched_schedule(void)
{
    /* Raise scheduling interrupt */
    __asm__ __volatile__("int %0" :: "i" (SCHEDULER_SW_INT_LINE));
    kernel_interrupt_set_irq_eoi(SCHEDULER_SW_INT_LINE);
}

OS_RETURN_E sched_sleep(const unsigned int time_ms)
{
    /* We cannot sleep in idle */
    if(active_thread == idle_thread)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    active_thread->wakeup_time = time_get_current_uptime() + time_ms;
    active_thread->state       = THREAD_STATE_SLEEPING;

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("[%d] Thread %d asleep until %d (%dms)\n", 
                        (uint32_t)time_get_current_uptime(), 
                        active_thread->tid,
                        (uint32_t)active_thread->wakeup_time,
                        time_ms);
    #endif

    sched_schedule();

    return OS_NO_ERR;
}

uint32_t sched_get_thread_count(void)
{
    return thread_count;
}

int32_t sched_get_tid(void)
{
    return active_thread->tid;
}

int32_t sched_get_ptid(void)
{
    return active_thread->ptid;
}

uint32_t sched_get_priority(void)
{
    return active_thread->priority;
}

OS_RETURN_E sched_set_priority(const uint32_t priority)
{
    /* Check if priority is free */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    active_thread->priority = priority;

    return OS_NO_ERR;
}

OS_RETURN_E get_threads_info(thread_info_t* threads, int32_t* size)
{
    int32_t               i;
    kernel_queue_node_t*  cursor;
    kernel_thread_t*      cursor_thread;
    uint32_t              word;

    if(threads == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(size == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    if(*size > (int)thread_count)
    {
        *size = thread_count;
    }

    ENTER_CRITICAL(word);

    /* Walk the thread list and fill the structures */
    cursor = global_threads_table->head;
    cursor_thread = (kernel_thread_t*)cursor->data;
    for(i = 0; cursor != NULL && i < *size; ++i)
    {
        thread_info_t *current = &threads[i];
        current->tid = cursor_thread->tid;
        current->ptid = cursor_thread->ptid;
        strncpy(current->name, cursor_thread->name, THREAD_MAX_NAME_LENGTH);
        current->priority = cursor_thread->priority;
        current->state = cursor_thread->state;
        current->start_time = cursor_thread->start_time;
        if(current->state != THREAD_STATE_ZOMBIE)
        {
            current->end_time = time_get_current_uptime();
        }
        else
        {
            current->end_time = cursor_thread->end_time;
        }

        cursor = cursor->next;
        cursor_thread = (kernel_thread_t*)cursor->data;
    }

    EXIT_CRITICAL(word);

    return OS_NO_ERR;
}

void sched_set_thread_termination_cause(const THREAD_TERMINATE_CAUSE_E term_cause)
{
    active_thread->return_cause = term_cause;
}

void sched_terminate_thread(void)
{
    active_thread->return_state = THREAD_RETURN_STATE_KILLED;

    active_thread->end_time = time_get_current_uptime();

    /* Exit thread properly */
    thread_exit();
}

OS_RETURN_E sched_create_thread(thread_t* thread,                          
                                const uint32_t priority,
                                const char* name,
                                const uint32_t stack_size,
                                void* (*function)(void*),
                                void* args)
{
    OS_RETURN_E          err;
    kernel_thread_t*     new_thread;
    kernel_queue_node_t* new_thread_node;
    kernel_queue_node_t* seconde_new_thread_node;
    kernel_queue_node_t* children_new_thread_node;
    uint32_t             stack_index;
    uint32_t             word;

    if(thread != NULL)
    {
        *thread = NULL;
    }

    /* Check if priority is valid */
    if(priority > KERNEL_LOWEST_PRIORITY)
    {
        return OS_ERR_FORBIDEN_PRIORITY;
    }

    ENTER_CRITICAL(word);

    new_thread = kmalloc(sizeof(kernel_thread_t));
    new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR ||
       new_thread == NULL || 
       new_thread_node == NULL)
    {
        if(new_thread != NULL)
        {
            kfree(new_thread);
        }
        else 
        {
            err = OS_ERR_MALLOC;
        }
        EXIT_CRITICAL(word);
        return err;
    }
    memset(new_thread, 0, sizeof(kernel_thread_t));

    /* Init thread settings */
    new_thread->tid            = ++last_given_tid;
    new_thread->ptid           = active_thread->tid;
    new_thread->priority       = priority;
    new_thread->init_prio      = priority;
    new_thread->args           = args;
    new_thread->function       = function;
    new_thread->joining_thread = NULL;
    new_thread->state          = THREAD_STATE_READY;

    new_thread->children = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_node(&new_thread_node);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return err;
    }

    /* Init thread stack and align stack size */
    stack_index = (stack_size + ALIGN - 1) & (~(ALIGN - 1));
    stack_index /= sizeof(uint32_t);
    new_thread->stack = kmalloc(stack_index * sizeof(uint32_t));
    if(new_thread->stack == NULL)
    {
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_queue(&new_thread->children);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return OS_ERR_MALLOC;
    }

    /* Init thread context */
    new_thread->eip = (uint32_t) thread_wrapper;
    new_thread->esp =
        (uint32_t)&new_thread->stack[stack_index - 17];
    new_thread->ebp =
        (uint32_t)&new_thread->stack[stack_index - 1];
    new_thread->tss_esp = 
        (uint32_t)&new_thread->kernel_stack + THREAD_KERNEL_STACK_SIZE;

    /* Init thread stack */
    new_thread->stack[stack_index - 1]  = THREAD_INIT_EFLAGS;
    new_thread->stack[stack_index - 2]  = THREAD_INIT_CS;
    new_thread->stack[stack_index - 3]  = new_thread->eip;
    new_thread->stack[stack_index - 4]  = 0; /* UNUSED (error core) */
    new_thread->stack[stack_index - 5]  = 0; /* UNUSED (int id) */
    new_thread->stack[stack_index - 6]  = THREAD_INIT_DS;
    new_thread->stack[stack_index - 7]  = THREAD_INIT_ES;
    new_thread->stack[stack_index - 8]  = THREAD_INIT_FS;
    new_thread->stack[stack_index - 9]  = THREAD_INIT_GS;
    new_thread->stack[stack_index - 10] = THREAD_INIT_SS;
    new_thread->stack[stack_index - 11] = THREAD_INIT_EAX;
    new_thread->stack[stack_index - 12] = THREAD_INIT_EBX;
    new_thread->stack[stack_index - 13] = THREAD_INIT_ECX;
    new_thread->stack[stack_index - 14] = THREAD_INIT_EDX;
    new_thread->stack[stack_index - 15] = THREAD_INIT_ESI;
    new_thread->stack[stack_index - 16] = THREAD_INIT_EDI;
    new_thread->stack[stack_index - 17] = new_thread->ebp;
    new_thread->stack[stack_index - 18] = new_thread->esp;

    strncpy(new_thread->name, name, THREAD_MAX_NAME_LENGTH);

    /* Add thread to the system's queues. */
    seconde_new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return err;
    }

    children_new_thread_node = kernel_queue_create_node(new_thread, &err);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return err;
    }

    err = kernel_queue_push(new_thread_node, active_threads_table[priority]);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&children_new_thread_node);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return err;
    }

     err = kernel_queue_push(seconde_new_thread_node, global_threads_table);
     if(err != OS_NO_ERR)
     {
         kernel_queue_delete_queue(&new_thread->children);
         kernel_queue_delete_node(&children_new_thread_node);
         kernel_queue_delete_node(&new_thread_node);
         kernel_queue_delete_node(&seconde_new_thread_node);
         kernel_queue_remove(active_threads_table[priority], new_thread_node);
         kfree(new_thread->stack);
         kfree(new_thread);
         EXIT_CRITICAL(word);
         return err;
     }


    err = kernel_queue_push(children_new_thread_node, active_thread->children);
    if(err != OS_NO_ERR)
    {
        kernel_queue_delete_queue(&new_thread->children);
        kernel_queue_delete_node(&children_new_thread_node);
        kernel_queue_delete_node(&new_thread_node);
        kernel_queue_delete_node(&seconde_new_thread_node);
        kernel_queue_remove(active_threads_table[priority], new_thread_node);
        kernel_queue_remove(global_threads_table, seconde_new_thread_node);
        kfree(new_thread->stack);
        kfree(new_thread);
        EXIT_CRITICAL(word);
        return err;
    }

    ++thread_count;

    EXIT_CRITICAL(word);

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Created thread %d\n", new_thread->tid);
    #endif

    if(thread != NULL)
    {
        *thread = new_thread;
    }

    return OS_NO_ERR;
}

OS_RETURN_E sched_wait_thread(thread_t thread, void** ret_val, 
                              THREAD_TERMINATE_CAUSE_E* term_cause)
{
    if(thread == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d waiting for thread %d\n",
                         active_thread->tid,
                         thread->tid);
    #endif

    if(thread->state == THREAD_STATE_DEAD)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* If thread already done then remove it from the thread table */
    if(thread->state == THREAD_STATE_ZOMBIE)
    {
        thread->state = THREAD_STATE_DEAD;

        if(ret_val != NULL)
        {
            *ret_val = thread->ret_val;
        }
        sched_clean_joined_thread(thread);
        return OS_NO_ERR;
    }

    /* Wait for the thread to finish */
    active_thread->state   = THREAD_STATE_JOINING;
    thread->joining_thread = active_thread_node;

    /* Schedule thread */
    sched_schedule();

    if(ret_val != NULL)
    {
        *ret_val = thread->ret_val;
    }
    if(term_cause != NULL)
    {
        *term_cause = thread->return_cause;
    }
    sched_clean_joined_thread(thread);

    return OS_NO_ERR;
}

kernel_queue_node_t* sched_lock_thread(const THREAD_WAIT_TYPE_E block_type)
{
    kernel_queue_node_t* current_thread_node;

    /* Cant lock kernel thread */
    if(active_thread == idle_thread)
    {
        return NULL;
    }

    current_thread_node = active_thread_node;

    /* Lock the thread */
    active_thread->state      = THREAD_STATE_WAITING;
    active_thread->block_type = block_type;

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d locked, reason: %d\n",
                        active_thread->tid,
                        block_type);
    #endif

    return current_thread_node;
}

OS_RETURN_E sched_unlock_thread(kernel_queue_node_t* node,
                                const THREAD_WAIT_TYPE_E block_type,
                                const uint8_t do_schedule)
{
    OS_RETURN_E      err;
    uint32_t         word;
    kernel_thread_t* thread = (kernel_thread_t*)node->data;

    /* Check thread value */
    if(thread == NULL || thread == idle_thread)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Check thread state */
    if(thread->state != THREAD_STATE_WAITING ||
       thread->block_type != block_type)
    {
        switch(block_type)
        {
            case THREAD_WAIT_TYPE_SEM:
                return OS_ERR_NO_SEM_BLOCKED;
            case THREAD_WAIT_TYPE_MUTEX:
                return OS_ERR_NO_MUTEX_BLOCKED;
            default:
                return OS_ERR_NULL_POINTER;
        }
    }

    ENTER_CRITICAL(word);

    /* Unlock thread state */
    thread->state = THREAD_STATE_READY;
    err = kernel_queue_push(node, active_threads_table[thread->priority]);
    if(err != OS_NO_ERR)
    {
        EXIT_CRITICAL(word);
        kernel_error("Could not enqueue thread in active table[%d]\n", err);
        kernel_panic(err);
    }

    #if SCHED_KERNEL_DEBUG == 1
    kernel_serial_debug("Thread %d unlocked, reason: %d\n",
                         thread->tid,
                         block_type);
    #endif

    EXIT_CRITICAL(word);

    if(do_schedule)
    {
        sched_schedule();
    }

    return OS_NO_ERR;
}