/***************************************************************************//**
 * @file cpu.h
 *
 * @see cpu.c
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 21/03/2020
 *
 * @version 1.0
 *
 * @brief X86 CPU management functions
 *
 * @details X86 CPU manipulation functions. Wraps inline assembly calls for ease
 * of development.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __CPU_H_
#define __CPU_H_

#include <Lib/stdint.h> /* Generic int types */
#include <Lib/stddef.h> /* OS_RETURN_E */

/*******************************************************************************
 * CONSTANTS
 ******************************************************************************/

/** @brief CPU flags interrupt enabled flag. */
#define CPU_RFLAGS_IF 0x000000200
/** @brief CPU flags interrupt enabled bit shift. */
#define CPU_RFLAGS_IF_SHIFT 9

/** @brief CPUID capable flags. */
#define CPU_FLAG_CPU_CPUID_CAPABLE 0x00200000

/****************************
 * General Features
 ***************************/

/** @brief CPUID Streaming SIMD Extensions 3 flag. */
#define ECX_SSE3      (1 << 0)
/** @brief CPUID PCLMULQDQ Instruction flag. */
#define ECX_PCLMULQDQ (1 << 1)
/** @brief CPUID 64-Bit Debug Store Area flag. */
#define ECX_DTES64    (1 << 2)
/** @brief CPUID MONITOR/MWAIT flag. */
#define ECX_MONITOR   (1 << 3)
/** @brief CPUID CPL Qualified Debug Store flag. */
#define ECX_DS_CPL    (1 << 4)
/** @brief CPUID Virtual Machine Extensions flag. */
#define ECX_VMX       (1 << 5)
/** @brief CPUID Safer Mode Extensions flag. */
#define ECX_SMX       (1 << 6)
/** @brief CPUID Enhanced SpeedStep Technology flag. */
#define ECX_EST       (1 << 7)
/** @brief CPUID Thermal Monitor 2 flag. */
#define ECX_TM2       (1 << 8)
/** @brief CPUID Supplemental Streaming SIMD Extensions 3 flag. */
#define ECX_SSSE3     (1 << 9)
/** @brief CPUID L1 Context ID flag. */
#define ECX_CNXT_ID   (1 << 10)
/** @brief CPUID Fused Multiply Add flag. */
#define ECX_FMA       (1 << 12)
/** @brief CPUID CMPXCHG16B Instruction flag. */
#define ECX_CX16      (1 << 13)
/** @brief CPUID xTPR Update Control flag. */
#define ECX_XTPR      (1 << 14)
/** @brief CPUID Perf/Debug Capability MSR flag. */
#define ECX_PDCM      (1 << 15)
/** @brief CPUID Process-context Identifiers flag. */
#define ECX_PCID      (1 << 17)
/** @brief CPUID Direct Cache Access flag. */
#define ECX_DCA       (1 << 18)
/** @brief CPUID Streaming SIMD Extensions 4.1 flag. */
#define ECX_SSE41     (1 << 19)
/** @brief CPUID Streaming SIMD Extensions 4.2 flag. */
#define ECX_SSE42     (1 << 20)
/** @brief CPUID Extended xAPIC Support flag. */
#define ECX_X2APIC    (1 << 21)
/** @brief CPUID MOVBE Instruction flag. */
#define ECX_MOVBE     (1 << 22)
/** @brief CPUID POPCNT Instruction flag. */
#define ECX_POPCNT    (1 << 23)
/** @brief CPUID Local APIC supports TSC Deadline flag. */
#define ECX_TSC       (1 << 24)
/** @brief CPUID AESNI Instruction flag. */
#define ECX_AESNI     (1 << 25)
/** @brief CPUID XSAVE/XSTOR States flag. */
#define ECX_XSAVE     (1 << 26)
/** @brief CPUID OS Enabled Extended State Management flag. */
#define ECX_OSXSAVE   (1 << 27)
/** @brief CPUID AVX Instructions flag. */
#define ECX_AVX       (1 << 28)
/** @brief CPUID 16-bit Floating Point Instructions flag. */
#define ECX_F16C      (1 << 29)
/** @brief CPUID RDRAND Instruction flag. */
#define ECX_RDRAND    (1 << 30)
/** @brief CPUID Floating-Point Unit On-Chip flag. */
#define EDX_FPU       (1 << 0)
/** @brief CPUID Virtual 8086 Mode Extensions flag. */
#define EDX_VME       (1 << 1)
/** @brief CPUID Debugging Extensions flag. */
#define EDX_DE        (1 << 2)
/** @brief CPUID Page Size Extension flag. */
#define EDX_PSE       (1 << 3)
/** @brief CPUID Time Stamp Counter flag. */
#define EDX_TSC       (1 << 4)
/** @brief CPUID Model Specific Registers flag. */
#define EDX_MSR       (1 << 5)
/** @brief CPUID Physical Address Extension flag. */
#define EDX_PAE       (1 << 6)
/** @brief CPUID Machine-Check Exception flag. */
#define EDX_MCE       (1 << 7)
/** @brief CPUID CMPXCHG8 Instruction flag. */
#define EDX_CX8       (1 << 8)
/** @brief CPUID APIC On-Chip flag. */
#define EDX_APIC      (1 << 9)
/** @brief CPUID SYSENTER/SYSEXIT instructions flag. */
#define EDX_SEP       (1 << 11)
/** @brief CPUID Memory Type Range Registers flag. */
#define EDX_MTRR      (1 << 12)
/** @brief CPUID Page Global Bit flag. */
#define EDX_PGE       (1 << 13)
/** @brief CPUID Machine-Check Architecture flag. */
#define EDX_MCA       (1 << 14)
/** @brief CPUID Conditional Move Instruction flag. */
#define EDX_CMOV      (1 << 15)
/** @brief CPUID Page Attribute Table flag. */
#define EDX_PAT       (1 << 16)
/** @brief CPUID 36-bit Page Size Extension flag. */
#define EDX_PSE36     (1 << 17)
/** @brief CPUID Processor Serial Number flag. */
#define EDX_PSN       (1 << 18)
/** @brief CPUID CLFLUSH Instruction flag. */
#define EDX_CLFLUSH   (1 << 19)
/** @brief CPUID Debug Store flag. */
#define EDX_DS        (1 << 21)
/** @brief CPUID Thermal Monitor and Clock Facilities flag. */
#define EDX_ACPI      (1 << 22)
/** @brief CPUID MMX Technology flag. */
#define EDX_MMX       (1 << 23)
/** @brief CPUID FXSAVE and FXSTOR Instructions flag. */
#define EDX_FXSR      (1 << 24)
/** @brief CPUID Streaming SIMD Extensions flag. */
#define EDX_SSE       (1 << 25)
/** @brief CPUID Streaming SIMD Extensions 2 flag. */
#define EDX_SSE2      (1 << 26)
/** @brief CPUID Self Snoop flag. */
#define EDX_SS        (1 << 27)
/** @brief CPUID Multi-Threading flag. */
#define EDX_HTT       (1 << 28)
/** @brief CPUID Thermal Monitor flag. */
#define EDX_TM        (1 << 29)
/** @brief CPUID Pending Break Enable flag. */
#define EDX_PBE       (1 << 31)

