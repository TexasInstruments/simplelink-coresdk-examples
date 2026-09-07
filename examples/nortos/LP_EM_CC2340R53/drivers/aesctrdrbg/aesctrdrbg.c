/*
 * Copyright (c) 2026 Texas Instruments Incorporated
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
 *  ======== aesctrdrbg.c ========
 */
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/AESCTRDRBG.h>
#include <ti/display/Display.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/cryptoutils/utils/CryptoUtils.h>
#include <ti/devices/DeviceFamily.h>
#include "aesctrdrbgTestVectors.h"

/* Driver configuration */
#include "ti_drivers_config.h"

/* Defines */
#define MAX_MSG_LENGTH       384
#define MAX_PLAINTEXT_LENGTH 144
// #define NUM_BANNER_CHARS         60

/* Display pre-formatted strings */
static const char promptStartup[]               = "\nAESCTRDRBG Driver encrypt demo.\n";
static const char promptDesc[]                  = "This example will run NIST SP800-90A test vectors.\n";
static const char promptCount[]                 = "\nCOUNT: %d";
static const char promptEntropyInput[]          = "EntropyInput           = %s";
static const char promptPersonalizationString[] = "PersonalizationString  = %s";
static const char promptEntropyInputReseed[]    = "EntropyInputReseed     = %s";
static const char promptReturnedBits[]          = "ReturnedBits           = %s";
static const char promptExpectedReturnedBits[]  = "ExpectedReturnedBits   = %s";
static const char promptSuccess[]               = "\nAESCTRDRBG demo Success!\n";
static const char promptFail[]                  = "\nFAILURE!!!\n";

static char formattedMsg[MAX_MSG_LENGTH];

uint8_t returnedEntropy[MAX_PLAINTEXT_LENGTH] __attribute__((aligned(4)));

/*
 *  ======== printHexMessage ========
 */
