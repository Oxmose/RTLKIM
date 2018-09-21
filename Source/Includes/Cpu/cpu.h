/*******************************************************************************
 *
 * File: cpu.h
 *
 * Author: Alexy Torres Aurora Dugo
 *
 * Date: 03/10/2017
 *
 * Version: 1.0
 *
 * CPU management functions
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

#include <Lib/stdint.h> /* Generic int types */
#include <Lib/stddef.h> /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

#define CPU_FLAG_CPUID_CAPABLE 0x00200000

/* General Features */
#define ECX_SSE3                        (1 << 0)    /* Streaming SIMD Extensions 3 */
#define ECX_PCLMULQDQ                   (1 << 1)    /* PCLMULQDQ Instruction */
#define ECX_DTES64                      (1 << 2)    /* 64-Bit Debug Store Area */
#define ECX_MONITOR                     (1 << 3)    /* MONITOR/MWAIT */
#define ECX_DS_CPL                      (1 << 4)    /* CPL Qualified Debug Store */
#define ECX_VMX                         (1 << 5)    /* Virtual Machine Extensions */
#define ECX_SMX                         (1 << 6)    /* Safer Mode Extensions */
#define ECX_EST                         (1 << 7)    /* Enhanced SpeedStep Technology */
#define ECX_TM2                         (1 << 8)    /* Thermal Monitor 2 */
#define ECX_SSSE3                       (1 << 9)    /* Supplemental Streaming SIMD Extensions 3 */
#define ECX_CNXT_ID                     (1 << 10)   /* L1 Context ID */
#define ECX_FMA                         (1 << 12)   /* Fused Multiply Add */
#define ECX_CX16                        (1 << 13)   /* CMPXCHG16B Instruction */
#define ECX_XTPR                        (1 << 14)   /* xTPR Update Control */
#define ECX_PDCM                        (1 << 15)   /* Perf/Debug Capability MSR */
#define ECX_PCID                        (1 << 17)   /* Process-context Identifiers */
#define ECX_DCA                         (1 << 18)   /* Direct Cache Access */
#define ECX_SSE41                       (1 << 19)   /* Streaming SIMD Extensions 4.1 */
#define ECX_SSE42                       (1 << 20)   /* Streaming SIMD Extensions 4.2 */
#define ECX_X2APIC                      (1 << 21)   /* Extended xAPIC Support */
#define ECX_MOVBE                       (1 << 22)   /* MOVBE Instruction */
#define ECX_POPCNT                      (1 << 23)   /* POPCNT Instruction */
#define ECX_TSC                         (1 << 24)   /* Local APIC supports TSC Deadline */
#define ECX_AESNI                       (1 << 25)   /* AESNI Instruction */
#define ECX_XSAVE                       (1 << 26)   /* XSAVE/XSTOR States */
#define ECX_OSXSAVE                     (1 << 27)   /* OS Enabled Extended State Management */
#define ECX_AVX                         (1 << 28)   /* AVX Instructions */
#define ECX_F16C                        (1 << 29)   /* 16-bit Floating Point Instructions */
#define ECX_RDRAND                      (1 << 30)   /* RDRAND Instruction */

