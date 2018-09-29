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

/* Set here the tests that should run */
#define LOADER_OK_TEST 0
#define IDT_OK_TEST 0
#define GDT_OK_TEST 0
#define OUTPUT_TEST 0
#define PIC_DRIVER_TEST 0
#define INTERRUPT_OK_TEST 0
#define EXCEPTION_OK_TEST 0
#define PANIC_TEST 0
#define PIT_DRIVER_TEST 0

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

#endif /* __TEST_BANK_H_ */