void printHexMessage(Display_Handle displayHandle, const char *msgTitle, const uint8_t *msgOut, uint8_t msgLen)
{
    memset(&formattedMsg[0], 0, sizeof(formattedMsg));
    /* Format the message as printable hex */
    for (uint32_t j = 0U; j < msgLen; j++)
    {
        sprintf(formattedMsg + (j * 2), "%02X", *(msgOut + j));
    }

    Display_printf(displayHandle, 0U, 0U, msgTitle, formattedMsg);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Drivers handles. */
    AESCTRDRBG_Params params;
    AESCTRDRBG_Handle aesHandle;
    Display_Handle display;

    /* AES CTR_DRBG variables. */
    int_fast16_t aesStatus;
    uint32_t i = 0;

    /* Instantiate drivers instances. */
    Display_init();
    GPIO_init();

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);

    if (display == NULL)
    {
        /* Failed to open display driver */
        while (1) {}
    }

    /* Configure the red LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Configure the green LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on the red LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Turn off the green LED */
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);

    /* Print out the top banner. */
    Display_printf(display, 0U, 0U, promptStartup);

    Display_printf(display, 0U, 0U, promptDesc);

    for (i = 0; i < getBytesVectorsCount; i++)
    {
        /* Display test vector input parameters. */
        Display_printf(display, 0U, 0U, promptCount, i);
        printHexMessage(display, promptEntropyInput, getBytesVectors[i].entropyInput, getBytesVectors[i].entropyLength);
        printHexMessage(display,
                        promptPersonalizationString,
                        getBytesVectors[i].personalizationString,
                        getBytesVectors[i].personalizationStringLength);
        printHexMessage(display,
                        promptEntropyInputReseed,
                        getBytesVectors[i].reseedEntropy,
                        getBytesVectors[i].entropyLength);
        printHexMessage(display,
                        promptExpectedReturnedBits,
                        getBytesVectors[i].expectedReturnBytes,
                        getBytesVectors[i].returnBytesLength);

        /* Call driver initialization functions */
        AESCTRDRBG_init();
        AESCTRDRBG_Params_init(&params);

        params.keyLength                 = getBytesVectors[i].keyLength;
        params.reseedInterval            = ~((uint32_t)0);
        params.seed                      = getBytesVectors[i].entropyInput;
        params.personalizationData       = getBytesVectors[i].personalizationString;
        params.personalizationDataLength = getBytesVectors[i].personalizationStringLength;
        params.returnBehavior            = AESCTRDRBG_RETURN_BEHAVIOR_POLLING;

        /* Open AESCTRDRBG Driver interface */
        aesHandle = AESCTRDRBG_open(CONFIG_AESCTRDRBG_0, &params);

        /* Check to make sure that we have a valid handle to AES Driver */
        if (aesHandle == NULL)
        {
            /* AESCTRDRBG_open() failed */
            Display_printf(display, 0U, 0U, promptFail);

            AESCTRDRBG_close(aesHandle);

            while (1)
            {
                sleep(1);
                GPIO_toggle(CONFIG_GPIO_LED_0);
            }
        }

        aesStatus = AESCTRDRBG_reseed(aesHandle,
                                      getBytesVectors[i].reseedEntropy,
                                      getBytesVectors[i].additionalReseedData,
                                      getBytesVectors[i].additionalReseedDataLength);

        /* Check to make sure that AESCTRDRBG_reseed() returned success */
        if (aesStatus != AESCTRDRBG_STATUS_SUCCESS)
        {
            /* AESCTRDRBG_reseed() failed */
            Display_printf(display, 0U, 0U, promptFail);

            AESCTRDRBG_close(aesHandle);

            while (1)
            {
                sleep(1);
                GPIO_toggle(CONFIG_GPIO_LED_0);
            }
        }

        aesStatus = AESCTRDRBG_getRandomBytes(aesHandle, returnedEntropy, getBytesVectors[i].returnBytesLength);

        /* Check to make sure that AESCTRDRBG_getRandomBytes() returned success */
        if (aesStatus != AESCTRDRBG_STATUS_SUCCESS)
        {
            /* AESCTRDRBG_getRandomBytes() failed */
            Display_printf(display, 0U, 0U, promptFail);

            AESCTRDRBG_close(aesHandle);

            while (1)
            {
                sleep(1);
                GPIO_toggle(CONFIG_GPIO_LED_0);
            }
        }

        aesStatus = AESCTRDRBG_getRandomBytes(aesHandle, returnedEntropy, getBytesVectors[i].returnBytesLength);

        /* Check to make sure that AESCTRDRBG_getRandomBytes() returned success */
        if (aesStatus != AESCTRDRBG_STATUS_SUCCESS)
        {
            /* AESCTRDRBG_getRandomBytes() failed */
            Display_printf(display, 0U, 0U, promptFail);

            AESCTRDRBG_close(aesHandle);

            while (1)
            {
                sleep(1);
                GPIO_toggle(CONFIG_GPIO_LED_0);
            }
        }

        /* Constant time comparison of output entropy versus expected entropy. */
        if (!CryptoUtils_buffersMatch(&returnedEntropy[0],
                                      &getBytesVectors[i].expectedReturnBytes[0],
                                      getBytesVectors[i].returnBytesLength))
        {
            /* AESCTRDRBG_getRandomBytes() failed to output expected entropy. */
            Display_printf(display, 0U, 0U, promptFail);

            AESCTRDRBG_close(aesHandle);

            while (1)
            {
                sleep(1);
                GPIO_toggle(CONFIG_GPIO_LED_0);
            }
        }
        else
        {
            printHexMessage(display, promptReturnedBits, returnedEntropy, getBytesVectors[i].returnBytesLength);
        }

        AESCTRDRBG_close(aesHandle);
    }

    Display_printf(display, 0U, 0U, promptSuccess);

    /* Turn off red LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);

    /* Turn on green LED */
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);

    return 0;
}
