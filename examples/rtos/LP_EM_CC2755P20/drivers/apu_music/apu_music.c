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
 *  ======== apu_music.c ========
 */
#include <stdint.h>
#include <stddef.h>
#include <complex.h>
#include <math.h>

/* Driver Header files */
#include <ti/drivers/GPIO.h>
#include <ti/drivers/apu/APULPF3.h>

/* Display Header files */
#include <ti/display/Display.h>
#include <ti/display/DisplayUart2.h>

/* DPL header files */
#include <ti/drivers/dpl/ClockP.h>

/* Driver configuration */
#include "ti_drivers_config.h"
#include "apu_music_data.h"

#define INPUT_LEN          74  /* Number of pct values in a pct vector, i.e input size */
#define SMOOTHING_MAT_SIZE 24  /* Spatial smoothing matrix size */
#define MUSIC_LEN          512 /* Maximum number of elements in final MUSIC vector */

#define FB_AVG      1           /* Use forward-backward averaging on covariance matrix */
#define MEMORY_SIZE (1024 - 50) /* How much APU memory to use */

float complex inputBuffer[INPUT_LEN];
float pMusicReal[MUSIC_LEN];

float complex *apuMem = (float complex *)APULPF3_MEM_BASE;

float timeInfo(uint32_t startTick, uint32_t endTick)
{
    float elapsedTime;
    uint32_t tickPeriod = ClockP_getSystemTickPeriod();
    uint32_t t1;

    if (endTick > startTick)
    {
        elapsedTime = (endTick - startTick) * tickPeriod;
    }
    else
    {
        /* Timer rolled over */
        t1 = UINT32_MAX - startTick;

        elapsedTime = (t1 + endTick) * tickPeriod;
    }
    return elapsedTime;
}

/*
 *  ======== mainThread ========
 */
