/***************************************************************************//**
 * @file test_bank.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 22/09/2018
 *
 * @version 1.0
 *
 * @brief Kernel's main test bank.
 * 
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#ifndef __TEST_BANK_H_
#define __TEST_BANK_H_

/* Set here the test that should run (only one at a time) */
#define LOADER_OK_TEST 0
#define IDT_OK_TEST 0
#define GDT_OK_TEST 0
#define OUTPUT_TEST 0
#define PIC_DRIVER_TEST 0
#define INTERRUPT_OK_TEST 0
#define EXCEPTION_OK_TEST 0
#define PANIC_TEST 0
#define PIT_DRIVER_TEST 0
#define RTC_DRIVER_TEST 0
#define TIME_OK_TEST 0
#define BIOS_CALL_TEST 0
#define KHEAP_TEST 0
#define VGA_TEXT_TEST 0
#define VESA_TEXT_TEST 0
#define IO_APIC_DRIVER_TEST 0
#define LAPIC_TIMER_DRIVER_TEST 0
#define LAPIC_DRIVER_TEST 0
#define ATA_PIO_DRIVER_TEST 0
#define KERNEL_QUEUE_TEST 0

/* Put tests declarations here */
void loader_ok_test(void);
void idt_ok_test(void);
void gdt_ok_test(void);
void output_test(void);
void pic_driver_test(void);
void interrupt_ok_test(void);
void exception_ok_test(void);
void panic_test(void);
void pit_driver_test(void);
void rtc_driver_test(void);
void time_ok_test(void);
void bios_call_test(void);
void kheap_test(void);
void vga_text_test(void);
void vesa_text_test(void);
void io_apic_driver_test(void);
void lapic_timer_driver_test(void);
void lapic_driver_test(void);
void ata_pio_driver_test(void);
void kernel_queue_test(void);

#endif /* __TEST_BANK_H_ */