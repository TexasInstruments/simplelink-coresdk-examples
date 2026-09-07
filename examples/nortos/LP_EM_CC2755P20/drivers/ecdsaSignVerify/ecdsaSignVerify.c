/*
 * Copyright (c) 2024, Texas Instruments Incorporated
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
 *  ======== ecdsaSignVerify.c ========
 */

/* For usleep() */
#include <unistd.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/cryptoutils/cryptokey/CryptoKeyPlaintext.h>
#include <ti/drivers/ECDSA.h>
#include <ti/drivers/SHA2.h>

/* Driver configuration */
#include "ti_drivers_config.h"

#include "ecdsaTestVector.h"

#define TEST_VECTOR_COUNT       7
#define MSG_LENGTH              79
#define DIGEST_MAX_LENGTH       32
#define PLAINTEXT_MSG           ("Hello World. This message is to be sha2 hashed and ECDSA signed and verified. ")
#define SECP256_TEST_VECTOR_IDX 0
#define BRP256_TEST_VECTOR_IDX  4

/* Globals */
ECDSA_Handle ecdsaHandle;
ECDSA_Params ecdsaParams;

ECDSA_ReturnBehavior ecdsaReturnBehavior = ECDSA_RETURN_BEHAVIOR_CALLBACK;

SHA2_Handle sha2Handle;

ECDSA_OperationSign operationSign;
ECDSA_OperationVerify operationVerify;

SemaphoreP_Handle callbackSemHandle;

unsigned char plaintextMessageBuffer[MSG_LENGTH];
uint8_t digestBuffer[DIGEST_MAX_LENGTH];

/* Helper Functions defines */
static void toggleLEDInLoop(void)
{
    /* 1 second delay */
    uint32_t time = 1;

    while (1)
    {
        sleep(time);
        GPIO_toggle(CONFIG_GPIO_LED_0);
    }
}

