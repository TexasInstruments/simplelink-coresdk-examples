/*
 * Copyright (c) 2022-2025, Texas Instruments Incorporated
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
 *  ======== attestation.c ========
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* PSA Crypto header file */
#include <third_party/mbedtls/include/psa/crypto.h>
#include <psa/initial_attestation.h>

#include <ti/drivers/crypto/CryptoTFM_ns.h> /* tfm_get_version() */
#include <ti/display/Display.h>
#include <ti/drivers/GPIO.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define SECP256R1_PUBLIC_KEY_LENGTH (1 + 2 * PSA_BITS_TO_BYTES(256)) /* format: 0x04 | x | y */
#define IAK_PUBLIC_KEY_LENGTH       SECP256R1_PUBLIC_KEY_LENGTH
#define MAX_CHALLENGE_SIZE          64
#define MSG_BUFFER_SIZE             (PSA_INITIAL_ATTEST_MAX_TOKEN_SIZE + 50)

static uint8_t publicKey[IAK_PUBLIC_KEY_LENGTH];

static uint8_t nonce[MAX_CHALLENGE_SIZE];

static uint8_t tokenBuf[PSA_INITIAL_ATTEST_MAX_TOKEN_SIZE];

static char msgBuf[MSG_BUFFER_SIZE];

/*
 *  ======== printByteArray ========
 */
static void printByteArray(Display_Handle display, const char *desc, const uint8_t *array, size_t len)
{
    char *dest = &msgBuf[0];
    size_t i;

    strcpy(dest, desc);
    dest += strlen(desc);

    /* Format the message as printable hex */
    for (i = 0U; i < len; i++)
    {
        sprintf(dest + (i * 2), "%02X", *(array + i));
    }

    Display_printf(display, 0U, 0U, msgBuf);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    Display_Handle displayHandle;
    psa_status_t status;
    size_t tokenLength;
    size_t outputLength;
    uint8_t version[20] = {0};
    size_t versionSize  = sizeof(version);

    Board_init();

    Display_init();
    displayHandle = Display_open(Display_Type_UART, NULL);
    if (displayHandle == NULL)
    {
        /* Display_open() failed */
        while (1) {}
    }

    Display_printf(displayHandle, 0, 0, "PSA Attestation with Secure/Non-Secure isolation example:");

    /* Get the TF-M version */
    tfm_get_version(&version[0], &versionSize);
    Display_printf(displayHandle, 0, 0, "\nTF-M Version: %s", version);

    status = psa_crypto_init();
    if (status != PSA_SUCCESS)
    {
        Display_printf(displayHandle, 0, 0, "psa_crypto_init() failed, status = %d", status);
        return (NULL);
    }

    /* Turn on LED0 to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    Display_printf(displayHandle, 0, 0, "\nExporting attestation public key...");

    /* Export public IAK */
    status = psa_export_public_key(PSA_KEY_ID_IAK, publicKey, sizeof(publicKey), &outputLength);
    if (status != PSA_SUCCESS)
    {
        Display_printf(displayHandle, 0, 0, "psa_export_public_key() failed, status = %d", status);
        return (NULL);
    }

    /* Print public IAK as hex string */
    printByteArray(displayHandle, "Public IAK: ", publicKey, outputLength);

    Display_printf(displayHandle, 0, 0, "\nGenerating nonce for challenge...");

    /* Generate nonce for challenge */
    status = psa_generate_random(nonce, sizeof(nonce));
    if (status != PSA_SUCCESS)
    {
        Display_printf(displayHandle, 0, 0, "psa_generate_random() failed, status = %d", status);
        return (NULL);
    }

    /* Print nonce as hex string */
    printByteArray(displayHandle, "Nonce: ", nonce, sizeof(nonce));

    Display_printf(displayHandle, 0, 0, "\nGetting token size...");

    /* Get attestation token size */
    status = psa_initial_attest_get_token_size(sizeof(nonce), &tokenLength);
    if (status != PSA_SUCCESS)
    {
        Display_printf(displayHandle, 0, 0, "psa_initial_attest_get_token_size() failed, status = %d", status);
        return (NULL);
    }
    Display_printf(displayHandle, 0, 0, "Token size: %d bytes", tokenLength);

    if (tokenLength > sizeof(tokenBuf))
    {
        Display_printf(displayHandle, 0, 0, "Error: Token size exceeds buffer size!\n");
        return (NULL);
    }

    Display_printf(displayHandle, 0, 0, "\nRetrieving attestation token...");

    /* Get attestation token */
    status = psa_initial_attest_get_token(&nonce[0], sizeof(nonce), &tokenBuf[0], sizeof(tokenBuf), &tokenLength);
    if (status != PSA_SUCCESS)
    {
        Display_printf(displayHandle, 0, 0, "psa_initial_attest_get_token() failed, status = %d", status);
        return (NULL);
    }

    /* Print token as hex string */
    printByteArray(displayHandle, "Token: ", tokenBuf, tokenLength);

    /* Turn on LED1 to indicate token was successfully generated */
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);

    return (NULL);
}