/****************************
 * Extended Features
 ***************************/
/** @brief CPUID SYSCALL/SYSRET flag. */
#define EDX_SYSCALL   (1 << 11)
/** @brief CPUID Multiprocessor flag. */
#define EDX_MP        (1 << 19)
/** @brief CPUID Execute Disable Bit flag. */
#define EDX_XD        (1 << 20)
/** @brief CPUID MMX etended flag. */
#define EDX_MMX_EX    (1 << 22)
/** @brief CPUID FXSAVE/STOR available flag. */
#define EDX_FXSR     (1 << 24)
/** @brief CPUID FXSAVE/STOR optimized flag. */
#define EDX_FXSR_OPT  (1 << 25)
/** @brief CPUID 1 GB Pages flag. */
#define EDX_1GB_PAGE  (1 << 26)
/** @brief CPUID RDTSCP and IA32_TSC_AUX flag. */
#define EDX_RDTSCP    (1 << 27)
/** @brief CPUID 64-bit Architecture flag. */
#define EDX_64_BIT    (1 << 29)
/** @brief CPUID 3D Now etended flag. */
#define EDX_3DNOW_EX  (1 << 30)
/** @brief CPUID 3D Now flag. */
#define EDX_3DNOW     (1 << 31)
/** @brief CPUID LAHF Available in long mode flag */
#define ECX_LAHF_LM   (1 << 0)
/** @brief CPUID Hyperthreading not valid flag */
#define ECX_CMP_LEG   (1 << 1)
/** @brief CPUID Secure Virtual Machine flag */
#define ECX_SVM       (1 << 2)
/** @brief CPUID Extended API space flag */
#define ECX_EXTAPIC   (1 << 3)
/** @brief CPUID CR8 in protected mode flag */
#define ECX_CR8_LEG   (1 << 4)
/** @brief CPUID ABM available flag */
#define ECX_ABM       (1 << 5)
/** @brief CPUID SSE4A flag */
#define ECX_SSE4A     (1 << 6)
/** @brief CPUID Misaligne SSE mode flag */
#define ECX_MISASSE   (1 << 7)
/** @brief CPUID Prefetch flag */
#define ECX_PREFETCH  (1 << 8)
/** @brief CPUID OS Visible workaround flag */
#define ECX_OSVW      (1 << 9)
/** @brief CPUID Instruction based sampling flag */
#define ECX_IBS       (1 << 10)
/** @brief CPUID XIO intruction set flag */
#define ECX_XOP       (1 << 11)
/** @brief CPUID SKINIT instructions flag */
#define ECX_SKINIT    (1 << 12)
/** @brief CPUID watchdog timer flag */
#define ECX_WDT       (1 << 13)
/** @brief CPUID Light weight profiling flag */
#define ECX_LWP       (1 << 15)
/** @brief CPUID 4 operand fuxed multiply add flag */
#define ECX_FMA4      (1 << 16)
/** @brief CPUID Translation cache extension flag */
#define ECX_TCE       (1 << 17)
/** @brief CPUID NODE_ID MSR flag */
#define ECX_NODEIDMSR (1 << 19)
/** @brief CPUID Trailing bit manipulation flag */
#define ECX_TBM       (1 << 21)
/** @brief CPUID Topology extension flag */
#define ECX_TOPOEX    (1 << 22)
/** @brief CPUID Core performance counter extensions flag */
#define ECX_PERF_CORE (1 << 23)
/** @brief CPUID NB performance counter extensions flag */
#define ECX_PERF_NB   (1 << 24)
/** @brief CPUID Data breakpoint extensions flag */
#define ECX_DBX       (1 << 26)
/** @brief CPUID Performance TSC flag */
#define ECX_PERF_TSC  (1 << 27)
/** @brief CPUID L2I perf counter extensions flag */
#define ECX_PCX_L2I   (1 << 28)

