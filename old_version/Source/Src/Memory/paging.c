/***************************************************************************//**
 * @file paging.c
 *
 * @see paginh.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 30/03/2020
 *
 * @version 1.0
 *
 * @brief Kernel memory paging manager.
 *
 * @details Kernel memory paging manager. Allows to define memory fault handlers
 * and manage memory functions.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>           /* OS_RETURN_E */
#include <Lib/stdint.h>           /* Generic int types */
#include <Interrupt/exceptions.h> /* Exception management */
#include <Cpu/panic.h>            /* Kernel panic */
#include <Memory/kheap.h>         /* kmalloc */
#include <Sync/critical.h>        /* Spinlock */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Memory/paging.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

#if MAX_CPU_COUNT > 1
/** @brief Critical section spinlock. */
static spinlock_t lock = SPINLOCK_INIT_VALUE;
#endif

mem_handler_t* handler_list = NULL;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/


OS_RETURN_E paging_register_fault_handler(void (*handler)(const address_t),
                                          const address_t range_start,
                                          const address_t range_end)
{
    mem_handler_t* cursor;
    mem_handler_t* cursor_pre;
    mem_handler_t* new_range;

    uint32_t word;

    /* Check integrity */
    if(handler == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }
    if(range_end <= range_start)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Create the range */
    new_range = kmalloc(sizeof(mem_handler_t));
    if(new_range == NULL)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_MALLOC;
    }
    new_range->start   = range_start;
    new_range->end     = range_end;
    new_range->handler = handler;
    new_range->next    = NULL;

    /* If the handler list is empty, add a new one */
    if(handler_list != NULL)
    {
        /* Search for the range */
        cursor = handler_list;
        cursor_pre = cursor;
        while(cursor && cursor->start <= range_start)
        {
            cursor_pre = cursor;
            cursor     = cursor->next;
        }

        /* If found */
        if(cursor != NULL)
        {
            if(cursor == handler_list)
            {
                if(cursor->start < range_end)
                {
                    kfree(new_range);
                    #if MAX_CPU_COUNT > 1
                    EXIT_CRITICAL(word, &lock);
                    #else
                    EXIT_CRITICAL(word);
                    #endif
                    return OS_ERR_HANDLER_ALREADY_EXISTS;
                }

                 /* Link the range */
                new_range->next = cursor;
                handler_list = new_range;
            }
            /* Check that the cursor end is not in our range */
            else if(cursor_pre->end > range_start || cursor->start < range_end)
            {
                kfree(new_range);
                #if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(word, &lock);
                #else
                EXIT_CRITICAL(word);
                #endif
                return OS_ERR_HANDLER_ALREADY_EXISTS;
            }
            else 
            {
                /* Link the range */
                new_range->next = cursor_pre->next;
                cursor_pre->next = new_range;
            }
        }
        else 
        {
            if(cursor_pre->end > range_start)
            {
                kfree(new_range);
                #if MAX_CPU_COUNT > 1
                EXIT_CRITICAL(word, &lock);
                #else
                EXIT_CRITICAL(word);
                #endif
                return OS_ERR_HANDLER_ALREADY_EXISTS;
            }
            cursor_pre->next = new_range;
        }
    }
    else 
    {
        handler_list = new_range;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return OS_NO_ERR;
}

const mem_handler_t* paging_get_handler_list(void)
{
    return handler_list;
}