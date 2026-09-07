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
 *  ======== aescbc.c ========
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/AESCBC.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/UART2.h>
#include <ti/devices/DeviceFamily.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Defines */
#define MAX_MSG_LENGTH       272
#define MAX_PLAINTEXT_LENGTH 144
#define AESCBC_KEY_SIZE      32 /* bytes */
#define AESCBC_IV_LENGTH     32 /* bytes */
#define ASCII_ONE            0x31
#define ASCII_TWO            0x32
#define ASCII_EXIT           0x71
#define ASCII_CR             0x0D
#define NUM_BANNER_CHARS     60

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    /* HSM HW engine will be used for CC27XX device. */
    #define CryptoKeyPlaintext_initKey CryptoKeyPlaintextHSM_initKey
#endif

/* UART pre-formatted strings */
static const char promptStartup[]   = "\n\n\rAESCBC Driver encrypt demo.";
static const char promptEnter[]     = "\n\n\rThis example will encrypt and decrypt data using AESKey.\n\n\rEnter 1 for "
                                      "16 byte input\n\rEnter 2 for 128 byte input\n\rEnter q to exit\n\n\r";
static const char promptClearText[] = "\n\n\rPlaintext encrypted: \"";
static const char promptMessage[]   = "\"\n\n\rEncrypted result: \"";
static const char msgAESOpenFails[] = "\n\n\rFailed to open AES Driver!";
static const char msgEncryptFails[] = "\n\n\rFailed to encrypt message!";
static const char msgExit[]         = "\n\n\rExiting example...";

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
static const char promptMsgDecrypt[] = "\"\n\n\rDecrypted result: \"";
static const char msgDecryptFails[]  = "\n\n\rFailed to decrypt message!";
#else
static const char msgNotSupported[] = "\"\n\n\rDecryption not supported!";
#endif

/* AES CBC example structure.
 * Note plaintext is purposely word-aligned.
 * Otherwise CC23XX will fail to encrypt.
 */
typedef struct
{
    uint8_t key[AESCBC_KEY_SIZE];
    uint8_t keyLength;
    char plaintext[MAX_PLAINTEXT_LENGTH] __attribute__((aligned(4)));
    uint8_t plaintextLength;
    uint8_t iv[AESCBC_IV_LENGTH];
} CBCEncryptExample;

/* Message buffers */
static unsigned char testIndex;
static char formatedMsg[MAX_MSG_LENGTH];

/* AESCBC parameters. Word-aligned to satisfy HW limitation.
 * Both input and output needs to be word aligned for CC23XX
 * Only output needs to be word aligned for CC27XX
 */
uint8_t plaintext[MAX_PLAINTEXT_LENGTH] __attribute__((aligned(4)));
uint8_t ciphertext[MAX_PLAINTEXT_LENGTH] __attribute__((aligned(4)));

const CBCEncryptExample encryptionExampleVectors[] = {
    {
        /* Derived from Test vector 180 from NIST CAVP DVPT 256 */
        .key       = {0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0, 0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17},
        .keyLength = 16,
        .plaintext = {"TexasInstruments"},
        .plaintextLength = 16,
        .iv = {0x2f, 0xe2, 0xb3, 0x33, 0xce, 0xda, 0x8f, 0x98, 0xf4, 0xa9, 0x9b, 0x40, 0xd2, 0xcd, 0x34, 0xa8},
    },
    {
        /* Derived from Test vector 180 from NIST CAVP DVPT 256 */
        .key       = {0x1f, 0x8e, 0x49, 0x73, 0x95, 0x3f, 0x3f, 0xb0, 0xbd, 0x6b, 0x16, 0x66, 0x2e, 0x9a, 0x3c, 0x17},
        .keyLength = 16,
        .plaintext = {"Texas Instruments is a semiconductor company based in Dallas, TX. "
                      "It's a company that cares about its neighbors and communities."},
        .plaintextLength = 128,
        .iv = {0xaa, 0xd1, 0x58, 0x3c, 0xd9, 0x13, 0x65, 0xe3, 0xbb, 0x2f, 0x0c, 0x34, 0x30, 0xd0, 0x65, 0xbb},
    },
};

/*
 *  ======== printMessage ========
 */
void printMessage(UART2_Handle uartHandle, const char *msgTitle, const uint8_t *msgOut, uint8_t msgLen)
{
    uint32_t i;

    /* Print prompt */
    UART2_write(uartHandle, msgTitle, strlen(msgTitle), NULL);

    /* Format the message as printable hex */
    for (i = 0; i < msgLen; i++)
    {
        sprintf(formatedMsg + (i * 2), "%02X", *(msgOut + i));
    }

    /* Print result */
    UART2_write(uartHandle, formatedMsg, (2 * msgLen), NULL);
}

/*
 *  Print string message to console.
 *  Input params are expected to be characters.
 */
void printClearText(UART2_Handle uartHandle, const char *msgTitle, const char *msgOut, uint8_t msgLen)
{
    /* Print prompt */
    UART2_write(uartHandle, msgTitle, strlen(msgTitle), NULL);

    /* Print result */
    UART2_write(uartHandle, msgOut, msgLen, NULL);
}

/*
 *  Print a banner with the char provided.
 */
