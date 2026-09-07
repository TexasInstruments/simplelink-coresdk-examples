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

const float pctDist16Real[74] = {-2.4811098e-01, -2.1786949e-01, 3.9197508e-02,  2.0101980e-01,  8.1161090e-02,
                                 -5.3795967e-02, -1.8073385e-02, 2.0432769e-05,  1.3806594e-02,  -1.0005196e-02,
                                 -2.3907995e-02, -4.5822552e-02, -4.8136610e-02, 6.8471836e-03,  8.1065456e-02,
                                 1.0759124e-01,  3.7449966e-02,  1.3859289e-02,  -9.4926586e-02, -2.1452408e-01,
                                 -3.9437368e-02, 1.3389311e-01,  1.4008589e-01,  5.2878679e-02,  -1.0398838e-02,
                                 -5.5528549e-02, -4.8359163e-02, -4.8160011e-02, -3.1736389e-02, -1.8275717e-02,
                                 3.1961369e-02,  5.6766069e-02,  8.9293155e-02,  7.5888058e-02,  -3.5702901e-02,
                                 -1.6650324e-01, -1.3892780e-01, 1.0779874e-01,  7.0278119e-02,  1.8670446e-02,
                                 1.2893333e-02,  3.6121036e-02,  9.5952947e-03,  -3.4554878e-02, -4.6506465e-02,
                                 -3.8326874e-02, -1.4966068e-02, 1.2490250e-02,  2.1739104e-02,  9.3212776e-02,
                                 7.9840894e-02,  -7.7124641e-02, -9.7956824e-02, -4.5489780e-02, 1.4521082e-02,
                                 5.7367920e-02,  5.2288769e-02,  -1.8224159e-02, -4.3705626e-02, 3.2497658e-02,
                                 -1.8292023e-02, -2.8230014e-02, 9.6550319e-02,  3.3780902e-02,  -1.5214539e-01,
                                 -1.0976533e-01, 8.2867918e-02,  1.4363277e-01,  2.7197192e-02,  -1.7037584e-03,
                                 -4.1341161e-02, -1.3282977e-01, -5.1271506e-02, 9.7650475e-02};
const float pctDist16Imag[74] = {-8.7093223e-02, 1.5027912e-01,  2.3573023e-01,  5.5929313e-02,  -1.2520745e-01,
                                 -5.8586310e-02, 1.1698606e-02,  7.3647964e-05,  -1.3108223e-02, -2.5183097e-02,
                                 -1.9551721e-02, -1.8793775e-03, 4.7913045e-02,  8.0567050e-02,  6.6162112e-02,
                                 -3.5700205e-02, -7.9390293e-02, -9.3246304e-02, -1.3500960e-01, 4.7950331e-02,
                                 1.9616353e-01,  1.2237198e-01,  -2.9062444e-02, -8.9795859e-02, -8.8507025e-02,
                                 -4.6571075e-02, -9.3286464e-03, 1.5758848e-02,  3.2109407e-02,  5.4592633e-02,
                                 5.6653902e-02,  3.7643546e-02,  -6.6006191e-03, -9.4733760e-02, -1.3723377e-01,
                                 -5.7912043e-02, 1.5540450e-01,  1.2964111e-01,  -2.6105787e-02, -2.0072788e-02,
                                 -4.9118604e-03, -1.5488474e-02, -5.5845683e-02, -3.9351473e-02, -4.0297260e-03,
                                 2.1940417e-02,  4.4123703e-02,  3.8554845e-02,  4.1891802e-02,  3.0899893e-02,
                                 -1.1740376e-01, -9.4229812e-02, 6.4897384e-03,  7.0525007e-02,  5.8906971e-02,
                                 3.3959441e-02,  -3.5593571e-02, -5.5714259e-02, 2.5725193e-02,  1.3978284e-02,
                                 -2.9377288e-02, 5.3382804e-02,  1.7147149e-02,  -1.2444908e-01, -6.6266987e-02,
                                 1.3057423e-01,  1.4311156e-01,  -1.8358645e-02, -8.9365876e-02, -6.0498340e-02,
                                 -7.1851542e-02, -1.0719864e-02, 1.4905735e-01,  7.0663070e-02};
