
# RTLK
* RTLK is a really simple i386 kernel created for training purposes.
* RTLK supports multicore (SMP) kernel and is designed to execute in kernel mode.
* RTLK has a configuration file to adapt on every capable architecture.
* RTLK stood for for "Real Time Little Kernel", today it is not real time
anymore (was it even one day?)

----------

*RTLK Build status*

| Status | Master | Dev | 
| --- | --- | --- |
| Travis CI | [![Build Status](https://travis-ci.org/Oxmose/RTLKIM.svg?branch=master)](https://travis-ci.org/Oxmose/RTLKIM) | [![Build Status](https://travis-ci.org/Oxmose/RTLKIM.svg?branch=dev)](https://travis-ci.org/Oxmose/RTLKIM) |
| Codacy | [![Codacy Badge](https://api.codacy.com/project/badge/Grade/46c866cb90a843e7869414d64849de02)](https://www.codacy.com?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Oxmose/RTLKIM&amp;utm_campaign=Badge_Grade)| N/A |

## Why RTLK
* Just for fun.
* The code is not made to be efficient but to be readable and understandable.

## Bootloader
* RTLK uses multiboot to bootstrap the system. You should be able to use grub as bootloader.
* The command `make && make bootable` will create a bootable iso file in the iso folder.

## RTLK is NOT
* A secure kernel (user can do anything he wants)

## Features
### Synchronization
* Mutex: Non recursive/Recursive - Priority inheritance capable.
* Semaphore: FIFO based, priority is not revelant.
* Spinlocks: Disable interrupt on monocore, Test And Set on multicore.
* Message queues
* Mailboxes

### Scheduler
* Priority based scheduler.
* Round Robin for all the threads having the same priority.
* Sleep capable.
* Dynamic priority (threads can change their priority).

### x86 Support
* Basic x86 support.
* PIC support.
* IO-APIC support, disables PIC when present.
* Local APIC support, use Local APIC Timer for scheduling when detected.
* PIT support, used for scheduling when Local APIC Timer is not detected.
* RTC support.
* Basic ACPI support.
* Serial Output support.
* Interrupt API (handlers can be set by the user)
* 80x25 console drivers with 16 VGA colors support.
* Symetric multiprocessing support.
* FAT32 Filesystem with VFS layer.
* VESA Graphic support.

### Memory Management
* Heap for dynamic allocation
* Memory map

### C library Support
* RTLK API is to do
* POSIX API is to do
* C standard library is to do

## TODO / Wanna help?
There is still a lot of work to be done on this kernel, you can, of course, help
me develop it if you wish!

Give a look at the Trello to find out what I want to implement!