void printBanner(UART2_Handle uartHandle, const char *bannerChar)
{
    uint32_t i;

    UART2_write(uartHandle, "\"\n\r", 3, NULL);

    for (i = 0; i < NUM_BANNER_CHARS; i++)
    {
        UART2_write(uartHandle, bannerChar, strlen(bannerChar), NULL);
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    /* Driver handles */
    UART2_Handle uart2Handle;
    AESCBC_Handle aesHandle;

    /* UART variables */
    UART2_Params uart2Params;
    uint8_t input;

    /* AESCBC variables */
    AESCBC_Params params;
    CryptoKey cryptoKey;
    int_fast16_t encryptionResult;
#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
    int_fast16_t decryptionResult;
#endif

    AESCBC_Operation operation;

    /* Call driver initialization functions */
    AESCBC_init();
    AESCBC_Params_init(&params);
    params.returnBehavior = AESCBC_RETURN_BEHAVIOR_BLOCKING;

    /* Open UART for console output */
    UART2_Params_init(&uart2Params);
    uart2Params.readReturnMode = UART2_ReadReturnMode_FULL;

    uart2Handle = UART2_open(CONFIG_UART2_0, &uart2Params);

    if (!uart2Handle)
    {
        /* UART2_open() failed */
        while (1) {}
    }

    /* Open AESCBC Driver interface */
    aesHandle = AESCBC_open(CONFIG_AESCBC_0, &params);

    /* Check to make sure that we have a valid handle to AES Driver */
    if (aesHandle == NULL)
    {
        /* AESCBC_open() failed */
        printClearText(uart2Handle, (char *)msgAESOpenFails, NULL, 0);
        while (1) {}
    }

    /* Prompt startup message */
    UART2_write(uart2Handle, promptStartup, strlen(promptStartup), NULL);

    /* Loop forever */
    while (1)
    {
        /* Print prompt */
        UART2_write(uart2Handle, promptEnter, strlen(promptEnter), NULL);

        /* Reset testIndex for input */
        testIndex = 0;

        /* Read in from the console until carriage return is detected */
        while (1)
        {
            UART2_read(uart2Handle, &input, 1, NULL);

            /* If not carriage return, echo input and continue to read */
            if (input != ASCII_CR)
            {
                UART2_write(uart2Handle, &input, 1, NULL);
            }
            else
            {
                break;
            }

            /* Set the test index if it is '1' or '2' */
            if (input == ASCII_ONE || input == ASCII_TWO)
            {
                testIndex = input - ASCII_ONE;
            }

            /* If 'q' is entered, then example will exit. */
            if (input == ASCII_EXIT)
            {
                break;
            }
        }

        /* Break out of the outer loop. */
        if (input == ASCII_EXIT)
        {
            break;
        }

        CryptoKeyPlaintext_initKey(&cryptoKey,
                                   (uint8_t *)encryptionExampleVectors[testIndex].key,
                                   encryptionExampleVectors[testIndex].keyLength);
        AESCBC_OneStepOperation_init(&operation);

        operation.key         = &cryptoKey;
        operation.input       = (uint8_t *)encryptionExampleVectors[testIndex].plaintext;
        operation.inputLength = encryptionExampleVectors[testIndex].plaintextLength;
        operation.output      = ciphertext;
        operation.iv          = (uint8_t *)encryptionExampleVectors[testIndex].iv;

        /* Perform a single step encryption operation of the message */
        encryptionResult = AESCBC_oneStepEncrypt(aesHandle, &operation);

        if (encryptionResult != AESCBC_STATUS_SUCCESS)
        {
            /* Handle error, when oneStepEncrypt fails. */
            printClearText(uart2Handle, (char *)msgEncryptFails, NULL, 0);
            while (1) {}
        }

        /* Print ASCII text */
        printClearText(uart2Handle, (char *)promptClearText, (char *)operation.input, operation.inputLength);

        /* Print out the encrypted result */
        printMessage(uart2Handle, (char *)promptMessage, operation.output, operation.inputLength);

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX)
        /* Decrypt the encrypted result
         * For CC27XX devices, it would be valid to use CryptoKeyPlaintextHSM_initKey()
         * directly, if the targeted hardware accelerator is HSM module.
         */
        CryptoKeyPlaintext_initKey(&cryptoKey,
                                   (uint8_t *)encryptionExampleVectors[testIndex].key,
                                   encryptionExampleVectors[testIndex].keyLength);

        AESCBC_OneStepOperation_init(&operation);

        operation.key         = &cryptoKey;
        operation.input       = ciphertext;
        operation.inputLength = encryptionExampleVectors[testIndex].plaintextLength;
        operation.output      = plaintext;
        operation.iv          = (uint8_t *)encryptionExampleVectors[testIndex].iv;

        decryptionResult = AESCBC_oneStepDecrypt(aesHandle, &operation);
        if (decryptionResult != AESCBC_STATUS_SUCCESS)
        {
            /* Handle error, when oneStepDecrypt fails. */
            printClearText(uart2Handle, (char *)msgDecryptFails, NULL, 0);
            while (1) {}
        }

        /* Print out the encrypted result */
        printClearText(uart2Handle, (char *)promptMsgDecrypt, (char *)operation.output, operation.inputLength);

#else
        /* Print out decryption is not supported by CC23XX */
        printClearText(uart2Handle, (char *)msgNotSupported, NULL, 0);
#endif
        /* Print small banner to separate the next prompt */
        printBanner(uart2Handle, (char *)"*");
    }

    /* Close handle after AESCBC operations are completed. */
    printClearText(uart2Handle, (char *)msgExit, NULL, 0);
    AESCBC_close(aesHandle);
    return 0;
}
