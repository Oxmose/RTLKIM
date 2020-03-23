;-------------------------------------------------------------------------------
;
; File: ap_boot.S
;
; Author: Alexy Torres Aurora Dugo
;
; Date: 30/03/2018
;
; Version: 1.0
;
; Kernel entry point for AP cpus
;-------------------------------------------------------------------------------
[bits 64]
;-----------------------------------------
; EXTERN FUNCTIONS
;-----------------------------------------
extern init_cpu_count
extern init_cpu_count_phys
extern cpu_gdt_size
extern cpu_idt_size
extern ap_cpu_stacks
extern ap_cpu_stack_size
extern smp_ap_core_init
extern kernel_curr_pml4

%define CODE32 0x08
%define DATA32 0x10
%define CODE16 0x18
%define DATA16 0x20

%define CODE_LOCATION 0x4000
%define OFFSET_ADDR(addr)  (((addr) - ap_boot_loader) + CODE_LOCATION)

global ap_boot_loader_init
global ap_boot_loader

ap_boot_loader_init:
ap_boot_loader:

