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
 *  ======== psaAeadEncrypt.c ========
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* POSIX Header files */
#include <pthread.h>

/* PSA Crypto header file */
#include <third_party/psa_crypto/include/psa/crypto.h>

/* Driver Header files */
#include <ti/display/Display.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/cryptoutils/hsm/HSMLPF3.h>

#include <ti/devices/DeviceFamily.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#define THREAD_STACK_SIZE 1536

/* Array of valid PSA key lifetimes */
static const psa_key_lifetime_t lifetimes[] = {
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_LOCATION_LOCAL_STORAGE),
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_LOCAL_STORAGE),

#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX))
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_VOLATILE, PSA_KEY_LOCATION_HSM_ASSET_STORE),
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_DEFAULT, PSA_KEY_LOCATION_HSM_ASSET_STORE),
    PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_HSM_ASSET_STORE,
                                                   PSA_KEY_LOCATION_HSM_ASSET_STORE),
#endif
};

static const size_t lifetimeCnt = sizeof(lifetimes) / sizeof(psa_key_lifetime_t);

#define MAX_PLAINTEXT_LENGTH    32
#define MAX_MAC_LENGTH          16
#define KEY_DERIVATION_LOOP_CNT 5
#define EXPECTED_PASS_CNT       (lifetimeCnt + (lifetimeCnt * KEY_DERIVATION_LOOP_CNT))

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX)
    /* For CC35XX device, HSM module cannot access Flash directly.
     * Need to place plaintext in Data RAM, where HSM's DMA can access.
     */
    #define _CONST
#else
    #define _CONST const
#endif

typedef struct
{
    uint8_t key[32];
    uint8_t keyLength;
    uint8_t aad[64];
    uint8_t aadLength;
    uint8_t plaintext[MAX_PLAINTEXT_LENGTH];
    uint8_t plaintextLength;
    uint8_t nonce[16];
    uint8_t nonceLength;
    uint8_t expectedMac[MAX_MAC_LENGTH];
    uint8_t macLength;
    uint8_t expectedCiphertext[MAX_PLAINTEXT_LENGTH];
    psa_algorithm_t alg;
} AEADTestVector;

static _CONST AEADTestVector testVector = {
    /* Test vector 180 from NIST CAVP DVPT 128 */
    .key             = {0xf9, 0xfd, 0xca, 0x4a, 0xc6, 0x4f, 0xe7, 0xf0, 0x14, 0xde, 0x0f, 0x43, 0x03, 0x9c, 0x75, 0x71},
    .keyLength       = 16,
    .aad             = {0x37, 0x96, 0xcf, 0x51, 0xb8, 0x72, 0x66, 0x52, 0xa4, 0x20, 0x47, 0x33, 0xb8, 0xfb, 0xb0, 0x47,
                        0xcf, 0x00, 0xfb, 0x91, 0xa9, 0x83, 0x7e, 0x22, 0xec, 0x22, 0xb1, 0xa2, 0x68, 0xf8, 0x8e, 0x2c},
    .aadLength       = 32,
    .plaintext       = {0xa2, 0x65, 0x48, 0x0c, 0xa8, 0x8d, 0x5f, 0x53, 0x6d, 0xb0, 0xdc, 0x6a,
                        0xbc, 0x40, 0xfa, 0xf0, 0xd0, 0x5b, 0xe7, 0xa9, 0x66, 0x97, 0x77, 0x68},
    .plaintextLength = 24,
    .nonce           = {0x5a, 0x8a, 0xa4, 0x85, 0xc3, 0x16, 0xe9},
    .nonceLength     = 7,
    .expectedMac     = {0x38, 0xf1, 0x25, 0xfa},
    .macLength       = 4,
    .expectedCiphertext = {0x6b, 0xe3, 0x18, 0x60, 0xca, 0x27, 0x1e, 0xf4, 0x48, 0xde, 0x8f, 0x8d,
                           0x8b, 0x39, 0x34, 0x6d, 0xaf, 0x4b, 0x81, 0xd7, 0xe9, 0x2d, 0x65, 0xb3},
    .alg                = PSA_ALG_AEAD_WITH_SHORTENED_TAG(PSA_ALG_CCM, 4),
};

static const uint8_t context[] = "ThisIsAContextOfSufficientLength";
static const uint8_t label[]   = "ThisIsALabelOfSufficientLengthToSatisfyTheHSMRequirement";
#define TEST_MSG_SIZE   16 /* bytes */
#define MSG_BUFFER_SIZE 128