#define EDX_FPU                         (1 << 0)    /* Floating-Point Unit On-Chip */
#define EDX_VME                         (1 << 1)    /* Virtual 8086 Mode Extensions */
#define EDX_DE                          (1 << 2)    /* Debugging Extensions */
#define EDX_PSE                         (1 << 3)    /* Page Size Extension */
#define EDX_TSC                         (1 << 4)    /* Time Stamp Counter */
#define EDX_MSR                         (1 << 5)    /* Model Specific Registers */
#define EDX_PAE                         (1 << 6)    /* Physical Address Extension */
#define EDX_MCE                         (1 << 7)    /* Machine-Check Exception */
#define EDX_CX8                         (1 << 8)    /* CMPXCHG8 Instruction */
#define EDX_APIC                        (1 << 9)    /* APIC On-Chip */
#define EDX_SEP                         (1 << 11)   /* SYSENTER/SYSEXIT instructions */
#define EDX_MTRR                        (1 << 12)   /* Memory Type Range Registers */
#define EDX_PGE                         (1 << 13)   /* Page Global Bit */
#define EDX_MCA                         (1 << 14)   /* Machine-Check Architecture */
#define EDX_CMOV                        (1 << 15)   /* Conditional Move Instruction */
#define EDX_PAT                         (1 << 16)   /* Page Attribute Table */
#define EDX_PSE36                       (1 << 17)   /* 36-bit Page Size Extension */
#define EDX_PSN                         (1 << 18)   /* Processor Serial Number */
#define EDX_CLFLUSH                     (1 << 19)   /* CLFLUSH Instruction */
#define EDX_DS                          (1 << 21)   /* Debug Store */
#define EDX_ACPI                        (1 << 22)   /* Thermal Monitor and Software Clock Facilities */
#define EDX_MMX                         (1 << 23)   /* MMX Technology */
#define EDX_FXSR                        (1 << 24)   /* FXSAVE and FXSTOR Instructions */
#define EDX_SSE                         (1 << 25)   /* Streaming SIMD Extensions */
#define EDX_SSE2                        (1 << 26)   /* Streaming SIMD Extensions 2 */
#define EDX_SS                          (1 << 27)   /* Self Snoop */
#define EDX_HTT                         (1 << 28)   /* Multi-Threading */
#define EDX_TM                          (1 << 29)   /* Thermal Monitor */
#define EDX_PBE                         (1 << 31)   /* Pending Break Enable */

/* Extended Features */
#define EDX_SYSCALL                     (1 << 11)   /* SYSCALL/SYSRET */
#define EDX_XD                          (1 << 20)   /* Execute Disable Bit */
#define EDX_1GB_PAGE                    (1 << 26)   /* 1 GB Pages */
#define EDX_RDTSCP                      (1 << 27)   /* RDTSCP and IA32_TSC_AUX */
#define EDX_64_BIT                      (1 << 29)   /* 64-bit Architecture */

/* Signatures for different CPU implementations as returned in uses
   of cpuid with level 0.  */
#define SIG_AMD_EBX   0x68747541
#define SIG_AMD_ECX   0x444d4163
#define SIG_AMD_EDX   0x69746e65

#define SIG_CENTAUR_EBX   0x746e6543
#define SIG_CENTAUR_ECX   0x736c7561
#define SIG_CENTAUR_EDX   0x48727561

#define SIG_CYRIX_EBX 0x69727943
#define SIG_CYRIX_ECX 0x64616574
#define SIG_CYRIX_EDX 0x736e4978

#define SIG_INTEL_EBX 0x756e6547
#define SIG_INTEL_ECX 0x6c65746e
#define SIG_INTEL_EDX 0x49656e69

#define SIG_TM1_EBX   0x6e617254
#define SIG_TM1_ECX   0x55504361
#define SIG_TM1_EDX   0x74656d73

#define SIG_TM2_EBX   0x756e6547
#define SIG_TM2_ECX   0x3638784d
#define SIG_TM2_EDX   0x54656e69

#define SIG_NSC_EBX   0x646f6547
#define SIG_NSC_ECX   0x43534e20
#define SIG_NSC_EDX   0x79622065

#define SIG_NEXGEN_EBX    0x4778654e
#define SIG_NEXGEN_ECX    0x6e657669
#define SIG_NEXGEN_EDX    0x72446e65

#define SIG_RISE_EBX  0x65736952
#define SIG_RISE_ECX  0x65736952
#define SIG_RISE_EDX  0x65736952

#define SIG_SIS_EBX   0x20536953
#define SIG_SIS_ECX   0x20536953
#define SIG_SIS_EDX   0x20536953

#define SIG_UMC_EBX   0x20434d55
#define SIG_UMC_ECX   0x20434d55
#define SIG_UMC_EDX   0x20434d55

#define SIG_VIA_EBX   0x20414956
#define SIG_VIA_ECX   0x20414956
#define SIG_VIA_EDX   0x20414956

#define SIG_VORTEX_EBX    0x74726f56
#define SIG_VORTEX_ECX    0x436f5320
#define SIG_VORTEX_EDX    0x36387865

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

