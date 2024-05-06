/*
 * Copyright (c) 2023, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 *  ======== s2rram.c ========
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

/* Driver Header files */
#include <ti/display/Display.h>
#include <ti/drivers/Power.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define S2RRAM_BUFFER_SIZE 0x200

/* Define buffer to be placed in S2R RAM */
#ifdef __IAR_SYSTEMS_ICC__
    #pragma location = ".s2rram"
#elif (defined(__GNUC__) || defined(__clang__))
__attribute__((section(".s2rram")))
#else
    #error Unsupported Compiler
#endif
volatile uint8_t s2rramBuffer[S2RRAM_BUFFER_SIZE];

bool verifyBuffer(uint8_t *buffer, uint8_t expectedValue, uint32_t size)
{
    for (uint32_t index = 0; index < size; index++)
    {
        if (buffer[index] != expectedValue)
        {
            return false;
        }
    }

    return true;
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Handle displayHandle;

    Display_init();

    displayHandle = Display_open(Display_Type_UART, NULL);
    if (displayHandle == NULL)
    {
        /* Display_open() failed */
        while (1) {}
    }

    Display_printf(displayHandle, 0, 0, "Starting the s2rram example");

    /* Ensure that S2R RAM is clocked */
    Power_setDependency(PowerLPF3_PERIPH_LFRD_S2RRAM);

    Display_printf(displayHandle, 0, 0, "Writing 0xAA to all bytes in S2R RAM buffer.");

    /* Write 0xAA to all bytes in the buffer in S2R RAM */
    memset((uint8_t *)s2rramBuffer, 0xAA, S2RRAM_BUFFER_SIZE);

    /* Prevent device from entering STANDBY.
     * The next statement will call sleep() which will
     * cause the device to enter STANDBY unless it has been disallowed.
     * The application cannot rely on any value in S2R RAM after the device
     * enters STANDBY. Since we want to read the data stored in S2R RAM after
     * sleeping, we need to prevent the device from entering STANDBY during
     * sleep.
     */
    Power_setConstraint(PowerLPF3_DISALLOW_STANDBY);

    /* Sleep for a second. */
    sleep(1);

    Display_printf(displayHandle, 0, 0, "Verifying S2R RAM buffer after sleeping...");

    /* Read and verify data from S2R RAM after sleeping */
    if (verifyBuffer((uint8_t *)s2rramBuffer, 0xAA, S2RRAM_BUFFER_SIZE))
    {
        Display_printf(displayHandle, 0, 0, "Verification was successful.");
    }
    else
    {
        Display_printf(displayHandle, 0, 0, "Verification was unsuccessful.");
    }

    /* Release disallow STANDBY constraint. It it no longer needed, since we
     * ensure by design that the remaining part of the application will not
     * enter STANDBY.
     */
    Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);

    /* Sleep for a second. This will now cause the device to enter STANDBY, but
     * it will not be a problem, because the application will not read the
     * contents of s2rramBuffer before writing to it below.
     */
    sleep(1);

    Display_printf(displayHandle, 0, 0, "Writing 0x55 to all bytes in S2R RAM buffer.");

    /* Write 0x55 to all bytes in the buffer in S2R RAM */
    memset((uint8_t *)s2rramBuffer, 0x55, S2RRAM_BUFFER_SIZE);

    /* Perform dummy operation known not to put the device in STANDBY */
    Display_printf(displayHandle, 0, 0, "This is a dummy operation that puts the device into Idle but not STANDBY.");
    Display_printf(displayHandle, 0, 0, "It could be any operation that is known not to put the device in STANDBY.");
    Display_printf(displayHandle, 0, 0, "For example a SPI transaction, crypto operation, etc.");

    Display_printf(displayHandle, 0, 0, "Verifying S2R RAM buffer after performing dummy operation...");

    /* Read and verify data from S2R RAM after sleeping */
    if (verifyBuffer((uint8_t *)s2rramBuffer, 0x55, S2RRAM_BUFFER_SIZE))
    {
        Display_printf(displayHandle, 0, 0, "Verification was successful.");
    }
    else
    {
        Display_printf(displayHandle, 0, 0, "Verification was unsuccessful.");
    }

    /* Remove dependency on S2R RAM, since we are not using it anymore */
    Power_releaseDependency(PowerLPF3_PERIPH_LFRD_S2RRAM);

    return (NULL);
}
