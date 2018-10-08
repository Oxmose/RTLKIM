/***************************************************************************//**
 * @file kernel_queue.c
 * 
 * @see kernel_queue.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 09/03/2018
 *
 * @version 1.5
 *
 * @brief Kernel's queue structures.
 * 
 * @details Kernel's queue structures. These queues are used by the kernel as
 * priority queue or regular queues. A kernel queue can virtually store every 
 * type of data and is just a wrapper.
 * 
 * @warning Kernel's queues are not thread safe.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>      /* OS_RETURN_E */
#include <Lib/stdint.h>      /* Generic int types */
#include <Lib/string.h>      /* memset */
#include <Memory/kheap.h>    /* kmalloc, kfree */
#include <Interrupt/panic.h> /* kernel_panic */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Core/kernel_queue.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

kernel_queue_node_t* kernel_queue_create_node(void* data, OS_RETURN_E *error)
{
    kernel_queue_node_t* new_node;

    /* Create new node */
    new_node = kmalloc(sizeof(kernel_queue_node_t));

    if(new_node == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }
     /* Init the structure */
    memset(new_node, 0, sizeof(kernel_queue_node_t));
    new_node->data = data;
    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }
    return new_node;
}

OS_RETURN_E kernel_queue_delete_node(kernel_queue_node_t** node)
{
    if(node == NULL || *node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Checkqueuechaining */
    if((*node)->enlisted != 0)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    kfree(*node);

    *node = NULL;

    return OS_NO_ERR;
}

kernel_queue_t* kernel_queue_create_queue(OS_RETURN_E *error)
{
    kernel_queue_t* newqueue;

    /* Create new node */
    newqueue = kmalloc(sizeof(kernel_queue_t));
    if(newqueue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MALLOC;
        }
        return NULL;
    }

    /* Init the structure */
    memset(newqueue, 0, sizeof(kernel_queue_t));

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return newqueue;
}

OS_RETURN_E kernel_queue_delete_queue(kernel_queue_t** queue)
{
    if(queue == NULL || *queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Check queue chaining */
    if((*queue)->head != NULL || (*queue)->tail != NULL)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    kfree(*queue);

    *queue = NULL;

    return OS_NO_ERR;
}

OS_RETURN_E kernel_queue_push(kernel_queue_node_t* node,
                             kernel_queue_t* queue)
{
    #if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Enqueue 0x%08x in queue 0x%08x\n",
                        (uint32_t)node,
                        (uint32_t)queue);
    #endif

    if(node == NULL || queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        /* Set the first item */
        queue->head = node;
        queue->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        /* Just put on the tail */
        node->next = queue->head;
        node->prev = NULL;
        queue->head->prev = node;
        queue->head = node;
    }

    ++queue->size;
    node->enlisted = 1;

    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }

    return OS_NO_ERR;
}


OS_RETURN_E kernel_queue_push_prio(kernel_queue_node_t* node,
                                   kernel_queue_t* queue,
                                   const uint32_t priority)
{
    kernel_queue_node_t* cursor;

    #if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Enqueue 0x%08x in queue 0x%08x\n",
                        (uint32_t)node,
                        (uint32_t)queue);
    #endif

    if(node == NULL || queue == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    node->priority = priority;

    /* If this queue is empty */
    if(queue->head == NULL)
    {
        /* Set the first item */
        queue->head = node;
        queue->tail = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else
    {
        cursor = queue->head;
        while(cursor != NULL && cursor->priority > priority)
        {
            cursor = cursor->next;
        }

        if(cursor != NULL)
        {
            node->next = cursor;
            node->prev = cursor->prev;
            cursor->prev = node;
            if(node->prev != NULL)
            {
                node->prev->next = node;
            }
            else
            {
                queue->head = node;
            }
        }
        else
        {
            /* Just put on the tail */
            node->prev = queue->tail;
            node->next = NULL;
            queue->tail->next = node;
            queue->tail = node;
        }
    }
    ++queue->size;
    node->enlisted = 1;

    if(node->next != NULL && node->prev != NULL && node->next == node->prev)
    {
        kernel_panic(OS_ERR_UNAUTHORIZED_ACTION);
    }
    return OS_NO_ERR;
}

kernel_queue_node_t* kernel_queue_pop(kernel_queue_t* queue,
                                      OS_RETURN_E* error)
{
    kernel_queue_node_t*  node;

    #if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Dequeue kernel element in queue 0x%08x\n",
                        (uint32_t)queue);
    #endif

    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }


    /* If this queue is empty */
    if(queue->head == NULL)
    {
        return NULL;
    }

    /* Dequeue the last item */
    node = queue->tail;
    if(node->prev != NULL)
    {
        node->prev->next = NULL;
        queue->tail = node->prev;
    }
    else
    {
        queue->head = NULL;
        queue->tail = NULL;
    }

    --queue->size;

    node->next = NULL;
    node->prev = NULL;
    node->enlisted = 0;

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return node;
}

kernel_queue_node_t* kernel_queue_find(kernel_queue_t* queue, void* data,
                                       OS_RETURN_E *error)
{
    kernel_queue_node_t* node;

    #if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Find kernel data 0x%08x in queue 0x%08x\n",
                        (uint32_t)data,
                        (uint32_t)queue);
    #endif

    /* If this queue is empty */
    if(queue == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }
        return NULL;
    }

    /* Search for the data */
    node = queue->head;
    while(node != NULL && node->data != data)
    {
        node = node->next;
    }

    /* No such data */
    if(node == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NO_SUCH_ID;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return node;
}

OS_RETURN_E kernel_queue_remove(kernel_queue_t* queue,
                               kernel_queue_node_t* node)
{
    kernel_queue_node_t* cursor;
    if(queue == NULL || node == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if QUEUE_KERNEL_DEBUG == 1
    kernel_serial_debug("Remove node kernel node 0x%08x inqueue0x%08x\n",
                        (uint32_t)node,
                        (uint32_t)queue);
    #endif

    /* Search for node in the queue*/
    cursor = queue->head;
    while(cursor != NULL && cursor != node)
    {
        cursor = cursor->next;
    }

    if(cursor == NULL)
    {
        return OS_ERR_NO_SUCH_ID;
    }

    /* Manage link */
    if(cursor->prev != NULL && cursor->next != NULL)
    {
        cursor->prev->next = cursor->next;
        cursor->next->prev = cursor->prev;
    }
    else if(cursor->prev == NULL && cursor->next != NULL)
    {
        queue->head = cursor->next;
        cursor->next->prev = NULL;
    }
    else if(cursor->prev != NULL && cursor->next == NULL)
    {
        queue->tail = cursor->prev;
        cursor->prev->next = NULL;
    }
    else
    {
        queue->head = NULL;
        queue->tail = NULL;
    }

    node->next = NULL;
    node->prev = NULL;

    node->enlisted = 0;

    return OS_NO_ERR;
}