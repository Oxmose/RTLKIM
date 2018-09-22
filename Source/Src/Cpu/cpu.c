/***************************************************************************//**
 * @file cpu.c
 * 
 * @see cpu.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 03/10/2017
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

#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Lib/string.h>       /* memcpy */
#include <IO/kernel_output.h> /* kernel_info */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Cpu/cpu.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/* May not be static since is used as extern in ASM */
/** @brief CPU info storage, stores basix CPU information. */
cpu_info_t cpu_info;

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E get_cpu_info(cpu_info_t* info)
{
    if(info == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    memcpy(info, &cpu_info, sizeof(cpu_info_t));

    return OS_NO_ERR;
}

int8_t cpuid_capable(void)
{
    return ((cpu_info.cpu_flags & CPU_FLAG_CPUID_CAPABLE) >> 21) & 0x1;
}

OS_RETURN_E detect_cpu(const uint8_t print)
{
    if(cpuid_capable() == 1)
    {
        /* eax, ebx, ecx, edx */
        int32_t regs[4];
        uint32_t ret;

        ret = cpuid(CPUID_GETVENDORSTRING, (uint32_t*)regs);

    
        /* Check if CPUID return more that one available function */
        if(ret != 0 && print != 0)
        {

            kernel_info("CPU Vendor: ");

            for(int8_t j = 0; j < 4; ++j)
            {
                kernel_printf("%c", (char)((regs[1] >> (j * 8)) & 0xFF));
            }
            for(int8_t j = 0; j < 4; ++j)
            {

                kernel_printf("%c", (char)((regs[3] >> (j * 8)) & 0xFF));
            }
            for(int8_t j = 0; j < 4; ++j)
            {

                kernel_printf("%c", (char)((regs[2] >> (j * 8)) & 0xFF));
            }
        }
        else if(ret == 0)
        {
            if(print != 0)
            {
                kernel_info("Failed to get CPUID data");
            }
            return OS_ERR_UNAUTHORIZED_ACTION;
        }

        /* If we have general CPUID features */
        if(ret >= 0x01)
        {
            /* Get CPU features */
            cpuid(CPUID_GETFEATURES, (uint32_t*)regs);

            /* Save and display */
            cpu_info.cpuid_data[0] = regs[2];
            cpu_info.cpuid_data[1] = regs[3];

            if(print != 0)
            {

                kernel_printf(" | Features: ");

                if((regs[2] & ECX_SSE3) == ECX_SSE3) 
                { kernel_printf("SSE3 - "); }
                if((regs[2] & ECX_PCLMULQDQ) == ECX_PCLMULQDQ) 
                { kernel_printf("PCLMULQDQ - "); }
                if((regs[2] & ECX_DTES64) == ECX_DTES64) 
                { kernel_printf("DTES64 - "); }
                if((regs[2] & ECX_MONITOR) == ECX_MONITOR) 
                { kernel_printf("MONITOR - "); }
                if((regs[2] & ECX_DS_CPL) == ECX_DS_CPL) 
                { kernel_printf("DS_CPL - "); }
                if((regs[2] & ECX_VMX) == ECX_VMX) 
                { kernel_printf("VMX - "); }
                if((regs[2] & ECX_SMX) == ECX_SMX) 
                { kernel_printf("SMX - "); }
                if((regs[2] & ECX_EST) == ECX_EST) 
                { kernel_printf("EST - "); }
                if((regs[2] & ECX_TM2) == ECX_TM2) 
                { kernel_printf("TM2 - "); }
                if((regs[2] & ECX_SSSE3) == ECX_SSSE3) 
                { kernel_printf("SSSE3 - "); }
                if((regs[2] & ECX_CNXT_ID) == ECX_CNXT_ID) 
                { kernel_printf("CNXT_ID - "); }
                if((regs[2] & ECX_FMA) == ECX_FMA) 
                { kernel_printf("FMA - "); }
                if((regs[2] & ECX_CX16) == ECX_CX16) 
                { kernel_printf("CX16 - "); }
                if((regs[2] & ECX_XTPR) == ECX_XTPR) 
                { kernel_printf("XTPR - "); }
                if((regs[2] & ECX_PDCM) == ECX_PDCM) 
                { kernel_printf("PDCM - "); }
                if((regs[2] & ECX_PCID) == ECX_PCID) 
                { kernel_printf("PCID - "); }
                if((regs[2] & ECX_DCA) == ECX_DCA) 
                { kernel_printf("DCA - "); }
                if((regs[2] & ECX_SSE41) == ECX_SSE41) 
                { kernel_printf("SSE41 - "); }
                if((regs[2] & ECX_SSE42) == ECX_SSE42) 
                { kernel_printf("SSE42 - "); }
                if((regs[2] & ECX_X2APIC) == ECX_X2APIC) 
                { kernel_printf("X2APIC - "); }
                if((regs[2] & ECX_MOVBE) == ECX_MOVBE) 
                { kernel_printf("MOVBE - "); }
                if((regs[2] & ECX_POPCNT) == ECX_POPCNT) 
                { kernel_printf("POPCNT - "); }
                if((regs[2] & ECX_TSC) == ECX_TSC) 
                { kernel_printf("TSC - "); }
                if((regs[2] & ECX_AESNI) == ECX_AESNI) 
                { kernel_printf("AESNI - "); }
                if((regs[2] & ECX_XSAVE) == ECX_XSAVE) 
                { kernel_printf("XSAVE - "); }
                if((regs[2] & ECX_OSXSAVE) == ECX_OSXSAVE) 
                { kernel_printf("OSXSAVE - "); }
                if((regs[2] & ECX_AVX) == ECX_AVX) 
                { kernel_printf("AVX - "); }
                if((regs[2] & ECX_F16C) == ECX_F16C) 
                { kernel_printf("F16C - "); }
                if((regs[2] & ECX_RDRAND) == ECX_RDRAND) 
                { kernel_printf("RDRAND - "); }
                if((regs[3] & EDX_FPU) == EDX_FPU) 
                { kernel_printf("FPU - "); }
                if((regs[3] & EDX_VME) == EDX_VME) 
                { kernel_printf("VME - "); }
                if((regs[3] & EDX_DE) == EDX_DE) 
                { kernel_printf("DE - "); }
                if((regs[3] & EDX_PSE) == EDX_PSE) 
                { kernel_printf("PSE - "); }
                if((regs[3] & EDX_TSC) == EDX_TSC) 
                { kernel_printf("TSC - "); }
                if((regs[3] & EDX_MSR) == EDX_MSR) 
                { kernel_printf("MSR - "); }
                if((regs[3] & EDX_PAE) == EDX_PAE) 
                { kernel_printf("PAE - "); }
                if((regs[3] & EDX_MCE) == EDX_MCE) 
                { kernel_printf("MCE - "); }
                if((regs[3] & EDX_CX8) == EDX_CX8) 
                { kernel_printf("CX8 - "); }
                if((regs[3] & EDX_APIC) == EDX_APIC) 
                { kernel_printf("APIC - "); }
                if((regs[3] & EDX_SEP) == EDX_SEP) 
                { kernel_printf("SEP - "); }
                if((regs[3] & EDX_MTRR) == EDX_MTRR) 
                { kernel_printf("MTRR - "); }
                if((regs[3] & EDX_PGE) == EDX_PGE) 
                { kernel_printf("PGE - "); }
                if((regs[3] & EDX_MCA) == EDX_MCA) 
                { kernel_printf("MCA - "); }
                if((regs[3] & EDX_CMOV) == EDX_CMOV) 
                { kernel_printf("CMOV - "); }
                if((regs[3] & EDX_PAT) == EDX_PAT) 
                { kernel_printf("PAT - "); }
                if((regs[3] & EDX_PSE36) == EDX_PSE36) 
                { kernel_printf("PSE36 - "); }
                if((regs[3] & EDX_PSN) == EDX_PSN) 
                { kernel_printf("PSN - "); }
                if((regs[3] & EDX_CLFLUSH) == EDX_CLFLUSH) 
                { kernel_printf("CLFLUSH - "); }
                if((regs[3] & EDX_DS) == EDX_DS) 
                { kernel_printf("DS - "); }
                if((regs[3] & EDX_ACPI) == EDX_ACPI) 
                { kernel_printf("ACPI - "); }
                if((regs[3] & EDX_MMX) == EDX_MMX) 
                { kernel_printf("MMX - "); }
                if((regs[3] & EDX_FXSR) == EDX_FXSR) 
                { kernel_printf("FXSR - "); }
                if((regs[3] & EDX_SSE) == EDX_SSE) 
                { kernel_printf("SSE - "); }
                if((regs[3] & EDX_SSE2) == EDX_SSE2) 
                { kernel_printf("SSE2 - "); }
                if((regs[3] & EDX_SS) == EDX_SS) 
                { kernel_printf("SS - "); }
                if((regs[3] & EDX_HTT) == EDX_HTT) 
                { kernel_printf("HTT - "); }
                if((regs[3] & EDX_TM) == EDX_TM) 
                { kernel_printf("TM - "); }
                if((regs[3] & EDX_PBE) == EDX_PBE) 
                { kernel_printf("EDX_PBE - "); }
            }
        }

        if(print != 0)
        {
            kernel_printf("\n");
        }
    }
    else
    {
        if(print != 0)
        {
            kernel_info("CPUID not available\n");
        }
        return OS_ERR_UNAUTHORIZED_ACTION;
    }

    return OS_NO_ERR;
}