/****************************
 * CPU Vendor signatures
 ***************************/

/** @brief CPUID Vendor signature AMD EBX. */
#define SIG_AMD_EBX 0x68747541
/** @brief CPUID Vendor signature AMD ECX. */
#define SIG_AMD_ECX 0x444d4163
/** @brief CPUID Vendor signature AMD EDX. */
#define SIG_AMD_EDX 0x69746e65

/** @brief CPUID Vendor signature Centaur EBX. */
#define SIG_CENTAUR_EBX   0x746e6543
/** @brief CPUID Vendor signature Centaur ECX. */
#define SIG_CENTAUR_ECX   0x736c7561
/** @brief CPUID Vendor signature Centaur EDX. */
#define SIG_CENTAUR_EDX   0x48727561

/** @brief CPUID Vendor signature Cyrix EBX. */
#define SIG_CYRIX_EBX 0x69727943
/** @brief CPUID Vendor signature Cyrix ECX. */
#define SIG_CYRIX_ECX 0x64616574
/** @brief CPUID Vendor signature Cyrix EDX. */
#define SIG_CYRIX_EDX 0x736e4978

/** @brief CPUID Vendor signature Intel EBX. */
#define SIG_INTEL_EBX 0x756e6547
/** @brief CPUID Vendor signature Intel ECX. */
#define SIG_INTEL_ECX 0x6c65746e
/** @brief CPUID Vendor signature Intel EDX. */
#define SIG_INTEL_EDX 0x49656e69

