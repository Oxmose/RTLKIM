/***************************************************************************//**
 * @file queue.c
 *
 * @see queue.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Queue communication and synchronization primitive.
 *
 * @details Queue used to send multiple messages between threads. The queues
 * will block the threads when either full (on a sending thread) or empty (on a
 * receiving thread). The synchronization method used is the semaphore.
 *
 * @warning Queues can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/string.h>       /* memset */
#include <IO/kernel_output.h> /* kernel_error */
#include <Cpu/panic.h>  /* kernel_panic */
#include <Memory/kheap.h>     /* kmalloc kfree */
#include <Sync/semaphore.h>   /* semaphore_t */
#include <Sync/critical.h>    /* ENTER_CRITICAL, EXIT_CRITICAL */

/* UTK configuration file */
#include <config.h>

/* Header file */
#include <Communication/queue.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E queue_init(queue_t* queue, const uint32_t size)
{
    OS_RETURN_E err;

    /* Pointer check */
    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the queue */
    memset(queue, 0, sizeof(queue_t));

    #if MAX_CPU_COUNT > 1
    INIT_SPINLOCK(&queue->lock);
    #endif

    err = sem_init(&queue->queue_sem_read, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = sem_init(&queue->queue_sem_write, size);
    if(err != OS_NO_ERR)
    {
        err = sem_destroy(&queue->queue_sem_read);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return err;
    }

    queue->container = kmalloc(sizeof(void*) * size);
    if(queue->container == NULL)
    {
        err = sem_destroy(&queue->queue_sem_read);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        err = sem_destroy(&queue->queue_sem_write);
        if(err != OS_NO_ERR)
        {
            return err;
        }

        return OS_ERR_MALLOC;
    }
    queue->max_size = size;  /* Queue max size */

    queue->init = 1;

    #if USERQUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Queue 0x%08x INIT\n", (uint32_t)queue);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E queue_destroy(queue_t* queue)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if USERQUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Queue 0x%08x DESTROY\n", (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(queue->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    kfree(queue->container);

    /* Set the queue to a destroyed state */
    queue->init  = 0;


    err = sem_destroy(&queue->queue_sem_read);
    err |= sem_destroy(&queue->queue_sem_write);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

void* queue_pend(queue_t* queue, OS_RETURN_E* error)
{
    OS_RETURN_E err;
    void*       ret_val;
    uint32_t    word;

    #if USERQUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Queue 0x%08x PEND\n", (uint32_t)queue);
    #endif

    /* Check queue pointer */
    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Check for queue initialization */
    if(queue->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }

        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    /* If the queue is empty block thread */
    err = sem_pend(&queue->queue_sem_read);
    if(err != OS_NO_ERR)
    {
        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }
        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(queue->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }
        return NULL;
    }

    /* Get queue value */
    ret_val = queue->container[queue->index_bot];
    queue->index_bot = (queue->index_bot + 1) % queue->max_size;
    --queue->size;

    err = sem_post(&queue->queue_sem_write);
    if(err != OS_NO_ERR)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }
        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    #if USERQUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Queue 0x%08x ACQUIRED\n", (uint32_t)queue);
    #endif

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret_val;
}

OS_RETURN_E queue_post(queue_t* queue, void* element)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if USERQUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Queue 0x%08x POST\n", (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(queue->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    err = sem_pend(&queue->queue_sem_write);
    if(err != OS_NO_ERR)
    {
        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Set value of the queue */
    queue->container[queue->index_top] = element;
    queue->index_top = (queue->index_top + 1) % queue->max_size;
    ++queue->size;

    err = sem_post(&queue->queue_sem_read);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    if(err != OS_NO_ERR)
    {
        return OS_ERR_QUEUE_NON_INITIALIZED;
    }

    return OS_NO_ERR;
}

int32_t queue_isempty(queue_t* queue, OS_RETURN_E* error)
{
    int32_t size = queue_size(queue, error);

    if(size < 0)
    {
        return -1;
    }

    return size == 0;
}

int32_t queue_size(queue_t* queue, OS_RETURN_E* error)
{
    int32_t  ret;
    uint32_t word;

    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return -1;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &queue->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(queue->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &queue->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        if(error != NULL)
        {
            *error = OS_ERR_QUEUE_NON_INITIALIZED;
        }

        return -1;
    }

    ret = (queue->size);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &queue->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret;
}