static void ecdsaCallbackFxn(ECDSA_Handle handle,
                             int_fast16_t returnStatus,
                             ECDSA_Operation operation,
                             ECDSA_OperationType operationType)
{
    if (returnStatus != ECDSA_STATUS_SUCCESS)
    {
        toggleLEDInLoop();
    }

    SemaphoreP_post(callbackSemHandle);
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    int_fast16_t testResult;
    uint32_t j = 0;

    /* Call driver init functions */
    GPIO_init();
    ECDSA_init();
    SHA2_init();

    /* Configure the LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Turn on user LED */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    ECDSA_Params_init(&ecdsaParams);
    ecdsaParams.returnBehavior = ecdsaReturnBehavior;

    if (ecdsaReturnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK)
    {
        /* In callback mode, initialize the necessary metadata such as the callback semaphore and callback Fxn */
        SemaphoreP_Params semParams;
        SemaphoreP_Params_init(&semParams);
        semParams.mode    = SemaphoreP_Mode_BINARY;
        callbackSemHandle = SemaphoreP_create(0, &(semParams));

        ecdsaParams.callbackFxn = ecdsaCallbackFxn;
    }

    /* Open ECDSA */
    ecdsaHandle = ECDSA_open(0, &ecdsaParams);

    /* !!! WARNING !!!
     * For CC27XX devices, the ECDSA driver relies on a single HW accelerator for its sign and verify operations.
     * If the boot-up sequence of the HW accelerator or its corresponding SW architecture initialization fails,
     * the ECDSA driver will return a NULL handle.
     */
    if (ecdsaHandle == NULL)
    {
        /* Operation failed.
         * Toggle LED0 in an infinite loop
         */
        toggleLEDInLoop();
    }

    /* Open SHA2 */
    sha2Handle = SHA2_open(0, NULL);

    /* !!! WARNING !!!
     * For CC27XX devices, the SHA2 driver relies on a single HW accelerator for its hash operations.
     * If the boot-up sequence of the HW accelerator or its corresponding SW architecture initialization fails,
     * the SHA2 driver will return a NULL handle.
     */
    if (sha2Handle == NULL)
    {
        /* Operation failed.
         * Toggle LED0 in an infinite loop
         */
        toggleLEDInLoop();
    }

    /* Copy message to buffer */
    strncpy((char *)plaintextMessageBuffer, PLAINTEXT_MSG, MSG_LENGTH);

    /* For SECP-256 or BRP-256, we conduct the has in the code instead of relying on the test vector to demonstrate the
     * SHA2 and ECDSA drivers working together */
    testResult = SHA2_hashData(sha2Handle, plaintextMessageBuffer, MSG_LENGTH, digestBuffer);

    for (j = 0; j < TEST_VECTOR_COUNT; j++)
    {
        CryptoKey privateKey;

        /* Since CC27XX devices rely on a single HW engine for its ECDSA sign and verify operations,
         * Users must use the appropriate encoding (_HSM) when using the ECDSA driver for CC27XX device.
         */
        CryptoKeyPlaintextHSM_initKey(&privateKey, signVectors[j][0].privateKey, signVectors[j][0].curveLength);

        ECDSA_OperationSign_init(&operationSign);
        operationSign.myPrivateKey = &privateKey;

        /* For SECP-256 or BRP-256,
         * we conduct the hash in the code instead of relying on the pre-hashed test vector digest to demonstrate the
         * SHA2 and ECDSA drivers working together
         */
        if ((j == SECP256_TEST_VECTOR_IDX) || (j == BRP256_TEST_VECTOR_IDX))
        {
            operationSign.hash = digestBuffer;
        }
        else
        {
            operationSign.hash = signVectors[j][0].hash;
        }
        operationSign.r = signVectors[j][0].r;
        operationSign.s = signVectors[j][0].s;

        /* For CC27XX devices, unlike other devices and the ECDSA driver for them,
         * the ECDSA driver operations require that the user specifies the curve type
         * instead of the curve parameters to use in their sign and verify operations.
         *
         * Please refer to ti/drivers/ECDSA.h for a list of the supported Elliptical Curves for CC27XX devices.
         */
        operationSign.curveType = signVectors[j][0].curveType;

        /* Perform a sign operation on the provided data */
        testResult = ECDSA_sign(ecdsaHandle, &operationSign);

        if (testResult != ECDSA_STATUS_SUCCESS)
        {
            /* Operation failed.
             * Toggle LED0 in an infinite loop
             */
            toggleLEDInLoop();
        }

        if (ecdsaReturnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK)
        {
            SemaphoreP_pend(callbackSemHandle, SemaphoreP_WAIT_FOREVER);
        }

        CryptoKey publicKey;

        /* Since CC27XX devices rely on a single HW engine for its ECDSA sign and verify operations,
         * Users must use the appropriate encoding (_HSM) when using the ECDSA driver for CC27XX device.
         */
        CryptoKeyPlaintextHSM_initKey(&publicKey, signVectors[j][0].publicKey, 2 * signVectors[j][0].curveLength + 1);

        ECDSA_OperationVerify_init(&operationVerify);
        operationVerify.theirPublicKey = &publicKey;

        /* For SECP-256 or BRP-256,
         * we conduct the hash in the code instead of relying on the pre-hashed test vector digest to demonstrate the
         * SHA2 and ECDSA drivers working together
         */
        if ((j == SECP256_TEST_VECTOR_IDX) || (j == BRP256_TEST_VECTOR_IDX))
        {
            operationVerify.hash = digestBuffer;
        }
        else
        {
            operationVerify.hash = signVectors[j][0].hash;
        }
        operationVerify.r = signVectors[j][0].r;
        operationVerify.s = signVectors[j][0].s;

        /* For CC27XX devices, unlike other devices and the ECDSA driver for them,
         * the ECDSA driver operations require that the user specifies the curve type
         * instead of the curve parameters to use in their sign and verify operations.
         *
         * Please refer to ti/drivers/ECDSA.h for a list of the supported Elliptical Curves for CC27XX devices.
         */
        operationVerify.curveType = signVectors[j][0].curveType;

        /* Perform a verify operation on the provided data */
        testResult = ECDSA_verify(ecdsaHandle, &operationVerify);

        if (testResult != ECDSA_STATUS_SUCCESS)
        {
            /* Operation failed.
             * Toggle LED0 in an infinite loop
             */
            toggleLEDInLoop();
        }

        if (ecdsaReturnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK)
        {
            SemaphoreP_pend(callbackSemHandle, SemaphoreP_WAIT_FOREVER);
        }
    }

    /* If we are in callback mode, cleanup metadata */
    if (ecdsaReturnBehavior == ECDSA_RETURN_BEHAVIOR_CALLBACK)
    {
        SemaphoreP_delete(callbackSemHandle);
    }

    ECDSA_close(ecdsaHandle);

    SHA2_close(sha2Handle);

    return NULL;
}