void *mainThread(void *arg0)
{

    /* Ininitalize UART display parameters for displaying results */
    Display_Params params;
    Display_Params_init(&params);
    params.lineClearMode   = DISPLAY_CLEAR_NONE;
    Display_Handle hSerial = Display_open(Display_Type_UART, &params);

    /* Check if the selected Display type was found and successfully opened */
    if (hSerial)
    {
        Display_printf(hSerial, 0, 0, "Hello MUSIC!\n");
    }
    else
    {
        while (1) {}
    }

    APULPF3_init();

    /* Start counting MUSIC execution time */
    uint32_t tStart = 0;
    tStart          = ClockP_getSystemTicks();

    /* Load MUSIC data into memory */
    for (uint32_t i = 0; i < INPUT_LEN; i++)
    {
        inputBuffer[i] = pctDist16Real[i] + pctDist16Imag[i] * I;
    }

    /*
     * Step 1: Covariance matrix and spatial smoothing.
     * The resulting matrix is upper-triangular (including diagonal)
     */
    APULPF3_startOperationSequence();

    APULPF3_ComplexTriangleMatrix covMat = {.data = apuMem, .size = SMOOTHING_MAT_SIZE};

    APULPF3_ComplexVector smoothingVec = {.data = inputBuffer, .size = INPUT_LEN};

    APULPF3_covMatrixSpatialSmoothing(&smoothingVec, SMOOTHING_MAT_SIZE, FB_AVG, &covMat);

    /*
     * Step 2: Eigenvalue and eigenvector computation
     * Jacobi EVD produces two results, one at the input and one at the provided
     * output pointer.
     * Here we are placing the eigenvector matrix near the end of APU memory.
     */

    uint32_t eigVecsOffset = MEMORY_SIZE - SMOOTHING_MAT_SIZE * SMOOTHING_MAT_SIZE;

    APULPF3_ComplexVector eigVecs = {.data = apuMem + eigVecsOffset, .size = SMOOTHING_MAT_SIZE * SMOOTHING_MAT_SIZE};

    APULPF3_jacobiEVD(&covMat, 3, 0.5, 1e-8, &eigVecs);

    APULPF3_ComplexTriangleMatrix eigVals = {.data = apuMem, .size = SMOOTHING_MAT_SIZE};
    /* Now we extract the eigenvalues from the triangular matrix */
    float eigvalVec[SMOOTHING_MAT_SIZE];
    float *apuEigVals = (float *)eigVals.data;
    uint32_t diagAddr = 0;

    for (uint32_t i = 0; i < SMOOTHING_MAT_SIZE; ++i)
    {
        /* Access elements along the diagonal */
        diagAddr     = diagAddr + 2 * i;
        /* Since a complex float consists of two floats after each other
         * in memory, we can access the real part by multiplying the index by 2.
         * Casting the array to float prevents back-to-back reads to APU memory
         * which is not supported.
         */
        eigvalVec[i] = apuEigVals[diagAddr * 2];
    }

    /*
     * Step 3: Source Enumeration
     * AREG:
     * This method is based on the difference between the eigval
     * noise-noise gap and noise-signal gap.
     * Lee, Yunseong, Chanhong Park, Taeyoung Kim, Yeongyoon Choi, Kiseon Kim,
     * Dongho Kim, Myung-Sik Lee, and Dongkeun Lee.
     * Source Enumeration Approaches Using Eigenvalue Gaps and Machine Learning
     * Based Threshold for Direction-of-Arrival Estimation.
     * Applied Sciences 11, no. 4 (February 23, 2021): 1942.
     * https://doi.org/10.3390/app11041942.
     */

    /* Eigenvalues are non-negative and sorted descendingly,
     * So deltaIp and sumDelta should always be >= 0
     */
    float AREGMax   = 0;
    uint32_t posMax = 0;
    for (uint32_t i = 0; i < SMOOTHING_MAT_SIZE - 2; i++)
    {
        float deltaIp  = eigvalVec[SMOOTHING_MAT_SIZE - 3 - i] - eigvalVec[SMOOTHING_MAT_SIZE - 2 - i];
        float sumDelta = eigvalVec[SMOOTHING_MAT_SIZE - 2 - i] - eigvalVec[SMOOTHING_MAT_SIZE - 1];
        if (sumDelta != 0.0f)
        {
            float sumavg = sumDelta / (i + 1);
            float areg   = deltaIp / sumavg;
            if (areg > AREGMax)
            {
                AREGMax = areg;
                posMax  = i;
            }
        }
    }

    /*
     * numPaths = Number of sources (multipaths) estimated
     */

    uint32_t numPaths = SMOOTHING_MAT_SIZE - 2 - posMax;

    /* Step 4: MUSIC spectrum and finding the first peak
     * For each distance step in the range [0:stepLength:maxDistance],
     * we calculate the steering vector and the corresponding
     * music spectrum value.
     */

    /* Set up pointers into APU memory */
    complex float *pSteeringVec    = apuMem;
    complex float *pENMultSteering = apuMem + SMOOTHING_MAT_SIZE;
    complex float *pMusic          = pENMultSteering + (SMOOTHING_MAT_SIZE - numPaths);

    float pMusicMin   = 100000;
    float maxDistance = 50;
    float stepLength  = 3e2 / 2048;

    uint32_t pMusicLength = (uint32_t)(maxDistance / stepLength + 1.0f);

    for (uint32_t i = 0; i < pMusicLength; i++)
    {
        /* Generate steering vec for this i */

        APULPF3_ComplexVector unitCircle = {.data = pSteeringVec, .size = SMOOTHING_MAT_SIZE};

        APULPF3_ComplexVector musicVec = {.data = pMusic, .size = SMOOTHING_MAT_SIZE};

        APULPF3_unitCircle(SMOOTHING_MAT_SIZE, i, 0, 1, &unitCircle);

        /* Noise subspace is last SMOOTHING_MAT_SIZE-numPaths vectors in the eigenvector matrix
         * when they are sorted descendingly
         */
        APULPF3_ComplexMatrix noiseSubspace = {.data = eigVecs.data + numPaths * SMOOTHING_MAT_SIZE,
                                               .rows = SMOOTHING_MAT_SIZE,
                                               .cols = SMOOTHING_MAT_SIZE - numPaths};
        APULPF3_ComplexMatrix unitCircleMat = {.data = unitCircle.data, .rows = 1, .cols = SMOOTHING_MAT_SIZE};
        APULPF3_ComplexMatrix steeringMat = {.data = pENMultSteering, .rows = 1, .cols = SMOOTHING_MAT_SIZE - numPaths};

        APULPF3_matrixMult(&unitCircleMat, &noiseSubspace, &steeringMat);

        APULPF3_ComplexVector steeringVec = {.data = pENMultSteering, .size = SMOOTHING_MAT_SIZE - numPaths};

        /* Dot-product with itself (complex conjugated) */
        APULPF3_dotProduct(&steeringVec, &steeringVec, true, musicVec.data);

        /* Calculate music value and potentially update pMusicMin
         * Casting the array to float prevents back-to-back reads
         */
        float *musicVal = (float *)musicVec.data;
        float musicReal = 1.0f / *musicVal;
        pMusicReal[i]   = musicReal;

        if (musicReal < pMusicMin)
        {
            pMusicMin = musicReal;
        }
    }

    APULPF3_stopOperationSequence();

    /* Step 5: Final distance estimate.
     * Look for the position where the value starts to go down.
     */

    uint32_t position = 0;
    bool foundPos     = false;
    float limit       = 8 * pMusicMin;
    for (uint32_t i = 0; i < pMusicLength - 1; ++i)
    {
        if (((pMusicReal[i]) > (pMusicReal[i + 1])) && (pMusicReal[i] > limit))
        {
            position = i;
            foundPos = true;
            break;
        }
    }
    if (!foundPos)
    {
        position = pMusicLength - 1;
    }

    /* Translate position into distance
     * estimate. Divide by 2 due to roundtrip
     */
    float distanceEstimate = position * stepLength * 0.5f;

    /* Finish counting execution time and convert into milliseconds */
    uint32_t tEnd = ClockP_getSystemTicks();
    float runtime = timeInfo(tStart, tEnd) / 1000;

    Display_printf(hSerial, 1, 0, "Estimated distance: %f m\n", (double)distanceEstimate);
    Display_printf(hSerial, 2, 0, "MUSIC execution time: %f ms\n", (double)runtime);

    return (NULL);
}
