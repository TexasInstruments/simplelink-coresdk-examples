/*
 * Copyright (c) 2016-2024, Texas Instruments Incorporated
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
 *  ======== temperature.c ========
 */
#include <stdint.h>
#include <unistd.h>

/* RTOS header files */
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <timers.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/I2C.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/*
 *  ======== HIGH_TEMP ========
 *  Send alert when this temperature (in Celsius) is exceeded
 */
#define HIGH_TEMP 30

/*
 *  ======== TMP Registers ========
 */
#define TMP_BP_REG  0x0000 /* Die Temp Result Register for BP TMP sensor */
#define TMP_BP_ADDR 0x48;

/* Temperature written by the temperature thread and read by console thread */
volatile float temperatureC;
volatile float temperatureF;

/*
 *  ======== clearAlert ========
 *  Clear the LED
 */
static void clearAlert(float temperature)
{
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_OFF);
}

/*
 *  ======== sendAlert ========
 *  Okay, just light a LED in this example, but with the SimpleLink SDK,
 *  you could send it out over the radio to something cool!
 */
static void sendAlert(float temperature)
{
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);
}

/*
 *  ======== postSem ========
 *  Function called when the timer (created in setupTimer) expires.
 */
static void postSem(TimerHandle_t xTimer)
{
    SemaphoreHandle_t xSemaphore = pvTimerGetTimerID(xTimer);

    xSemaphoreGive(xSemaphore);
}

/*
 *  ======== setupTimer ========
 *  Create a timer that will expire at the period specified by the
 *  time arguments. When the timer expires, the passed in semaphore
 *  will be posted by the postSem function.
 *
 *  A non-zero return indicates a failure.
 */
SemaphoreHandle_t setupTimer(TimerHandle_t *TimerHandle_t, unsigned int ms)
{
    SemaphoreHandle_t xSemaphore;
    int retc;

    xSemaphore = xSemaphoreCreateCounting(10, 0);
    if (xSemaphore == NULL)
    {
        return (NULL);
    }

    /* Create the timer that wakes up the thread that will pend on the sem. */
    *TimerHandle_t = xTimerCreate(NULL,               // pcTimerName
                                  pdMS_TO_TICKS(ms),  // xTimerPeriod
                                  pdTRUE,             // uxAutoReload
                                  (void *)xSemaphore, // pvTimerID
                                  postSem);           // pxCallbackFunction

    if (*TimerHandle_t == NULL)
    {
        vSemaphoreDelete(xSemaphore);
        return (NULL);
    }

    retc = xTimerStart(*TimerHandle_t, pdMS_TO_TICKS(ms));
    if (retc == pdFAIL)
    {
        vSemaphoreDelete(xSemaphore);
        return (NULL);
    }

    return (xSemaphore);
}

/*
 *  ======== temperatureThread ========
 *  This thread reads the temperature every second via I2C and sends an
 *  alert if it goes above HIGH_TEMP.
 */
void temperatureThread(void *arg0)
{
    uint8_t txBuffer[1];
    uint8_t rxBuffer[2];
    I2C_Handle i2c;
    I2C_Params i2cParams;
    I2C_Transaction i2cTransaction;
    TimerHandle_t timerHandle;
    SemaphoreHandle_t xTemperatureSemaphore;
    int retc;

    /* Configure the LED and if applicable, the TMP_EN pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);
#ifdef CONFIG_GPIO_TMP_EN
    GPIO_setConfig(CONFIG_GPIO_TMP_EN, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_HIGH);
    /* 1.5 ms reset time for the TMP sensor */
    sleep(1);
#endif

    /*
     *  Create/Open the I2C that talks to the TMP sensor
     */
    I2C_init();

    I2C_Params_init(&i2cParams);
    i2cParams.bitRate = I2C_400kHz;
    i2c               = I2C_open(CONFIG_I2C_TMP, &i2cParams);
    if (i2c == NULL)
    {
        while (1) {}
    }

    /* Common I2C transaction setup */
    i2cTransaction.writeBuf   = txBuffer;
    i2cTransaction.writeCount = 1;
    i2cTransaction.readBuf    = rxBuffer;
    i2cTransaction.readCount  = 2;

    /* Try BP TMP values */
    txBuffer[0]                  = TMP_BP_REG;
    i2cTransaction.targetAddress = TMP_BP_ADDR;
    if (!I2C_transfer(i2c, &i2cTransaction))
    {
        /* Could not resolve a sensor, error */
        while (1) {}
    }

    /*
     *  The temperature thread blocks on the semaphore, which the
     *  timer will post every second. The timer is created in the
     *  setupTimer function. It's returned so the thread could change the
     *  period or delete it if desired.
     */
    xTemperatureSemaphore = setupTimer(&timerHandle, 1000);
    if (xTemperatureSemaphore == NULL)
    {
        while (1) {}
    }

    while (1)
    {
        if (I2C_transfer(i2c, &i2cTransaction))
        {
            /*
             *  Extract degrees C from the received data; see sensor datasheet.
             *  Make sure we are updating the global temperature variables
             *  in a thread-safe manner.
             */
            taskENTER_CRITICAL();
            temperatureC = (rxBuffer[0] << 6) | (rxBuffer[1] >> 2);
            temperatureC *= 0.03125;
            temperatureF = temperatureC * 9 / 5 + 32;
            taskEXIT_CRITICAL();

            /*  Send an alert if the temperature is too high!! */
            if ((int)temperatureC >= HIGH_TEMP)
            {
                sendAlert(temperatureC);
            }
            else
            {
                clearAlert(temperatureC);
            }
        }

        /* Block until the timer posts the semaphore. */
        retc = xSemaphoreTake(xTemperatureSemaphore, portMAX_DELAY);
        if (retc == pdFALSE)
        {
            while (1) {}
        }
    }
}