/** @brief CPUID Vendor signature TM1 EBX. */
#define SIG_TM1_EBX   0x6e617254
/** @brief CPUID Vendor signature TM1 ECX. */
#define SIG_TM1_ECX   0x55504361
/** @brief CPUID Vendor signature TM1 EDX. */
#define SIG_TM1_EDX   0x74656d73

/** @brief CPUID Vendor signature TM2 EBX. */
#define SIG_TM2_EBX   0x756e6547
/** @brief CPUID Vendor signature TM2 ECX. */
#define SIG_TM2_ECX   0x3638784d
/** @brief CPUID Vendor signature TM2 EDX. */
#define SIG_TM2_EDX   0x54656e69

/** @brief CPUID Vendor signature NSC EBX. */
#define SIG_NSC_EBX   0x646f6547
/** @brief CPUID Vendor signature NSC ECX. */
#define SIG_NSC_ECX   0x43534e20
/** @brief CPUID Vendor signature NSC EDX. */
#define SIG_NSC_EDX   0x79622065

/** @brief CPUID Vendor signature NextGen EBX. */
#define SIG_NEXGEN_EBX    0x4778654e
/** @brief CPUID Vendor signature NextGen ECX. */
#define SIG_NEXGEN_ECX    0x6e657669
/** @brief CPUID Vendor signature NextGen EDX. */
#define SIG_NEXGEN_EDX    0x72446e65

/** @brief CPUID Vendor signature Rise EBX. */
#define SIG_RISE_EBX  0x65736952
/** @brief CPUID Vendor signature Rise ECX. */
#define SIG_RISE_ECX  0x65736952
/** @brief CPUID Vendor signature Rise EDX. */
#define SIG_RISE_EDX  0x65736952

/** @brief CPUID Vendor signature SIS EBX. */
#define SIG_SIS_EBX   0x20536953
/** @brief CPUID Vendor signature SIS ECX. */
#define SIG_SIS_ECX   0x20536953
/** @brief CPUID Vendor signature SIS EDX. */
#define SIG_SIS_EDX   0x20536953

/** @brief CPUID Vendor signature UMC EBX. */
#define SIG_UMC_EBX   0x20434d55
/** @brief CPUID Vendor signature UMC ECX. */
#define SIG_UMC_ECX   0x20434d55
/** @brief CPUID Vendor signature UMC EDX. */
#define SIG_UMC_EDX   0x20434d55

/** @brief CPUID Vendor signature VIA EBX. */
#define SIG_VIA_EBX   0x20414956
/** @brief CPUID Vendor signature VIA ECX. */
#define SIG_VIA_ECX   0x20414956
/** @brief CPUID Vendor signature VIA EDX. */
#define SIG_VIA_EDX   0x20414956

/** @brief CPUID Vendor signature Vortex EBX. */
#define SIG_VORTEX_EBX    0x74726f56
/** @brief CPUID Vendor signature Vortex ECX. */
#define SIG_VORTEX_ECX    0x436f5320
/** @brief CPUID Vendor signature Vortex EDX. */
#define SIG_VORTEX_EDX    0x36387865

/*******************************************************************************
 * STRUCTURES
 ******************************************************************************/

/**
 * @brief CPUID Information data structure. Stores the data returned by a CPUID
 * instrucion.
 */
struct cpu_info
{
    /** @brief 32 Bits cpu flags. */
    int64_t cpu_flags;

