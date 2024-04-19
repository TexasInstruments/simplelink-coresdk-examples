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
 *  ======== systimtimestamp.c ========
 */
#include <stdint.h>
#include <string.h>
#include <stddef.h>

/* Driverlib header files */
#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(driverlib/interrupt.h)
#include DeviceFamily_constructPath(inc/hw_types.h)
#include DeviceFamily_constructPath(inc/hw_memmap.h)
#include DeviceFamily_constructPath(inc/hw_ioc.h)
#include DeviceFamily_constructPath(inc/hw_gpio.h)
#include DeviceFamily_constructPath(inc/hw_evtsvt.h)
#include DeviceFamily_constructPath(inc/hw_systim.h)

/* Correct field-name deviations between CC23X0 and CC27XX */
#if DeviceFamily_PARENT == DeviceFamily_PARENT_CC23X0
    #define GPIO_EVTCFG_EVTEN_ENABLE GPIO_EVTCFG_EVTEN_EN
    #define IOC_IOC0_INPEN_ENABLE    IOC_IOC0_INPEN_EN
#endif
#if DeviceFamily_PARENT == DeviceFamily_PARENT_CC27XX
    #define EVTSVT_SYSTIMC1SEL_PUBID_GPIO_EVT EVTSVT_SYSTIMC1SEL_PUBID_GPIO_EVT0
#endif

/* Driver Header files */
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/UART2.h>
#include <ti/drivers/Power.h>

/* Driver configuration */
#include "ti_drivers_config.h"

/* Message strings transferred to user terminal with UART TX */
const char rxStartMessage[]     = "\r\n\nStart listening for UART RX start bit! Send \"RX_ID_0\" to begin.";
const char txCaptureMessage[]   = "\r\nCaptured UART TX start bit at timestamp: 0x";
const char rxCaptureMessage[]   = "\r\nCaptured UART RX start bit at timestamp: 0x";
const char gpioStartMessage[]   = "\r\n\nStart listening for GPIO event!";
const char gpioCaptureMessage[] = "\r\nCaptured GPIO input event at timestamp: 0x";

/* User message expected to be received from user terminal over UART */
const char rxExpectedMessage[] = "RX_ID_0";

/* Variable holding the captured SYSTIM timer value */
uint32_t capturedTimeUsec;

/* Variable holding number of SYSTIM capture events triggered by UART RX/TX events.
 * Only used for debug purpose.
 */
volatile uint32_t uartEventCount;

/* Variable holding number of SYSTIM capture events triggered by GPIO events.
 * Only used for debug purpose.
 */
volatile uint32_t gpioEventCount;

/* Interrupt for handling SYSTIM channel 1 capture */
HwiP_Struct systimHwi;

/* Semaphore to pend on BUTTON_0 presses */
SemaphoreP_Struct semStruct;
SemaphoreP_Handle semHandle;

/*
 *  ======= SYSTIM callback ISR ======
 */
static void sysTimerCallbackISR(uintptr_t arg)
{
    /* Turn GREEN LED on for observation aid */
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_ON);

    /* Get status of events triggering SYSTIM channel 1 capture */
    uint32_t eventStatus = HWREG(GPIO_BASE + GPIO_O_RIS);

    /* Get captured time in number of usec.
     * Note that channel 1 only supports 1 usec resolution.
     * Note that this register read forces SYSTIM:RIS.EV1 to be cleared.
     */
    capturedTimeUsec = HWREG(SYSTIM_BASE + SYSTIM_O_CH1CC);

    /* Check if SYSTIM capture event was triggered by UART RX/TX event */
    if ((eventStatus & (GPIO_RIS_DIO0_SET << CONFIG_GPIO_UART2_0_RX)) ||
        (eventStatus & (GPIO_RIS_DIO0_SET << CONFIG_GPIO_UART2_0_TX)))
    {
        uartEventCount++;

        /* Clear event status for UART RX/TX pins */
        GPIO_clearInt(CONFIG_GPIO_UART2_0_RX);
        GPIO_clearInt(CONFIG_GPIO_UART2_0_TX);
    }

    /* Check if SYSTIM capture event was triggered by GPIO event */
    if (eventStatus & (GPIO_RIS_DIO0_SET << CONFIG_GPIO_BUTTON_0))
    {
        SemaphoreP_post(semHandle);
        gpioEventCount++;

        /* Release disallow STANDBY constraint */
        Power_releaseConstraint(PowerLPF3_DISALLOW_STANDBY);

        /* Clear event status for GPIO event */
        GPIO_clearInt(CONFIG_GPIO_BUTTON_0);
    }

    /* Turn GREEN LED off for observation aid */
    GPIO_write(CONFIG_GPIO_LED_1, CONFIG_GPIO_LED_OFF);
}

