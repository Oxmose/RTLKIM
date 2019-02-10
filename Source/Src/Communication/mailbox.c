/***************************************************************************//**
 * @file mailbox.c
 *
 * @see mailbox.h
 *
 * @author Alexy Torres Aurora Dugo
 *
 * @date 16/12/2017
 *
 * @version 3.0
 *
 * @brief Mailbox communication and synchronization primitive.
 *
 * @details Mailbox used to send single messages between threads. The mailboxes
 * will block the threads when either full (on a sending thread) or empty (on a
 * receiving thread). The synchronization method used is the semaphore.
 *
 * @warning Mailboxes can only be used when the current system is running and
 * the scheduler initialized.
 *
 * @copyright Alexy Torres Aurora Dugo
 ******************************************************************************/

#include <Lib/stddef.h>       /* OS_RETURN_E */
#include <Lib/stdint.h>       /* Generic int types */
#include <Lib/string.h>       /* memset */
#include <IO/kernel_output.h> /* kernel_error */
#include <Interrupt/panic.h>  /* kernel_panic */
#include <Sync/semaphore.h>   /* semaphore_t */
#include <Sync/critical.h>    /* ENTER_CRITICAL, EXIT_CRITICAL */

/* RTLK configuration file */
#include <config.h>

/* Header file */
#include <Communication/mailbox.h>

/*******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * FUNCTIONS
 ******************************************************************************/

OS_RETURN_E mailbox_init(mailbox_t* mailbox)
{
    OS_RETURN_E err;

    /* Pointer check */
    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    /* Init the mailbox */
    memset(mailbox, 0, sizeof(mailbox_t));

    INIT_SPINLOCK(&mailbox->lock);

    err = sem_init(&mailbox->mailbox_sem_read, 0);
    if(err != OS_NO_ERR)
    {
        return err;
    }
    err = sem_init(&mailbox->mailbox_sem_write, 1);
    if(err != OS_NO_ERR)
    {
        err = sem_destroy(&mailbox->mailbox_sem_read);
        if(err != OS_NO_ERR)
        {
            return err;
        }
        return err;
    }

    mailbox->init = 1;

    #if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%08x INIT\n", (uint32_t)mailbox);
    #endif

    return OS_NO_ERR;
}

OS_RETURN_E mailbox_destroy(mailbox_t* mailbox)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%08x DESTROY\n", (uint32_t)mailbox);
    #endif

    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(mailbox->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &mailbox->lock);
        #else
        EXIT_CRITICAL(word);
        #endif
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    /* Set the mailbox to a destroyed state */
    mailbox->init  = 0;

    err = sem_destroy(&mailbox->mailbox_sem_read);
    err |= sem_destroy(&mailbox->mailbox_sem_write);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    return err;
}

void* mailbox_pend(mailbox_t* mailbox, OS_RETURN_E* error)
{
    OS_RETURN_E err;
    void*       ret_val;
    uint32_t    word;

    #if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%08x PEND\n", (uint32_t)mailbox);
    #endif

    /* Check mailbox pointer */
    if(mailbox == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Check for mailbox initialization */
    if(mailbox->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &mailbox->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }

        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    /* If the mailbox is empty block thread */
    err = sem_pend(&mailbox->mailbox_sem_read);
    if(err != OS_NO_ERR)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(mailbox->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &mailbox->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    /* Get mailbox value */
    ret_val = mailbox->value;

    err = sem_post(&mailbox->mailbox_sem_write);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    if(err != OS_NO_ERR)
    {
        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }
        return NULL;
    }

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    #if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%08x ACQUIRED\n", (uint32_t)mailbox);
    #endif

    return ret_val;
}

OS_RETURN_E mailbox_post(mailbox_t* mailbox, void* element)
{
    OS_RETURN_E err;
    uint32_t    word;

    #if MAILBOX_KERNEL_DEBUG == 1
    kernel_serial_debug("Mailbox 0x%08x POST\n", (uint32_t)mailbox);
    #endif

    if(mailbox == NULL)
    {
        return OS_ERR_NULL_POINTER;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(mailbox->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &mailbox->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    err = sem_pend(&mailbox->mailbox_sem_write);
    if(err != OS_NO_ERR)
    {
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    /* Set value of the mailbox */
    mailbox->value = element;

    err = sem_post(&mailbox->mailbox_sem_read);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    if(err != OS_NO_ERR)
    {
        return OS_ERR_MAILBOX_NON_INITIALIZED;
    }

    return OS_NO_ERR;
}

int32_t mailbox_isempty(mailbox_t* mailbox, OS_RETURN_E* error)
{
    int32_t   ret;
    uint32_t word;

    if(mailbox == NULL)
    {
        if(error != NULL)
        {
            *error = OS_ERR_NULL_POINTER;
        }

        return -1;
    }

    #if MAX_CPU_COUNT > 1
    ENTER_CRITICAL(word, &mailbox->lock);
    #else
    ENTER_CRITICAL(word);
    #endif

    if(mailbox->init != 1)
    {
        #if MAX_CPU_COUNT > 1
        EXIT_CRITICAL(word, &mailbox->lock);
        #else
        EXIT_CRITICAL(word);
        #endif

        if(error != NULL)
        {
            *error = OS_ERR_MAILBOX_NON_INITIALIZED;
        }

        return -1;
    }

    ret = (mailbox->mailbox_sem_read.sem_level == 0);

    #if MAX_CPU_COUNT > 1
    EXIT_CRITICAL(word, &mailbox->lock);
    #else
    EXIT_CRITICAL(word);
    #endif

    if(error != NULL)
    {
        *error = OS_NO_ERR;
    }

    return ret;
}