#include <Lib/stddef.h>
#include <Lib/stdio.h>
#include <Memory/meminfo.h>
#include <Memory/kheap.h>
#include <Core/scheduler.h>

/* Used as example, it will be changed in the future. */
int main(void)
{
    uint32_t kheap_usage;
    uint32_t kheap_size;

    uint32_t k_mem_usage;
    uint32_t k_size;

    uint32_t total_size;

    uint32_t dir = 0;
    uint32_t i = 0;

    int8_t* ptrs[1000] = {NULL};

    kheap_size = meminfo_kernel_heap_size();
    k_size     = meminfo_kernel_total_size();

    total_size = meminfo_get_memory_size();

    printf("Total memory size: %dBytes\n", total_size);
    printf("Total kernel size: %dBytes\n", k_size);
    printf("TKernel heap reserved size: %dBytes\n", kheap_size);

    while(1)
    {
        if(i % 1000 == 0)
        {
            dir = !dir;
        }
        if(dir == 0)
        {
            kfree(ptrs[i%1000]);
        }
        else 
        {
            ptrs[i%1000] = kmalloc(sizeof(uint32_t) * 1000);
        }

        if(i%25 == 0)
        {
            kheap_usage = meminfo_kernel_heap_usage();
            k_mem_usage = meminfo_kernel_memory_usage();
            uint32_t heap_usage = 100 * kheap_usage / kheap_size;
            uint32_t mem_usage = 100 * k_mem_usage / total_size;
            printf("\rMem data: KHeap usage: %d (%d%%) | KMem usage: %d (%d%%)     ", 
                   kheap_usage, heap_usage, k_mem_usage, mem_usage);
            sched_sleep(100);
        }
        ++i;
    }

    return 0;
}