static char msgBuf[MSG_BUFFER_SIZE];
static Display_Handle display;

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
    for (i = 0; i < len; i++)
    {
        sprintf(dest + (i * 2), "%02X", *(array + i));
    }

    Display_printf(display, 0U, 0U, msgBuf);
}

/*
 *  ======== printKeyLifetime ========
 */
static void printKeyLifetime(psa_key_lifetime_t lifetime)
{
    char *dest                        = &msgBuf[0];
    psa_key_persistence_t persistence = PSA_KEY_LIFETIME_GET_PERSISTENCE(lifetime);
    psa_key_location_t location       = PSA_KEY_LIFETIME_GET_LOCATION(lifetime);

    strcpy(dest, "Key persistence/location: ");
    dest = &msgBuf[0] + strlen(msgBuf);

    if (persistence == PSA_KEY_PERSISTENCE_VOLATILE)
    {
        strcpy(dest, "[Volatile / ");
    }
    else if (persistence == PSA_KEY_PERSISTENCE_DEFAULT)
    {
        strcpy(dest, "[Default / ");
    }
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX))
    else if (persistence == PSA_KEY_PERSISTENCE_HSM_ASSET_STORE)
    {
        strcpy(dest, "[HSM Asset Store / ");
    }
#endif

    dest = &msgBuf[0] + strlen(msgBuf);

    if (location == PSA_KEY_LOCATION_LOCAL_STORAGE)
    {
        strcpy(dest, "Local Storage]");
    }
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX) || (DeviceFamily_PARENT == DeviceFamily_PARENT_CC35XX))
    else if (location == PSA_KEY_LOCATION_HSM_ASSET_STORE)
    {
        strcpy(dest, "HSM Asset Store]");
    }
#endif

    Display_printf(display, 0U, 0U, msgBuf);
}

static size_t setupKeyDerivation(psa_key_derivation_operation_t *derivation, psa_algorithm_t alg, psa_key_id_t keyID)
{
    psa_status_t status;
    size_t capacity;

    /* Set up the key derivation operation */
    status = psa_key_derivation_setup(derivation, alg);
    if (status != PSA_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: psa_key_derivation_setup() failed. Status = %d\n", status);
    }

    /* Provide the input key for derivation */
    status = psa_key_derivation_input_key(derivation, PSA_KEY_DERIVATION_INPUT_SECRET, keyID);
    if (status != PSA_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: psa_key_derivation_input_key() failed. Status = %d\n", status);
    }

    /* Set input label */
    status = psa_key_derivation_input_bytes(derivation, PSA_KEY_DERIVATION_INPUT_LABEL, label, sizeof(label));
    if (status != PSA_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: psa_key_derivation_input_bytes() failed. Status = %d\n", status);
    }

    /* Set input context. When deriving from the HUK or from the TKDK, the HSM
     * automatically appends a context. Therefore, we should not add the context
     * provided by the application.
     */
    if ((keyID != PSA_KEY_ID_HSM_HUK) && (keyID != PSA_KEY_ID_HSM_TKDK))
    {
        status = psa_key_derivation_input_bytes(derivation, PSA_KEY_DERIVATION_INPUT_CONTEXT, context, sizeof(context));
    }

    /* Verify the initial capacity */
    status = psa_key_derivation_get_capacity(derivation, &capacity);
    if (status != PSA_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: psa_key_derivation_get_capacity() failed. Status = %d\n", status);
    }
    return capacity;
}
/*
 *  ======== encryptThread ========
 */
static void *encryptThread(void *arg0)
{
    psa_key_attributes_t attributes;
    psa_key_id_t keyID;
    psa_key_lifetime_t lifetime;
    psa_status_t status;
    size_t index;
    size_t outputLength;
    size_t capacity;
    psa_key_derivation_operation_t derivation = PSA_KEY_DERIVATION_OPERATION_INIT;
    uint_fast8_t i, j;
    int_fast8_t passCnt = 0U;
    uint8_t ciphertext[MAX_PLAINTEXT_LENGTH + MAX_MAC_LENGTH];

    /* Print the encryption inputs */
    printByteArray(display, "Nonce: 0x", testVector.nonce, testVector.nonceLength);
    printByteArray(display, "AAD: 0x", testVector.aad, testVector.aadLength);
    printByteArray(display, "Plaintext: 0x", testVector.plaintext, testVector.plaintextLength);
    printByteArray(display, "Key: 0x", testVector.key, testVector.keyLength);
    Display_printf(display, 0U, 0U, "");

    /* Key Generation with different key lifetimes */
    for (i = 0U; i < lifetimeCnt; i++)
    {
        lifetime = lifetimes[i];

        /* Init key attributes */
        attributes = PSA_KEY_ATTRIBUTES_INIT;
        psa_set_key_algorithm(&attributes, testVector.alg);
        psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
        psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(testVector.keyLength));
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_ENCRYPT);
        psa_set_key_lifetime(&attributes, lifetime);

        if (PSA_KEY_LIFETIME_GET_PERSISTENCE(lifetime) != PSA_KEY_PERSISTENCE_VOLATILE)
        {
            /* Set the key ID for non-volatile keys, Range:[PSA_KEY_ID_USER_MIN - PSA_KEY_ID_USER_MAX] */
            keyID = PSA_KEY_ID_USER_MIN + i;
            psa_set_key_id(&attributes, keyID);

            /* Attempt to delete the key to ensure psa_import_key() works
             * everytime. Ignore return value as it may fail if the key does not
             * exist.
             */
            (void)psa_destroy_key(keyID);
        }

        printKeyLifetime(lifetime);

        /* Import the key */
        status = psa_import_key(&attributes, testVector.key, testVector.keyLength, &keyID);

        if (status != PSA_SUCCESS)
        {
            Display_printf(display, 0U, 0U, "Error: psa_import_key() failed. Status = %d\n", status);

            if (status == PSA_ERROR_ALREADY_EXISTS)
            {
                /* Attempt to delete the existing key. So, next run could be successful. */
                status = psa_destroy_key(keyID);
                Display_printf(display, 0U, 0U, "Destroy Key: psa_destroy_key() called. Status = %d\n", status);
            }

            /* Skip to next key lifetime if key import fails */
            continue;
        }

        /* Retrieve the updated key attributes */
        status = psa_get_key_attributes(keyID, &attributes);

        if (status != PSA_SUCCESS)
        {
            Display_printf(display, 0U, 0U, "Error: psa_get_key_attributes() failed. Status = %d\n", status);
        }

        Display_printf(display, 0U, 0U, "Key ID: 0x%x", psa_get_key_id(&attributes));

        /* Zero the ciphertext output buffer */
        (void)memset(&ciphertext[0], 0, sizeof(ciphertext));

        Display_printf(display, 0U, 0U, "Calling psa_aead_encrypt()");

        /* Encrypt */
        status = psa_aead_encrypt(keyID,
                                  testVector.alg,
                                  testVector.nonce,
                                  testVector.nonceLength,
                                  testVector.aad,
                                  testVector.aadLength,
                                  testVector.plaintext,
                                  testVector.plaintextLength,
                                  ciphertext,
                                  sizeof(ciphertext),
                                  &outputLength);

        if (status == PSA_SUCCESS)
        {
            printByteArray(display, "Ciphertext: 0x", ciphertext, outputLength);

            /* Verify ciphertext output matches expected */
            for (index = 0; index < testVector.plaintextLength; index++)
            {
                if (ciphertext[index] != testVector.expectedCiphertext[index])
                {
                    Display_printf(display,
                                   0U,
                                   0U,
                                   "Error: ciphertext[%d] = 0x%02x does not match expected 0x%02x\n",
                                   index,
                                   ciphertext[index],
                                   testVector.expectedCiphertext[index]);

                    status = PSA_ERROR_GENERIC_ERROR;
                    break;
                }
            }

            if (status == PSA_SUCCESS)
            {
                /* Verify MAC is correct */
                for (index = 0; index < testVector.macLength; index++)
                {
                    /* The output MAC is appended to the ciphertext output */
                    if (ciphertext[testVector.plaintextLength + index] != testVector.expectedMac[index])
                    {
                        Display_printf(display,
                                       0U,
                                       0U,
                                       "Error: MAC[%d] = 0x%02x does not match expected 0x%02x\n",
                                       index,
                                       ciphertext[testVector.plaintextLength + index],
                                       testVector.expectedMac[index]);

                        status = PSA_ERROR_INVALID_SIGNATURE;
                        break;
                    }
                }
            }

            if (status == PSA_SUCCESS)
            {
                passCnt++;
                Display_printf(display, 0U, 0U, "PASSED!\n");
            }
        }
        else
        {
            Display_printf(display, 0U, 0U, "Error: psa_aead_encrypt() failed. Status = %d\n", status);
        }

        /* Destroy the key that was imported */
        status = psa_destroy_key(keyID);

        if (status != PSA_SUCCESS)
        {
            passCnt--;
            Display_printf(display, 0U, 0U, "Error: psa_destroy_key() failed. Status = %d\n", status);
        }
    }

    Display_printf(display, 0U, 0U, "\nStarting key derivation...\n");

    /* Key Derivation */
    for (i = 0U; i < lifetimeCnt; i++)
    {
        psa_key_id_t newkeyID;

        lifetime = lifetimes[i];

        /* configure key attributes for the key you want to use to derive a new key */
        attributes = PSA_KEY_ATTRIBUTES_INIT;
        psa_set_key_algorithm(&attributes, PSA_ALG_SP800_108_COUNTER_CMAC);
        psa_set_key_bits(&attributes, PSA_BYTES_TO_BITS(testVector.keyLength));
        psa_set_key_type(&attributes, PSA_KEY_TYPE_AES);
        psa_set_key_usage_flags(&attributes, PSA_KEY_USAGE_DERIVE);
        psa_set_key_lifetime(&attributes, lifetime);

        if (PSA_KEY_LIFETIME_GET_PERSISTENCE(lifetime) != PSA_KEY_PERSISTENCE_VOLATILE)
        {
            /* Set the key ID for non-volatile keys, Range:[PSA_KEY_ID_USER_MIN - PSA_KEY_ID_USER_MAX] */
            keyID = 5U + i;
            psa_set_key_id(&attributes, keyID);

            /* Attempt to delete the key to ensure psa_import_key() works
             * everytime. Ignore return value as it may fail if the key does not
             * exist.
             */
            (void)psa_destroy_key(keyID);
        }

        printKeyLifetime(lifetime);

        /* Import the key */
        status = psa_import_key(&attributes, testVector.key, testVector.keyLength, &keyID);

        if (status != PSA_SUCCESS)
        {
            Display_printf(display, 0U, 0U, "Error: psa_import_key() failed. Status = %d\n", status);

            if (status == PSA_ERROR_ALREADY_EXISTS)
            {
                /* Attempt to delete the existing key. So, next run could be successful. */
                status = psa_destroy_key(keyID);
                Display_printf(display, 0U, 0U, "Destroy Key: psa_destroy_key() called. Status = %d\n", status);
            }

            /* Skip to next key lifetime if key import fails */
            continue;
        }

        capacity = setupKeyDerivation(&derivation, PSA_ALG_SP800_108_COUNTER_CMAC, keyID);
        /* Set capacity to KEY_DERIVATION_LOOP_CNT */
        capacity = KEY_DERIVATION_LOOP_CNT * TEST_MSG_SIZE;
        status   = psa_key_derivation_set_capacity(&derivation, capacity);

        /* Read capacity to verify it was set */
        status = psa_key_derivation_get_capacity(&derivation, &capacity);
        if (status == PSA_SUCCESS)
        {
            Display_printf(display, 0U, 0U, "Capacity is set to = %d\n", capacity);
        }

        /* Setup new key attributes */
        psa_key_attributes_t newAttributes = PSA_KEY_ATTRIBUTES_INIT;
        psa_set_key_algorithm(&newAttributes, PSA_ALG_ECB_NO_PADDING);
        psa_set_key_bits(&newAttributes, PSA_BYTES_TO_BITS(TEST_MSG_SIZE));
        psa_set_key_type(&newAttributes, PSA_KEY_TYPE_AES);
        psa_set_key_usage_flags(&newAttributes, (PSA_KEY_USAGE_ENCRYPT));
        /* Derived keys can only be produced with Asset Store location and persistence */
        psa_set_key_lifetime(&newAttributes,
                             PSA_KEY_LIFETIME_FROM_PERSISTENCE_AND_LOCATION(PSA_KEY_PERSISTENCE_HSM_ASSET_STORE,
                                                                            PSA_KEY_LOCATION_HSM_ASSET_STORE));
        for (j = 0U; j < KEY_DERIVATION_LOOP_CNT; j++)
        {
            newkeyID = 10U + (i * 10U) + j;
            psa_set_key_id(&newAttributes, newkeyID);

            /* Perform the key derivation to output HSM asset */
            status = psa_key_derivation_output_key(&newAttributes, &derivation, &newkeyID);
            if (status == PSA_SUCCESS)
            {
                Display_printf(display, 0U, 0U, "A key has been newly derived, KeyID = %d", newkeyID);
            }
            /* Retrieve the updated key attributes */
            status = psa_get_key_attributes(newkeyID, &newAttributes);

            if (status != PSA_SUCCESS)
            {
                Display_printf(display, 0U, 0U, "Error: psa_get_key_attributes() failed. Status = %d\n", status);
                continue;
            }

            Display_printf(display, 0U, 0U, "Read KeyID: %d", psa_get_key_id(&newAttributes));

            /* Verify the capacity was reduced by the correct amount */
            status = psa_key_derivation_get_capacity(&derivation, &capacity);
            if (status == PSA_SUCCESS)
            {
                Display_printf(display, 0U, 0U, "Remaining Capacity is = %d", capacity);
            }

            /* Zero the ciphertext output buffer */
            (void)memset(&ciphertext[0], 0, sizeof(ciphertext));

            Display_printf(display, 0U, 0U, "Calling psa_cipher_encrypt()");

            /* Encrypt */
            status = psa_cipher_encrypt(newkeyID,
                                        PSA_ALG_ECB_NO_PADDING,
                                        testVector.plaintext,
                                        TEST_MSG_SIZE,
                                        ciphertext,
                                        sizeof(ciphertext),
                                        &outputLength);

            if (status == PSA_SUCCESS)
            {
                passCnt++;
                printByteArray(display, "Ciphertext: 0x", ciphertext, outputLength);
                Display_printf(display, 0U, 0U, "ciphertext should be different every time \n");
            }
            else
            {
                Display_printf(display, 0U, 0U, "Error: psa_aead_encrypt() failed. Status = %d\n", status);
            }

            /* Destroy the derived key as there are only a limited number of HSM Assets available */
            status = psa_destroy_key(newkeyID);
        }

        /* Destroy the original key */
        status = psa_destroy_key(keyID);

        if (status != PSA_SUCCESS)
        {
            passCnt--;
            Display_printf(display, 0U, 0U, "Error: psa_destroy_key() failed. Status = %d\n", status);
        }
    }

    if (passCnt == EXPECTED_PASS_CNT)
    {
        Display_printf(display, 0U, 0U, "DONE!\n");
        GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);
    }

    return (NULL);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    int retc;
    int_fast16_t ret;
    psa_status_t status;
    pthread_attr_t attrs;
    pthread_t thread0;
    struct sched_param priParam;

    /* Initialize display driver */
    Display_init();

    /* Open the display for output */
    display = Display_open(Display_Type_UART, NULL);
    if (display == NULL)
    {
        /* Failed to open display driver */
        while (1) {}
    }

    Display_printf(display, 0U, 0U, "\nStarting the PSA Crypto AEAD Encrypt example.\n");

    /* Initialize PSA Crypto */
    status = psa_crypto_init();
    if (status != PSA_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: psa_crypto_init() failed. Status = %d\n", status);
        while (1) {}
    }

    Display_printf(display, 0U, 0U, "Provisioning Hardware Unique Key (HUK)...\n");

    /* Provision the HW Unique Key needed to store key blobs */
    ret = HSMLPF3_provisionHUK();
    if (ret != HSMLPF3_STATUS_SUCCESS)
    {
        Display_printf(display, 0U, 0U, "Error: HSMLPF3_provisionHUK() failed. Status = %d\n", ret);
        while (1) {}
    }

    /* Turn on LED0 to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Set priority and stack size attributes */
    priParam.sched_priority = 1;

    retc = pthread_attr_init(&attrs);
    retc |= pthread_attr_setschedparam(&attrs, &priParam);
    retc |= pthread_attr_setdetachstate(&attrs, PTHREAD_CREATE_DETACHED);
    retc |= pthread_attr_setstacksize(&attrs, THREAD_STACK_SIZE);
    if (retc != 0)
    {
        /* Failed to set thread attributes */
        while (1) {}
    }

    /* Create encrypt thread */
    retc = pthread_create(&thread0, &attrs, encryptThread, NULL);
    if (retc != 0)
    {
        /* pthread_create() failed */
        while (1) {}
    }

    return (NULL);
}