/*
 *  ======= Converts integer (32-bit) to ASCII-string (hexadecimal format) ======
 */
static void integerToAscii(uint32_t integerVal, char *asciiArray)
{
    uint32_t nibbleVal;

    for (uint32_t i = 0; i < 8; i++)
    {
        nibbleVal = ((integerVal >> (i * 4)) & 0x0000000F);
        if (nibbleVal <= 9)
        {
            /* 0x0 - 0x9 */
            asciiArray[7 - i] = '0' + nibbleVal;
        }
        else
        {
            /* 0xa - 0xf */
            asciiArray[7 - i] = ('a' - 10) + nibbleVal;
        }
    }
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{
    bool expectedUserMsgRecieved = false;

    /* Array holding timestamp as ASCII characters in hexadecimal format */
    char timestampAsciiArray[8];

    UART2_Handle uart;
    UART2_Params uartParams;
    size_t bytesWritten        = 0;
    uint32_t status            = UART2_STATUS_SUCCESS;
    uint32_t rxTimestampUsec   = 0;
    uint32_t txTimestampUsec   = 0;
    uint32_t gpioTimestampUsec = 0;

    /* Call driver init functions */
    GPIO_init();

    /* Configure the RED LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_0, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Configure the GREEN LED pin */
    GPIO_setConfig(CONFIG_GPIO_LED_1, GPIO_CFG_OUT_STD | GPIO_CFG_OUT_LOW);

    /* Create a UART where the default read and write mode is BLOCKING */
    UART2_Params_init(&uartParams);
    uartParams.baudRate = 115200;

    uart = UART2_open(CONFIG_UART2_0, &uartParams);

    if (uart == NULL)
    {
        /* UART2_open() failed */
        while (1) {}
    }

    /*
     * ======== Demonstrate capture of UART RX event =========
     */

    /* Configure GPIO and IOC to generate a GPIO event when a falling edge is detected on the UART RX pin */
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) = GPIO_EVTCFG_DIOSEL_M & (CONFIG_GPIO_UART2_0_RX << GPIO_EVTCFG_DIOSEL_S);
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) |= GPIO_EVTCFG_EVTEN_ENABLE;
    HWREG(IOC_BASE + (IOC_O_IOC0 + (CONFIG_GPIO_UART2_0_RX * 4))) |= IOC_IOC0_EDGEDET_EDGE_NEG;

    /* Configure GPIO event to be input to SYSTIM channel 1 */
    HWREG(EVTSVT_BASE + EVTSVT_O_SYSTIMC1SEL) = EVTSVT_SYSTIMC1SEL_PUBID_GPIO_EVT;

    /* Configure SYSTIM to capture event on channel 1.
     *   - Capture re-arm is disabled
     *   - Capture on rising edge of event signal from GPIO
     *   - Channel in capture mode.
     *     SYSTIM:CH1CFG.MODE bit is cleared by HW at capture event.
     */
    HWREG(SYSTIM_BASE + SYSTIM_O_CH1CFG) = (SYSTIM_CH1CFG_REARM_DIS | SYSTIM_CH1CFG_INP_RISE | SYSTIM_CH1CFG_MODE_CAPT);

    /* Enable CPU IRQ0 interrupt on SYSTIM channel 1 capture event.
     *   - Note that the channel event is cleared when reading the channel
     *     capture value which is done in the sysTimerCallbackISR() function.
     */
    HwiP_construct(&systimHwi, INT_CPUIRQ0, sysTimerCallbackISR, NULL);
    HWREG(EVTSVT_BASE + EVTSVT_O_CPUIRQ0SEL) = EVTSVT_CPUIRQ0SEL_PUBID_SYSTIM1;
    HwiP_enableInterrupt(INT_CPUIRQ0);

    /* Turn on user RED LED to indicate successful initialization */
    GPIO_write(CONFIG_GPIO_LED_0, CONFIG_GPIO_LED_ON);

    /* Write example start message over UART */
    UART2_write(uart, rxStartMessage, sizeof(rxStartMessage) - 1, &bytesWritten);

    /* Loop waiting for expected message from user received over UART */
    uint32_t noOfBytesToReceive = sizeof(rxExpectedMessage) - 1;
    char rxReadMsg[sizeof(rxExpectedMessage)];

    while (expectedUserMsgRecieved == false)
    {
        /* Read user message over UART */
        uint32_t byteIndex = 0;
        while (noOfBytesToReceive > 0)
        {
            status = UART2_read(uart, &rxReadMsg[byteIndex], 1, NULL);
            if (status != UART2_STATUS_SUCCESS)
            {
                /* UART2_read() failed */
                while (1) {}
            }

            /* Check if unexpected message character read */
            if (rxReadMsg[byteIndex] != rxExpectedMessage[byteIndex])
            {
                /* Unexpected message character read.
                 *   - Re-arm SYSTIM channel 1 event capture
                 *   - Set captured time to zero
                 *   - Set expected number of bytes to receive
                 */
                capturedTimeUsec = 0;
                HWREG(SYSTIM_BASE + SYSTIM_O_CH1CFG) |= SYSTIM_CH1CFG_MODE_CAPT;
                byteIndex          = 0;
                noOfBytesToReceive = sizeof(rxExpectedMessage) - 1;
            }
            else
            {
                /* Expected message character read */
                byteIndex++;
                noOfBytesToReceive--;
            }
        }

        /* Signal that expected message was received. This will force exit of while-loop. */
        expectedUserMsgRecieved = true;
    }

    /*
     * ======== Demonstrate capture of UART TX event =========
     */

    /* Expected message read.
     *  - Store captured time
     *  - Disable edge detection on UART RX pin within IOC
     *  - Clear any edge event status for UART RX pin
     *  - Configure to generate a GPIO event when a falling edge is detected
     *    on the UART TX pin. This requires the IOC pin input buffer to be enabled.
     *  - Re-arm SYSTIM channel 1 capture for detection of event signal from GPIO
     *    with rising edge.
     *  - Note that the DIO number dependent IOCx register is addressed by using
     *    times four (* 4) as a DIO to register offset conversion.
     */
    rxTimestampUsec  = capturedTimeUsec;
    capturedTimeUsec = 0;

    HWREG(IOC_BASE + (IOC_O_IOC0 + (CONFIG_GPIO_UART2_0_RX * 4))) &= ~IOC_IOC0_EDGEDET_EDGE_NEG;
    GPIO_clearInt(CONFIG_GPIO_UART2_0_RX);
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) = GPIO_EVTCFG_DIOSEL_M & (CONFIG_GPIO_UART2_0_TX << GPIO_EVTCFG_DIOSEL_S);
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) |= GPIO_EVTCFG_EVTEN_ENABLE;
    HWREG(IOC_BASE + (IOC_O_IOC0 + (CONFIG_GPIO_UART2_0_TX * 4))) |= (IOC_IOC0_EDGEDET_EDGE_NEG |
                                                                      IOC_IOC0_INPEN_ENABLE);
    HWREG(SYSTIM_BASE + SYSTIM_O_CH1CFG) = (SYSTIM_CH1CFG_INP_RISE | SYSTIM_CH1CFG_MODE_CAPT);

    /* Send timestamp for the detected UART RX start bit and then disable edge detection on UART TX pin within IOC */
    integerToAscii(rxTimestampUsec, timestampAsciiArray);
    bytesWritten = 0;
    UART2_write(uart, rxCaptureMessage, sizeof(rxCaptureMessage) - 1, &bytesWritten);
    HWREG(IOC_BASE + (IOC_O_IOC0 + (CONFIG_GPIO_UART2_0_TX * 4))) &= ~(IOC_IOC0_EDGEDET_EDGE_NEG |
                                                                       IOC_IOC0_INPEN_ENABLE);
    bytesWritten = 0;
    UART2_write(uart, timestampAsciiArray, sizeof(timestampAsciiArray), &bytesWritten);

    /* Send timestamp for detected UART TX start bit */
    txTimestampUsec  = capturedTimeUsec;
    capturedTimeUsec = 0;
    integerToAscii(txTimestampUsec, timestampAsciiArray);
    bytesWritten = 0;
    UART2_write(uart, txCaptureMessage, sizeof(txCaptureMessage) - 1, &bytesWritten);
    bytesWritten = 0;
    UART2_write(uart, timestampAsciiArray, sizeof(timestampAsciiArray), &bytesWritten);

    /*
     * ======== Demonstrate capture of GPIO event =========
     */

    /* Construct semaphore */
    semHandle = SemaphoreP_constructBinary(&semStruct, 0);

    /* Configure for detection of GPIO falling edge event on BUTTON_0 DIO.
     * This configuration will enable edge detection in IOC.
     */
    GPIO_setConfig(CONFIG_GPIO_BUTTON_0, GPIO_CFG_IN_PU | GPIO_CFG_IN_INT_FALLING);

    /* Enable GPIO to publish event on BUTTON_0 DIO to EVTSVT */
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) = GPIO_EVTCFG_DIOSEL_M & (CONFIG_GPIO_BUTTON_0 << GPIO_EVTCFG_DIOSEL_S);
    HWREG(GPIO_BASE + GPIO_O_EVTCFG) |= GPIO_EVTCFG_EVTEN_ENABLE;

    /* Loop forever listening for GPIO events from BUTTON_0 pin */
    while (1)
    {
        /* Write GPIO start message over UART */
        bytesWritten = 0;
        UART2_write(uart, gpioStartMessage, sizeof(gpioStartMessage) - 1, &bytesWritten);

        /* Re-arm SYSTIM channel 1 capture on falling edge.
         * The SYSTIM:CH1CFG.MODE bit will be cleared by HW at capture event.
         */
        HWREG(SYSTIM_BASE + SYSTIM_O_CH1CFG) |= (SYSTIM_CH1CFG_INP_FALL | SYSTIM_CH1CFG_MODE_CAPT);

        /* Wait for the user to press BUTTON_0 before proceeding.
         * Disallow transition to standby power mode since the SYSTIM module
         * is not running during standby.
         */
        Power_setConstraint(PowerLPF3_DISALLOW_STANDBY);
        SemaphoreP_pend(semHandle, SemaphoreP_WAIT_FOREVER);

        /* Send timestamp for GPIO input event. Timestamp has already been captured by SYSTIM interrupt. */
        gpioTimestampUsec = capturedTimeUsec;
        capturedTimeUsec  = 0;
        integerToAscii(gpioTimestampUsec, timestampAsciiArray);
        bytesWritten = 0;
        UART2_write(uart, gpioCaptureMessage, sizeof(gpioCaptureMessage) - 1, &bytesWritten);
        bytesWritten = 0;
        UART2_write(uart, timestampAsciiArray, sizeof(timestampAsciiArray), &bytesWritten);
    }
}