typedef struct cpu_info
{
    int32_t cpu_flags;

    /* [0] is ECX values from CPUID request
     * [1] is EDX
     */
    uint32_t cpuid_data[2];
} cpu_info_t;

typedef enum cpuid_requests
{
    CPUID_GETVENDORSTRING,
    CPUID_GETFEATURES,
    CPUID_GETTLB,
    CPUID_GETSERIAL,

    CPUID_INTELEXTENDED=0x80000000,
    CPUID_INTELFEATURES,
    CPUID_INTELBRANDSTRING,
    CPUID_INTELBRANDSTRINGMORE,
    CPUID_INTELBRANDSTRINGEND,
} CPUID_REQ_E;


/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/* Fill the structure in parameters with the CPU information
 *
 * @returns The succes state or the error code.
 * @param info The pointer to the structure to receive the data.
 */
OS_RETURN_E get_cpu_info(cpu_info_t* info);

/* Tell is the CPUID intruction is available on the CPU.
 *
 * @returns 1 if the cpuid instruction is available, 0 otherwise.
 */
int8_t cpuid_capable(void);

/* Detect CPU features and save then in the system cpu_info_t structure. */
void detect_cpu(void);

/* Return highest supported input value for cpuid instruction.  ext can
 * be either 0x0 or 0x80000000 to return highest supported value for
 * basic or extended cpuid information.  Function returns 0 if cpuid
 * is not supported or whatever cpuid returns in eax register.  If sig
 * pointer is non-null, then first four bytes of the SIG
 * (as found in ebx register) are returned in location pointed by sig.
 *
 * @param ext The opperation code for the CPUID instruction.
 * @returns The highest supported input value for cpuid instruction.
 */
__inline__ static uint32_t get_cpuid_max (uint32_t ext)
{
    uint32_t regs[4];
    if(cpuid_capable() == 0)
    {
        return 0;
    }

    /* Host supports cpuid.  Return highest supported cpuid input value.  */
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(ext));

    return regs[0];
}

/* Return cpuid data for requested cpuid leaf, as found in returned
 *  eax, ebx, ecx and edx registers.  The function checks if cpuid is
 * supported and returns 1 for valid cpuid information or 0 for
 * unsupported cpuid leaf.  All pointers are required to be non-null.
 *
 * @param code The opperation code for the CPUID instruction.
 * @param regs The register used to store the CPUID instruction return.
 * @returns 1 in case of succes, 0 otherwise.
 */

__inline__ static int32_t cpuid (uint32_t code,
                                 uint32_t regs[4])
{
    if(cpuid_capable() == 0)
    {
        return 0;
    }
    uint32_t ext = code & 0x80000000;
    uint32_t maxlevel = get_cpuid_max(ext);

    if (maxlevel == 0 || maxlevel < code)
    {
        return 0;
    }
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(code));
    return 1;
}

/* Clear interupt bit which results in disabling interupts */
__inline__ static void cli(void)
{
    __asm__ __volatile__("cli":::"memory");
}

/* Sets interupt bit which results in enabling interupts */
__inline__ static void sti(void)
{
    __asm__ __volatile__("sti":::"memory");
}

/* Halts the CPU for lower energy consuption */
__inline__ static void hlt(void)
{
    __asm__ __volatile__ ("hlt":::"memory");
}


/* Save CPU flags
 *
 * @returns The current flags that were saved.
 */
__inline__ static uint32_t save_flags(void)
{
    uint32_t flags;

    __asm__ __volatile__(
        "pushfl\n"
        "\tpopl    %0\n"
        : "=g" (flags)
        :
        : "memory"
    );

    return flags;
}

/* Restore CPU flags
 *
 * @param flags The flags to be restored.
 */
__inline__ static void restore_flags(uint32_t flags)
{
    __asm__ __volatile__(
        "pushl    %0\n"
        "\tpopfl\n"
        :
        : "g" (flags)
        : "memory"
    );
}

