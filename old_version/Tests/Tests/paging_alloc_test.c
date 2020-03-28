
#include <Lib/stdio.h>
#include <Lib/stdint.h>
#include <Lib/stddef.h>
#include <IO/kernel_output.h>
#include <Tests/test_bank.h>
#include <Memory/paging_alloc.h>

#if PAGING_ALLOC_TEST == 1
extern void testmode_paging_add_page(address_t start, uint64_t size);
extern mem_area_t* testmode_paging_get_area(void);
extern const mem_area_t* paging_get_free_frames(void);
extern const mem_area_t* paging_get_free_pages(void);
void paging_alloc_test(void)
{

    const mem_area_t* frames;
    const mem_area_t* pages;
    const mem_area_t* cursor;

    kernel_printf("[TESTMODE] Paging Alloc Tests\n");

    frames = paging_get_free_frames();
    pages = paging_get_free_pages();

    kernel_printf("\n[TESTMODE] Init page, frame list \n");

    cursor = pages;
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    cursor = frames;
    while(cursor)
    {
        kernel_printf("[TESTMODE] Frame range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n[TESTMODE] Test pages \n");

    testmode_paging_add_page(4, 5LL);
    testmode_paging_add_page(13, 20LL);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(10, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(11, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(9, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(3, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(12, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(1, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n --- \n");

    testmode_paging_add_page(0, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }


    kernel_printf("\n --- \n");

    testmode_paging_add_page(101, 1);

    cursor = testmode_paging_get_area();
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_printf("\n[TESTMODE]Now testing frame allocation \n");
    uint32_t* frame;
    kernel_printf("[TESTMODE]Silent alloc\n");
    for(uint32_t i = 0; i < 100; ++i)
    {
        frame = kernel_paging_alloc_frames(1, NULL);
    }
    for(uint32_t i = 0; i < 30; ++i)
    {
        frame = kernel_paging_alloc_frames(1, NULL);
        kernel_printf("[TESTMODE]Allocated 0x%08x\n", frame);
    }

    kernel_paging_free_frames((void*)0x03FDD000, 1);
    kernel_paging_free_frames((void*)0x03FDA000, 1);

    frames = paging_get_free_frames();

    cursor = frames;
    while(cursor)
    {
        kernel_printf("[TESTMODE] Frame range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_paging_free_frames((void*)0x03FDB000, 1);
    kernel_paging_free_frames((void*)0x03FDC000, 1);

    frames = paging_get_free_frames();

    kernel_printf(" --- \n");

    cursor = frames;
    while(cursor)
    {
        kernel_printf("[TESTMODE] Frame range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    frame = kernel_paging_alloc_frames(1, NULL);
    kernel_printf("[TESTMODE]Allocated 0x%08x\n", frame);

    kernel_paging_free_frames((void*)0x03FD1000, 1);

    frame = kernel_paging_alloc_frames(1, NULL);
    kernel_printf("[TESTMODE]Allocated 0x%08x\n", frame);



    kernel_printf("\n[TESTMODE]Now testing page allocation \n");
    uint32_t* page;
    kernel_printf("[TESTMODE]Silent alloc\n");
    for(uint32_t i = 0; i < 100 - 10; ++i)
    {
        page = kernel_paging_alloc_pages(1, NULL);
    }
    for(uint32_t i = 0; i < 11; ++i)
    {
        page = kernel_paging_alloc_pages(1, NULL);
        kernel_printf("[TESTMODE]Allocated 0x%08x\n", page);
    }


    kernel_paging_free_pages((void*)0xE3FDD000, 1);
    kernel_paging_free_pages((void*)0xE3FDA000, 1);

    pages = paging_get_free_pages();

    cursor = pages;
    while(cursor)
    {
        kernel_printf("[TESTMODE] Page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    kernel_paging_free_pages((void*)0xE3FDB000, 1);
    kernel_paging_free_pages((void*)0xE3FDC000, 1);

    pages = paging_get_free_pages();

    kernel_printf(" --- \n");

    cursor = pages;
    while(cursor)
    {
        kernel_printf("[TESTMODE] page range 0x%08x -> 0x%08x\n",
        cursor->start, cursor->start + cursor->size);
        cursor = cursor->next;
    }

    page = kernel_paging_alloc_pages(1, NULL);
    kernel_printf("[TESTMODE]Allocated 0x%08x\n", page);

    kernel_paging_free_pages((void*)0xE3FD1000, 1);

    page = kernel_paging_alloc_pages(1, NULL);
    kernel_printf("[TESTMODE]Allocated 0x%08x\n", page);


    while(1)
    {
        __asm__ ("hlt");
    }
}
#else
void paging_alloc_test(void)
{
}
#endif