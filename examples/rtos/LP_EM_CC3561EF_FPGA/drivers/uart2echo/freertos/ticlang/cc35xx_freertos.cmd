/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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

--stack_size=2048
--heap_size=0
--entry_point resetISR

/* Retain interrupt vector table variable                                    */
--retain "*(.resetVecs)"

/* Suppress warnings and errors:                                             */
/* - 10063: Warning about entry point not being _c_int00                     */
/* - 16011, 16012: 8-byte alignment errors. Observed when linking in object  */
/*   files compiled using Keil (ARM compiler)                                */
--diag_suppress=10063,16011,16012

/* Set severity of diagnostics to Remark instead of Warning                  */
/* - 10068: Warning about no matching log_ptr* sections                      */
--diag_remark=10068

#define DRAM_BASE               0x28000000
#define DRAM_SIZE               0x00030000 /* (Static only) DRAM1: 128K + DRAM2: 64K */

/* System memory map */
MEMORY
{
    INT_VEC    (RWX) : origin = 0x00000000, length = 0x0000011C
    CRAM       (RWX) : origin = 0x000003a0, length = 0x00007c60
    FLASH      (RWX) : origin = 0x10000000, length = 0x00040000 /* TODO: Update flash size for non FPGA device. The external flash on FPGAs (M24M02-DRMN6TP) has 256KB (2Mbit) of external flash */
    PERIPH_API (RW)  : origin = 0x45602000, length = 0x0000001F
    DRAM       (RWX) : origin = DRAM_BASE,  length = DRAM_SIZE
    MEM_POOL   (RW)  : origin = 0x28044000, length = 0x00004000
    DB_MEM     (RW)  : origin = 0x45A80000, length = 0x0000FFFF
    PHY_CTX    (RW)  : origin = 0x45900000, length = 0x00010000
    PHY_SCR    (RW)  : origin = 0x45910000, length = 0x00004800
    CPERAM     (RWX) : origin = 0x45C00000, length = 0x00010000 /* 64K PROGRAM MEMORY  */
    MCERAM     (RWX) : origin = 0x45C80000, length = 0x00001000 /* 4K PROGRAM MEMORY   */
    RFERAM     (RWX) : origin = 0x45CA0000, length = 0x00001000 /* 4K PROGRAM MEMORY   */
    MDMRAM     (RWX) : origin = 0x45CC0000, length = 0x00000100 /* 256B PROGRAM MEMORY */
    /* Explicitly placed off target for the storage of logging data.
     * The ARM memory map allocates 1 GB of external memory from 0x60000000 - 0x9FFFFFFF.
     * Unlikely that all of this will be used, so we are using the upper parts of the region.
     * ARM memory map: https://developer.arm.com/documentation/ddi0337/e/memory-map/about-the-memory-map*/
    LOG_DATA (R) : origin = 0x90000000, length = 0x40000        /* 256 KB */
    LOG_PTR  (R) : origin = 0x94000008, length = 0x40000        /* 256 KB */
}

/* Section allocation in memory */
SECTIONS
{
    .resetVecs      :   > INT_VEC
    .ctx_ull        :   > PHY_CTX
    .scr_ull        :   > PHY_SCR
    .cperam         :   > CPERAM         /* CPE CODE */
    .rferam         :   > RFERAM         /* RFE CODE */
    .mceram         :   > MCERAM         /* MCE CODE */
    .mdmram         :   > MDMRAM         /* MDM CODE */
    .db_mem         :   > DB_MEM
    .text.__TI      : { *(.text.__TI*) } > FLASH PALIGN(4)
    .text           :   > FLASH PALIGN(4)
    .const          :   > FLASH PALIGN(4)
    .constdata      :   > FLASH PALIGN(4)
    .pinit          :   > FLASH PALIGN(4)
    .rodata         :   > FLASH PALIGN(4)
    .TI.ramfunc     : {} load=FLASH, run=CRAM, table(BINIT)
    .data           :   > DRAM
    .cinit          :   > FLASH PALIGN(4)
    .bss            :   > DRAM
    .cio            :   > CRAM
    .sysmem         :   > DRAM
    .perif_if       :   > PERIPH_API
    .ARM.exidx      :   > DRAM
    .init_array     :   > FLASH PALIGN(4)
    .binit          :   > FLASH PALIGN(4)
    .mem_pool       :   > MEM_POOL
    .vtable         :   > DRAM
    .args           :   > DRAM
    .stack          :   > DRAM (HIGH)
    .nonretenvar    :   > DRAM
    .emb_text       :   > FLASH PALIGN(4)

    .log_data       :   > LOG_DATA, type = COPY
    .log_ptr        : { *(.log_ptr*) } > LOG_PTR align 4, type = COPY
}