/* Write byte on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outb(uint8_t value, uint16_t port)
{
    __asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

/* Write word on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outw(uint16_t value, uint16_t port)
{
    __asm__ __volatile__("outw %0, %1" : : "a" (value), "Nd" (port));
}

/* Write long on port.
 *
 * @param value The value to send to the port.
 * @param port The port to which the value has to be written.
 */
__inline__ static void outl(uint32_t value, uint16_t port)
{
    __asm__ __volatile__("outl %0, %1" : : "a" (value), "Nd" (port));
}

/* Read byte on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint8_t inb(uint16_t port)
{
    uint8_t rega;
    __asm__ __volatile__("inb %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Read word on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint16_t inw(uint16_t port)
{
    uint16_t rega;
    __asm__ __volatile__("inw %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Read long on port.
 *
 * @returns The value read fron the port.
 * @param port The port to which the value has to be read.
 */
__inline__ static uint32_t inl(uint16_t port)
{
    uint32_t rega;
    __asm__ __volatile__("inl %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/* Compare and swap word atomicaly.
 *
 * @returns The value of the lock
 * @param p_val The pointer to the lock.
 * @param oldval The old value to swap.
 * @param newval The new value to be swapped.
 */
__inline__ static uint32_t cpu_compare_and_swap(volatile uint32_t* p_val,
        int oldval, int newval)
{
    uint8_t prev;
    __asm__ __volatile__ (
            "lock cmpxchg %1, %2\n"
            "setne %%al"
                : "=a" (prev)
                : "r" (newval), "m" (*p_val), "0" (oldval)
                : "memory");
    return prev;
}

/* Test and set atomic operation.
 *
 * @param lock The spinlock to apply the test on.
 */
__inline__ static int cpu_test_and_set(volatile uint32_t* lock)
{
        return cpu_compare_and_swap(lock, 0, 1);
}

/* Read the current value of the CPU's time-stamp counter and store into
 * EDX:EAX. The time-stamp counter contains the amount of clock ticks that have
 * elapsed since the last CPU reset. The value is stored in a 64-bit MSR and is
 * incremented after each clock cycle.
 *
 * @return The CPU's time stampe
 */
__inline__ static uint64_t rdtsc()
{
    uint64_t ret;
    __asm__ __volatile__ ( "rdtsc" : "=A"(ret) );
    return ret;
}

/*******************************************************************************
 * Memory mapped IOs, avoid compilers to reorganize memory access
 *
 * So instead of doing : *addr = value, do
 * mapped_io_write(addr, value)
 ******************************************************************************/

__inline__ static void mapped_io_write_8(void* volatile addr,
                                         const uint8_t value)
{
    *(volatile uint8_t*)(addr) = value;
}

__inline__ static void mapped_io_write_16(void* volatile addr,
                                          const uint16_t value)
{
    *(volatile uint16_t*)(addr) = value;
}

__inline__ static void mapped_io_write_32(void* volatile addr,
                                          const uint32_t value)
{
    *(volatile uint32_t*)(addr) = value;
}

__inline__ static void mapped_io_write_64(void* volatile addr,
                                          const uint64_t value)
{
    *(volatile uint64_t*)(addr) = value;
}

__inline__ static uint8_t mapped_io_read_8(const volatile void* addr)
{
    return *(volatile uint8_t*)(addr);
}

__inline__ static uint16_t mapped_io_read_16(const volatile void* addr)
{
    return *(volatile uint16_t*)(addr);
}

__inline__ static uint32_t mapped_io_read_32(const volatile void* addr)
{
    return *(volatile uint32_t*)(addr);
}

__inline__ static uint64_t mapped_io_read_64(const volatile void* addr)
{
    return *(volatile uint64_t*)(addr);
}

__inline__ static void mapped_io_read_sized(const volatile void* addr,
                                            void* value,
                                            uint32_t size)
{
    volatile uint8_t* base = (volatile uint8_t*)addr;
    uint8_t* dest = (uint8_t*)value;

    /* Tranfert memory until size limit is reached */
    while (size > 0)
    {
        *dest = *base;
        ++base;
        ++dest;
        --size;
    }
}

#endif /* __CPU_H_ */