    /**
     * @brief Returned CPUID data, [0] contains ECX value and [1] contains EDX
     * value.
     */
    uint32_t cpuid_data[2];
};

/** @brief Defines cpu_info_t type as a shorcut for struct cpu_info. */
typedef struct cpu_info cpu_info_t;

/** @brief Defines The different CPUID requests. */
enum CPUID_REQ
{
    /** @brief Request vendor string. */
    CPUID_GETVENDORSTRING,
    /** @brief Request capabled CPUID features. */
    CPUID_GETFEATURES,
    /** @brief Request TLB. */
    CPUID_GETTLB,
    /** @brief Request serial. */
    CPUID_GETSERIAL,

    /** @brief Request extended CPUID features. */
    CPUID_INTELEXTENDED_AVAILABLE=0x80000000,
    /** @brief Request Intel CPUID features. */
    CPUID_INTELFEATURES,
    /** @brief Request Intel brand string. */
    CPUID_INTELBRANDSTRING,
    /** @brief Request Intel brand string extended. */
    CPUID_INTELBRANDSTRINGMORE,
    /** @brief Request Intel brand string end. */
    CPUID_INTELBRANDSTRINGEND,
};

/** @brief Defines CPUID_REQ_E type as a shorcut for enum CPUID_REQ. */
typedef enum cpuid_requests CPUID_REQ_E;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

/**
 * @brief Fills the structure in parameters with the CPU information.
 *
 * @details The function will copy the data gathered at boot to the buffer
 * given as parameter. If the pointer to the buffer is NULL then the function
 * returns an error.
 *
 * @param[out] info The pointer to the structure to receive the data.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_NULL_POINTER is returned if info if NULL.
 *
 * @warning The function cpu_detect() must have been called at least once before
 * using cpu_get_info. Otherwise the data gatered are undefined.
 */
OS_RETURN_E cpu_get_info(cpu_info_t* info);


/**
 * @brief Return the SSE state.
 *
 * @details Return the SSE state.
 *
 * @return The function return 0 if SSE is not enabled, 1 otherwise.
 */
uint8_t cpu_is_sse_enabled(void);

/**
 * @brief Returns 1 if the CPUID intruction is available on the CPU. 0 is
 * returned otherwise.
 *
 * @return 1 if the CPUID instruction is available, 0 otherwise.
 */
int32_t cpu_cpuid_capable(void);

/**
 * @brief Detects CPU features and save then in the system's cpu_info_t
 * structure. Print the data.
 *
 * @details The function requests the CPUID data to the running CPU. The data
 * are then saved in an internatl kernel's structure. The function will also
 * print the gathered data if the parameter print is not set to 0.
 *
 * @param[in] print If not set to 0, the function will print a message with
 * the collected data.
 *
 * @return The success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the kernel could not detect the
 * CPU.
 */
OS_RETURN_E cpu_detect(const uint32_t print);

/**
 * @brief Enables the SSE features of the CPU.
 * 
 * @details Enables the SSE features of the CPU. This also enables the FPU 
 * at the same time.
 * 
 * @return he success state or the error code.
 * - OS_NO_ERR is returned if no error is encountered.
 * - OS_ERR_UNAUTHORIZED_ACTION is returned if the CPU does not support SSE.
 */
OS_RETURN_E cpu_enable_sse(void);

/**
 * @brief Returns the highest support CPUID feature request ID.
 *
 * @details Returns the highest supported input value for CPUID instruction.
 * ext canbe either 0x0 or 0x80000000 to return highest supported value for
 * basic or extended CPUID information.  Function returns 0 if CPUID
 * is not supported or whatever CPUID returns in eax register.  If sig
 * pointer is non-null, then first four bytes of the SIG
 * (as found in ebx register) are returned in location pointed by sig.
 *
 * @param[in] ext The opperation code for the CPUID instruction.
 * @return The highest supported input value for CPUID instruction.
 */
__inline__ static uint32_t cpu_get_cpuid_max (const uint32_t ext)
{
    uint32_t regs[4];
    if(cpu_cpuid_capable() == 0)
    {
        return 0;
    }

    /* Host supports CPUID.  Return highest supported CPUID input value. */
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(ext));

    return regs[0];
}

