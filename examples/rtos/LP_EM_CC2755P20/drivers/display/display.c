/*
 * Copyright (c) 2024-2025, Texas Instruments Incorporated
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
 *  ======== display.c ========
 */
#include <stdint.h>
#include <unistd.h>

/* TI-Drivers Header files */
#include <ti/drivers/GPIO.h>

/* Display Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart2.h>
#include <ti/display/AnsiColor.h>

/* Driver Configuration */
#include "ti_drivers_config.h"

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    unsigned int ledPinValue;
    unsigned int loopCount = 0;

    GPIO_init();
    Display_init();

    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Initialize display and try to open both UART and LCD types of display. */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode = DISPLAY_CLEAR_BOTH;

    /*
     * Open UART display.
     */
    Display_Handle hSerial = Display_open(Display_Type_UART, &params);

    if (hSerial == NULL)
    {
        /* Failed to open a display */
        while (1) {}
    }

    Display_printf(hSerial, 0, 0, "Hello Serial!");

    /* Sleep for a bit to show the hello serial message, before clearing the
     * serial display.
     */
    sleep(6);

    char *serialLedOn  = "On";
    char *serialLedOff = "Off";

    /* If serial display can handle ANSI colors, use colored strings instead.
     *
     * You configure DisplayUart to use the ANSI variant by choosing what
     * function pointer list it should use, for example:
     *
     * const Display_Config Display_config[] = {
     *   {
     *      .fxnTablePtr = &DisplayUartAnsi_fxnTable, // Alternatively: DisplayUartMin_
     *      ...
     */
    if (Display_getType(hSerial) & Display_Type_ANSI)
    {
        serialLedOn  = ANSI_COLOR(FG_GREEN, ATTR_BOLD) "On" ANSI_COLOR(ATTR_RESET);
        serialLedOff = ANSI_COLOR(FG_RED, ATTR_UNDERLINE) "Off" ANSI_COLOR(ATTR_RESET);
    }

    /* Loop forever, alternating LED state and Display output. */
    while (1)
    {
        ledPinValue = GPIO_read(CONFIG_GPIO_LED_0);

        /* Print to UART */
        Display_clearLine(hSerial, ledPinValue ? 1 : 0);
        Display_printf(hSerial,
                       ledPinValue ? 0 : 1, /* Line */
                       0,                   /* Column */
                       "LED: %s",
                       (ledPinValue == CONFIG_GPIO_LED_ON) ? serialLedOn : serialLedOff);

        /* If ANSI is supported, print a "log" in the scrolling region */
        if (Display_getType(hSerial) & Display_Type_ANSI)
        {
            char *currLedState = (ledPinValue == CONFIG_GPIO_LED_ON) ? serialLedOn : serialLedOff;
            Display_printf(hSerial, DisplayUart2_SCROLLING, 0, "[ %d ] LED: %s", loopCount++, currLedState);
        }

        sleep(1);

        /* Toggle LED */
        GPIO_toggle(CONFIG_GPIO_LED_0);
    }
}
