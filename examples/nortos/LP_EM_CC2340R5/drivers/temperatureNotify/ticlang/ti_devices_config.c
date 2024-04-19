/*
 *  ======== ti_devices_config.c ========
 *  Customer Configuration for CC23X0 devices.
 *
 *  DO NOT EDIT - This file is generated by the SysConfig tool.
 *
 */

#ifndef __TI_DEVICES_CC23X0_CONFIG_C__
#define __TI_DEVICES_CC23X0_CONFIG_C__

#include <ti/devices/DeviceFamily.h>
#include DeviceFamily_constructPath(inc/hw_ccfg.h)

#if defined(__IAR_SYSTEMS_ICC__)
__root const ccfg_t ccfg @ ".ccfg" =
#elif defined(__TI_COMPILER_VERSION__)
#pragma DATA_SECTION(ccfg, ".ccfg")
#pragma RETAIN(ccfg)
const ccfg_t ccfg =
#elif defined(__llvm__)
const ccfg_t ccfg __attribute__((section(".ccfg"), retain)) =
#else
const ccfg_t ccfg __attribute__((section(".ccfg"), used)) =
#endif
{
    .bootCfg.pBldrVtor = XCFG_BC_PBLDR_UNDEF,

    .bootCfg.bldrParam.serialRomBldrParamStruct.bldrEnabled = XCFG_BC_BLDR_DIS,
    .bootCfg.bldrParam.serialRomBldrParamStruct.serialIoCfgIndex = 0,
    .bootCfg.bldrParam.serialRomBldrParamStruct.pinTriggerDio = 0,
    .bootCfg.bldrParam.serialRomBldrParamStruct.pinTriggerEnabled = XCFG_BC_PINTRIG_DIS,
    .bootCfg.bldrParam.serialRomBldrParamStruct.pinTriggerLevel = XCFG_BC_PINTRIG_LEVEL_LO,
    .bootCfg.pAppVtor = (void*)0x0,

    /* Set to 0. This checksum must be calculated and updated prior to
     * programming the application to the device (this is normally handled as a
     * post-build step, or by the flashloader).
     */
    .bootCfg.crc32 = 0x00000000,

    .hwOpts = {0xffffffff, 0xffffffff},

    .permissions.allowDebugPort = CCFG_PERMISSION_ALLOW,
    .permissions.allowEnergyTrace = CCFG_PERMISSION_ALLOW,
    .permissions.allowFlashVerify = CCFG_PERMISSION_ALLOW,
    .permissions.allowFlashProgram = CCFG_PERMISSION_ALLOW,
    .permissions.allowChipErase = CCFG_PERMISSION_ALLOW,
    .permissions.allowToolsClientMode = CCFG_PERMISSION_ALLOW,
    .permissions.allowFakeStby = CCFG_PERMISSION_ALLOW,
    .permissions.allowReturnToFactory = CCFG_PERMISSION_ALLOW,

    .misc.saciTimeoutOverride = 1U,
    .misc.saciTimeoutExp = 7,

    .flashProt.writeEraseProt.mainSectors0_31 = 0xffffffff,
    .flashProt.writeEraseProt.mainSectors32_255 = 0xffffffff,

    .flashProt.writeEraseProt.ccfgSector = 0,
    .flashProt.writeEraseProt.fcfgSector = 0,
    .flashProt.writeEraseProt.engrSector = 0,

    .flashProt.res = 0xFFFFFFFFU,

    .flashProt.chipEraseRetain.mainSectors0_31 = 0x0,
    .flashProt.chipEraseRetain.mainSectors32_255 = 0x0,

    /* Set to 0. This checksum (across hwOpts through hwInitCopyList) must be
     * calculated and updated prior to programming the application to the device
     * (this is normally handled as a post-build step, or by the flashloader).
     */
    .crc32 = 0x00000000,

    /* Set to 0. This checksum must be calculated and updated prior to
     * programming the application to the device (this is normally handled as a
     * post-build step, or by the flashloader).
     */
    .userRecord.crc32 = 0x00000000,

    .debugCfg.authorization = CCFG_DBGAUTH_DBGOPEN,
    .debugCfg.allowBldr = CCFG_DBGBLDR_ALLOW,
    .debugCfg.pwdId = {0x01, 0x01, 0x02, 0x03, 0x05, 0x08, 0x0d, 0x15},
    .debugCfg.pwdHash = {0x6d, 0xd7, 0xe4, 0x36, 0xeb, 0xf4, 0x31, 0xdf,
                         0x95, 0xae, 0x15, 0xee, 0x03, 0xba, 0x8e, 0xe4,
                         0xc4, 0xc6, 0x3f, 0xd8, 0x45, 0x3f, 0x67, 0x5e,
                         0x74, 0xd7, 0xc2, 0x01, 0x2c, 0x90, 0x58, 0xe5},

    /* Set to 0. This checksum must be calculated and updated prior to
     * programming the application to the device (this is normally handled as a
     * post-build step, or by the flashloader).
     */
    .debugCfg.crc32 = 0x00000000,
};

#endif // __TI_DEVICES_CC23X0_CONFIG_C__