/**
 * @brief Returns the CPUID data for a requested leaf.
 *
 * @details Returns CPUID data for requested CPUID leaf, as found in returned
 * eax, ebx, ecx and edx registers.  The function checks if CPUID is
 * supported and returns 1 for valid CPUID information or 0 for
 * unsupported CPUID leaf. All pointers are required to be non-null.
 *
 * @param[in] code The opperation code for the CPUID instruction.
 * @param[out] regs The register used to store the CPUID instruction return.
 * @return 1 in case of succes, 0 otherwise.
 */
__inline__ static int32_t cpu_cpuid(const uint32_t code,
                                    uint32_t regs[4])
{
    if(cpu_cpuid_capable() == 0)
    {
        return 0;
    }
    uint32_t ext = code & 0x80000000;
    uint32_t maxlevel = cpu_get_cpuid_max(ext);

    if (maxlevel == 0 || maxlevel < code)
    {
        return 0;
    }
    __asm__ __volatile__("cpuid":"=a"(*regs),"=b"(*(regs+1)),
                         "=c"(*(regs+2)),"=d"(*(regs+3)):"a"(code));
    return 1;
}

/** @brief Clears interupt bit which results in disabling interrupts. */
__inline__ static void cpu_clear_interrupt(void)
{
    __asm__ __volatile__("cli":::"memory");
}

/** @brief Sets interrupt bit which results in enabling interupts. */
__inline__ static void cpu_set_interrupt(void)
{
    __asm__ __volatile__("sti":::"memory");
}

/** @brief Halts the CPU for lower energy consuption. */
__inline__ static void cpu_hlt(void)
{
    __asm__ __volatile__ ("hlt":::"memory");
}

/**
 * @brief Returns the current CPU flags.
 *
 * @return The current CPU flags.
 */
__inline__ static uint64_t cpu_save_flags(void)
{
    uint64_t flags;

    __asm__ __volatile__(
        "pushfq\n"
        "\tpop    %0\n"
        : "=g" (flags)
        :
        : "memory"
    );

    return flags;
}

/**
 * @brief Restores CPU flags
 *
 * @param[in] flags The flags to be restored.
 */
__inline__ static void cpu_restore_flags(const uint64_t flags)
{
    __asm__ __volatile__(
        "push    %0\n"
        "\tpopfl\n"
        :
        : "g" (flags)
        : "memory"
    );
}

/**
 * @brief Writes byte on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
__inline__ static void cpu_outb(const uint8_t value, const uint16_t port)
{
    __asm__ __volatile__("outb %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Writes word on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
__inline__ static void cpu_outw(const uint16_t value, const uint16_t port)
{
    __asm__ __volatile__("outw %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Writes long on port.
 *
 * @param[in] value The value to send to the port.
 * @param[in] port The port to which the value has to be written.
 */
__inline__ static void cpu_outl(const uint32_t value, const uint16_t port)
{
    __asm__ __volatile__("outl %0, %1" : : "a" (value), "Nd" (port));
}

/**
 * @brief Reads byte on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
__inline__ static uint8_t cpu_inb(const uint16_t port)
{
    uint8_t rega;
    __asm__ __volatile__("inb %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Reads word on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
__inline__ static uint16_t cpu_inw(const uint16_t port)
{
    uint16_t rega;
    __asm__ __volatile__("inw %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Reads long on port.
 *
 * @return The value read from the port.
 *
 * @param[in] port The port to which the value has to be read.
 */
__inline__ static uint32_t cpu_inl(const uint16_t port)
{
    uint32_t rega;
    __asm__ __volatile__("inl %1,%0" : "=a" (rega) : "Nd" (port));
    return rega;
}

