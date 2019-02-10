/***************************************************************************//**
 * @file semaphore.c
 *
 * @see semaphore.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 17/12/2017
 *
 * @version 2.0
 *
 * @brief Semaphore synchronization primitive.
 *
 * @details Semaphore synchronization primitive implemantation.
 * The semaphore are used to synchronyse the threads. The semaphore waiting list
 * is a FIFO with no regards of the waiting threads priority.
 *
 * @warning Semaphores can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Lib/stdint.h>           /* Generic int types */
#include <Lib/string.h>           /* memset */
#include <Core/kernel_queue.h>    /* kernel_queue_t, kernel_queue_node_t */
#include <Core/scheduler.h>       /* sched_lock_thread, sched_unlock_thread */
#include <Sync/critical.h>        /* ENTER_CRITICAL EXIT_CRITICIAL */
#include <IO/kernel_output.h>     /* kernel_error */
#include <Interrupt/interrupts.h> /* kernel_interrupt_get_state */
#include <Interrupt/panic.h>      /* kernel_panic */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Sync/semaphore.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E sem_init(semaphore_t* sem, const int32_t init_level)
{
    OS_RETURN_E err;
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the semaphore*/
    memset(sem, 0, sizeof(semaphore_t));

    sem->sem_level = init_level;

    #if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&sem->lock);
    #endif

    sem->waiting_threads = kernel_queue_create_queue(&err);
    if(err != OS_NO_ERR)
    {
        return err;
    }

    sem->init = 1;

    #if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%08x initialized\n", (uint32_t)sem);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_destroy(semaphore_t* sem)
{
    kernel_queue_node_t* node;
    OS_RETURN_E          err;
    uint32_t             word;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &sem->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(sem->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    sem->init = 0;

    /* Unlock all threead*/
    while((node = kernel_queue_pop(sem->waiting_threads, &err)) != NULL)
    {
        if(err != OS_NO_ERR)
        {
            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif
            kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }
        err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 0);
        if(err != OS_NO_ERR)
        {
            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif
            kernel_error("Could not unlock thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }

        #if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Semaphore 0x%08x unlocked thead %d\n",
                            (uint32_t)sem,
                            ((kernel_thread_t*)node->data)->tid);
        #endif
    }
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
        kernel_panic(err);
    }

    #ifdef DEBUG_MUTEX
    kernel_serial_debug("Semaphore 0x%08x destroyed\n", (uint32_t)sem);
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &sem->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_pend(semaphore_t* sem)
{
    OS_RETURN_E err;
    uint32_t    word;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &sem->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(sem->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the semaphore
     * has not been destroyed
     */
    while(sem->init == 1 &&
          sem->sem_level < 1)
    {
        kernel_queue_node_t* active_thread = sched_lock_thread(THREAD_WAIT_TYPE_SEM);

        if(active_thread == NULL)
        {
            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif
            kernel_error("Could not lock this thread to semaphore[%d]\n",
                         OS_ERR_NULL_POINTER);
            kernel_panic(OS_ERR_NULL_POINTER);
        }

        err = kernel_queue_push(active_thread, sem->waiting_threads);

        if(err != OS_NO_ERR)
        {
            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif
            kernel_error("Could not enqueue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }

        #if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Semaphore 0x%08x locked thead %d\n",
                            (uint32_t)sem,
                            ((kernel_thread_t*)active_thread->data)->tid);
        #endif

        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        sched_schedule();

        #if MAX_CPU_COUNT > 1
        ENTER_CRITICAL(word, &sem->lock);
        #else
        ENTER_CRITICAL(word);
        #endif
    }

    if(sem->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Decrement sem level */
    --(sem->sem_level);

    #if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%08x aquired by thead %d\n",
                        (uint32_t)sem,
                        sched_get_tid());
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &sem->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_post(semaphore_t* sem)
{
    OS_RETURN_E err;
    uint32_t    word;

    /* Check if semaphore is initialized */
    if(sem == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &sem->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(sem->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Increment sem level */
    ++sem->sem_level;

    /* Check if we can unlock a blocked thread on the semaphore */
    if(sem->sem_level > 0)
    {
        kernel_queue_node_t* node;

        if((node = kernel_queue_pop(sem->waiting_threads, &err))
            != NULL)
        {
            if(err != OS_NO_ERR)
            {
                #if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(word, &sem->lock);
                #else
                EXIT_CRITICAL(word);
                #endif
                kernel_error("Could not dequeue thread from semaphore[%d]\n",
                             err);
                kernel_panic(err);
            }

            #if SEMAPHORE_KERNEL_DEBUG == 1
            kernel_serial_debug("Semaphore 0x%08x unlocked thead %d\n",
                                (uint32_t)sem,
                                ((kernel_thread_t*)node->data)->tid);
            #endif

            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif

            /* Do not schedule in interrupt handlers */
            if(kernel_interrupt_get_state() > 0)
            {
                err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 0);
            }
            else
            {
                err = sched_unlock_thread(node, THREAD_WAIT_TYPE_SEM, 1);
            }

            if(err != OS_NO_ERR)
            {
                #if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(word, &sem->lock);
                #else
                EXIT_CRITICAL(word);
                #endif
                kernel_error("Could not unlock thread from semaphore[%d]\n",
                             err);
                kernel_panic(err);
            }

            #if SEMAPHORE_KERNEL_DEBUG == 1
            kernel_serial_debug("Semaphore 0x%08x released by thead %d\n",
                                (uint32_t)sem,
                                sched_get_tid());
            #endif

            return OS_NO_ERR;
        }
        if(err != OS_NO_ERR)
        {
            #if MAX_CPU_COUNT > 1
            EXIT_CRITICAL(word, &sem->lock);
            #else
            EXIT_CRITICAL(word);
            #endif
            kernel_error("Could not dequeue thread from semaphore[%d]\n", err);
            kernel_panic(err);
        }
    }

    #if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Semaphore 0x%08x released by thead %d\n",
                        (uint32_t)sem,
                        sched_get_tid());
    #endif

    /* If here, we did not find any waiting process */
    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &sem->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E sem_try_pend(semaphore_t* sem, int32_t* value)
{
    uint32_t word;

    /* Check if semaphore is initialized */
    if(sem == NULL || value == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &sem->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(sem->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    /* Check if we can enter the critical section, also check if the semaphore
     * has not been destroyed
     */
    if(sem != NULL &&
       sem->sem_level < 1)
    {
        *value = sem->sem_level;

        #if SEMAPHORE_KERNEL_DEBUG == 1
        kernel_serial_debug("Locked semaphore 0x%08x try pend by thead %d\n",
                            (uint32_t)sem,
                            sched_get_tid());
        #endif

        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_SEM_LOCKED;
    }
    else if(sem != NULL && sem->init == 1)
    {
        *value = --sem->sem_level;
    }
    else
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &sem->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_SEM_UNINITIALIZED;
    }

    #if SEMAPHORE_KERNEL_DEBUG == 1
    kernel_serial_debug("Unlocked semaphore 0x%08x try pend and aquired by "
                        "thead %d\n",
                        (uint32_t)sem,
                        sched_get_tid());
    #endif

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &sem->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}