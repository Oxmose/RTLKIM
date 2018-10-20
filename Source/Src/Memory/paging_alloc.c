/***************************************************************************//**
 * @file paging_alloc.c
 *
 * @see paging_alloc.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 20/10/2018
 *
 * @version 1.0
 *
 * @brief Kernel memory physical frame and virtual page allocator.
 *
 * @details Kernel memory physical frame and virtual page allocator. This module
 * allows to* allocate and deallocate physical memory frame and virtual pages.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>     /* OS_RETURN_E */
#include <Lib/stdint.h>     /* Generic int types */
#include <Memory/meminfo.h> /* mem_range_t */
#include <Memory/kheap.h>   /* kmalloc */
#include <Memory/paging.h>  /* KERNEL_PAGE_SIZE */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Memory/paging_alloc.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/
static mem_area_t* free_frames;

static mem_area_t* free_pages;

/** @brief Memory map structure's size. */
extern uint32_t    memory_map_size;

/** @brief Memory map storage as an array of range. */
extern mem_range_t memory_map_data[];


extern uint8_t     _kernel_end;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

static OS_RETURN_E add_free(const uint32_t start, const uint32_t size,
                            mem_area_t** list)
{
    mem_area_t* cursor;
    mem_area_t* save;
    mem_area_t* new_node;

    if(*list == NULL)
    {
        /* Create first node */
        *list = kmalloc(sizeof(mem_area_t));

        if(list == NULL)
        {
            return OS_ERR_MALLOC;
        }

        (*list)->start = start;
        (*list)->size  = size;

        (*list)->next = NULL;
        (*list)->prev = NULL;

        return OS_NO_ERR;
    }

    /* Search and link new node */
    cursor = *list;
    save = cursor;
    while(cursor != NULL)
    {
        if(cursor->start > start)
        {
            break;
        }
        save = cursor;
        cursor = cursor->next;
    }

    /* Check boundaries */
    if(save != cursor && save->start + save->size > start)
    {
        return OS_ERR_UNAUTHORIZED_ACTION;
    }
    if(save != cursor && cursor != NULL)
    {
        if(start + size > cursor->start)
        {
            return OS_ERR_UNAUTHORIZED_ACTION;
        }
    }

    /* End of list */
    if(cursor == NULL)
    {
        /* Try to merge */
        if(save->start + save->size != start)
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = NULL;
            new_node->prev = save;
            save->next     = new_node;
        }
        else
        {
            save->size += size;
        }
    }
    else if(cursor == save)
    {
        /* Add at the begining */
        if(start + size == cursor->start)
        {
            cursor->size += size;
            cursor->start = start;
        }
        else
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = cursor;
            new_node->prev = NULL;
            cursor->prev     = new_node;
            *list = new_node;
        }
    }
    else
    {
        /* Try to merge */
        if(save->start + save->size == start)
        {
            if(start + size == cursor->start)
            {
                save->size += size + cursor->size;
                save->next = cursor->next;

                kfree(cursor);
            }
            else
            {
                save->size += size;
            }
        }
        else if(start + size == cursor->start)
        {
            cursor->size += size;
            cursor->start = start;
        }
        else
        {
            new_node = kmalloc(sizeof(mem_area_t));
            if(new_node == NULL)
            {
                return OS_ERR_MALLOC;
            }
            new_node->size  = size;
            new_node->start = start;
            new_node->next = cursor;
            new_node->prev = save;
            save->next     = new_node;
            cursor->prev   = new_node;
        }
    }
    return OS_NO_ERR;
}

static void remove_free(mem_area_t* node, mem_area_t** list)
{
    if(node->next == NULL)
    {
        if(node->prev == NULL)
        {
            kfree(node);
            *list = NULL;
        }
        else
        {
            node->prev->next = NULL;
            kfree(node);
        }
    }
    else if(node->prev == NULL)
    {
        node->next->prev = NULL;
        *list = node->next;
        kfree(node);
    }
    else
    {
        node->next->prev = node->prev;
        node->prev->next = node->next;
        kfree(node);
    }
}