/**
 * @brief Compare and swap word atomicaly.
 *
 * @details This function can be used by synchronization primitive to compare
 * and swap a word atomicaly. cpu_compare_and_swap implements the usual compare
 * and swap behavior.
 *
 * @return The value of the lock, either old val if the lock was not aquired or
 * newval if the lock was aquired.
 *
 * @param[in,out] p_val The pointer to the lock.
 * @param[in] oldval The old value to swap.
 * @param[in] newval The new value to be swapped.
 */
__inline__ static uint32_t cpu_compare_and_swap(volatile uint32_t* p_val,
                                                const int oldval,
                                                const int newval)
{
    uint32_t prev;
    __asm__ __volatile__ (
            "lock cmpxchg %1, %2\n"
            "setne %%al"
                : "=a" (prev)
                : "r" (newval), "m" (*p_val), "0" (oldval)
                : "memory");
    return prev;
}

/**
 * @brief Test and set atomic operation.
 *
 * @details This function can be used by synchronization primitive to test
 * and set a word atomicaly. s implements the usual test and set behavior.
 *
 * @param[in,out] lock The spinlock to apply the test on.
 */
__inline__ static int cpu_test_and_set(volatile uint32_t* lock)
{
        return cpu_compare_and_swap(lock, 0, 1);
}

/**
 * @brief Reads the TSC value of the CPU.
 *
 * @details Reads the current value of the CPU's time-stamp counter and store
 * into EDX:EAX. The time-stamp counter contains the amount of clock ticks that
 * have elapsed since the last CPU reset. The value is stored in a 64-bit MSR
 * and is incremented after each clock cycle.
 *
 * @return The CPU's TSC time stamp.
 */
__inline__ static uint64_t cpu_rdtsc(void)
{
    uint64_t ret;
    __asm__ __volatile__ ( "rdtsc" : "=A"(ret) );
    return ret;
}

/*******************************************************************************
 * Memory mapped IOs, avoids compilers to reorganize memory access
 *
 * So instead of doing : *addr = value, do
 * mapped_io_write(addr, value)
 ******************************************************************************/

/**
 * @brief Memory mapped IO byte write access.
 *
 * @details Memory mapped IOs byte write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_8(void* volatile addr,
                                         const uint8_t value)
{
    *(volatile uint8_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO half word write access.
 *
 * @details Memory mapped IOs half word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_16(void* volatile addr,
                                          const uint16_t value)
{
    *(volatile uint16_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO word write access.
 *
 * @details Memory mapped IOs word write access. Avoids compilers to reorganize
 * memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_32(void* volatile addr,
                                          const uint32_t value)
{
    *(volatile uint32_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO double word write access.
 *
 * @details Memory mapped IOs double word write access. Avoids compilers to
 * reorganize memory access, instead of doing : *addr = value, do
 * mapped_io_write(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to write.
 * @param[in] value The value to write to the IO.
 */
__inline__ static void mapped_io_write_64(void* volatile addr,
                                          const uint64_t value)
{
    *(volatile uint64_t*)(addr) = value;
}

/**
 * @brief Memory mapped IO byte read access.
 *
 * @details Memory mapped IOs byte read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint8_t mapped_io_read_8(const volatile void* addr)
{
    return *(volatile uint8_t*)(addr);
}

/**
 * @brief Memory mapped IO half word read access.
 *
 * @details Memory mapped IOs half word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint16_t mapped_io_read_16(const volatile void* addr)
{
    return *(volatile uint16_t*)(addr);
}

/**
 * @brief Memory mapped IO word read access.
 *
 * @details Memory mapped IOs word read access. Avoids compilers to reorganize
 * memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint32_t mapped_io_read_32(const volatile void* addr)
{
    return *(volatile uint32_t*)(addr);
}

/**
 * @brief Memory mapped IO double word read access.
 *
 * @details Memory mapped IOs double word read access. Avoids compilers to
 * reorganize memory access, instead of doing : value = *addr, do
 * mapped_io_read(addr, value) to avoid instruction reorganization.
 *
 * @param[in] addr The address of the IO to read.
 */
__inline__ static uint64_t mapped_io_read_64(const volatile void* addr)
{
    return *(volatile uint64_t*)(addr);
}
#endif /* __CPU_H_ */
