
  

# Real Time Little Kernel

* RTLK is a simple x86 kernel created for training and educational purposes. The final version is aimed to supports multicore (SMP) architectures. RTLK is is designed to execute in kernel mode with paging disable. However one of the roadmap objective is to bring paging to the kernel to allows it to become a higher-half kernel. The code of the kernel is not made to be efficient but to be readable and understandable.

* RTLK has a configuration file allowing the kernel to be customizable depending on the system it will run.
  

----------

  

*RTLK Build status*

  

| Status | Master | Dev |

| --- | --- | --- |

| Travis CI | [![Build Status](https://travis-ci.org/Oxmose/RTLKIM.svg?branch=master)](https://travis-ci.org/Oxmose/RTLKIM) | [![Build Status](https://travis-ci.org/Oxmose/RTLKIM.svg?branch=dev)](https://travis-ci.org/Oxmose/RTLKIM) |

| Codacy | [![Codacy Badge](https://api.codacy.com/project/badge/Grade/14abd7a3d98d40d1abeb2ba71a06e054)](https://www.codacy.com/app/Oxmose/RTLKIM?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=Oxmose/RTLKIM&amp;utm_campaign=Badge_Grade)| N/A |

  ----------

## Features

### Synchronization

* Mutex: Non recursive/Recursive - Priority inheritance capable.
* Semaphore: FIFO based, priority of the locking threads are not relevant to select the next thread to unlock.
* Spinlocks: Disable interrupt on monocore systems, Test And Set on multicore systems.
* Message queues and mailboxes

### Scheduler

* Priority based scheduler (threads can change their priority at execution time).
* Round Robin for all the threads having the same priority.

### x86 Support

* Basic x86 support (CPUID, Boot sequence, Memory detection).
* PIC and IO-APIC support (PIC is used as a fallback when IO-APIC is disabled or not present).
* Local APIC support (can be disabled manually of is disabled if not detected or IO-APIC is disabled),
* PIT support (may be used as fallback for scheduler timer when LAPIC is disabled, is not, the PIT can be used as an auxiliary timer source).
* RTC support.
* Basic ACPI support (simple parsing used to enable multicore features).
* SMBIOS support.
* Serial output support.
* Interrupt API (handlers can be set by the user).
* 80x25 16colors VGA support. VESA Graphic support. 
* Symetric multiprocessing support.
* FAT32 Filesystem with VFS layer.
* Time management API.
* Keyboard (ASCII QWERTY) and mouse drivers.

### Memory Management

* Kernel heap for dynamic memory allocation.
* Memory map detection at boot.

### C library Support

* RTLK API
* POSIX API may be added at some point.
* C standard library may be added at some point.
* 
## TODOs

There is still a lot of work to be done on this kernel, you can, of course, help
me develop it if you wish!
Give a look at the GitHub project page to find out what I want to implement!

## Why did I started RTLK
The first reason why I started to code RTLK is my passion for OSes. What can be better to learn how OSes work than creating one?

## Bootloader
RTLK uses the multiboot standard to bootstrap the system. You should then be able to use grub as bootloader.
The command `make && make bootable` will create a bootable iso file that you will be capable of using on any x86 system that RTLK supports.