static void* get_block(OS_RETURN_E* err, mem_area_t** list)
{
    mem_area_t* cursor;
    mem_area_t* selected;
    uint32_t    address;
    /* Search for the next block with this size */
    cursor = *list;
    selected = NULL;
    while(cursor)
    {
        if(cursor->size >= KERNEL_PAGE_SIZE)
        {
            selected = cursor;
            break;
        }
        cursor = cursor->next;
    }
    if(selected == NULL)
    {
        if(err != NULL)
        {
            *err = OS_ERR_NO_MORE_FREE_MEM;
        }

        return NULL;
    }

    /* Save the block address */
    address = selected->start;

    /* Modify the block */
    selected->size -= KERNEL_PAGE_SIZE;
    selected->start += KERNEL_PAGE_SIZE;

    if(selected->size == 0)
    {
        remove_free(selected, list);
    }

    if(err != NULL)
    {
        *err = OS_NO_ERR;
    }
    return (void*)address;
}

OS_RETURN_E paging_alloc_init(void)
{
    uint32_t    i;
    uint32_t    start;
    OS_RETURN_E err;

    free_frames = NULL;
    free_pages  = NULL;

    /* Init the free memory linked list */
    for(i = 0; i < memory_map_size; ++i)
    {
        /* Check if free */
        if(memory_map_data[i].type == 1 &&
           memory_map_data[i].limit >
            (uint32_t)&_kernel_end - KERNEL_MEM_OFFSET)
        {
            start = MAX((uint32_t)&_kernel_end - KERNEL_MEM_OFFSET,
                        memory_map_data[i].base);
            err = add_free(start,
                           memory_map_data[i].limit - start,
                           &free_frames);
            if(err != OS_NO_ERR)
            {
                return err;
            }

            #if PAGING_KERNEL_DEBUG == 1
            kernel_serial_debug("Added free frame area 0x%08x (%uB)\n",
                                start, memory_map_data[i].limit - start);
            #endif
        }
    }

    /* Init the free pages */
    /* Add kernel preceding memory */
    err = add_free(0x00000000, KERNEL_MEM_OFFSET, &free_pages);
    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Added free page area 0x%08x (%uB)\n",
                        0x00000000, KERNEL_MEM_OFFSET);
    #endif
    if(err != OS_NO_ERR)
    {
        return err;
    }

    /* Add kernel succeding memory */
    err = add_free((uint32_t)&_kernel_end,
                   0xFFFFFFFF - (uint32_t)&_kernel_end + 1,
                   &free_pages);
    #if PAGING_KERNEL_DEBUG == 1
    kernel_serial_debug("Added free page area 0x%08x (%uB)\n",
                         (uint32_t)&_kernel_end,
                         0xFFFFFFFF - (uint32_t)&_kernel_end + 1);
    #endif

    if(err != OS_NO_ERR)
    {
        return err;
    }

    return OS_NO_ERR;
}

void* paging_alloc_frame(OS_RETURN_E* err)
{
   return get_block(err, &free_frames);
}

OS_RETURN_E paging_free_frame(void* frame_addr)
{
    return add_free((uint32_t)frame_addr, KERNEL_PAGE_SIZE, &free_frames);
}

void* paging_alloc_page(OS_RETURN_E* err)
{
    return get_block(err, &free_pages);
}

OS_RETURN_E paging_free_page(void* page_addr)
{
    return add_free((uint32_t)page_addr, KERNEL_PAGE_SIZE, &free_pages);
}

/* Test Mode */
#if TEST_MODE_ENABLED == 1
const mem_area_t* paging_get_free_frames(void)
{
    return free_frames;
}

const mem_area_t* paging_get_free_pages(void)
{
    return free_pages;
}


mem_area_t* test_page;
void testmode_paging_add_page(uint32_t start, uint32_t size)
{
    add_free(start, size, &test_page);
}
mem_area_t* testmode_paging_get_area(void)
{
    return test_page;
}
#endif