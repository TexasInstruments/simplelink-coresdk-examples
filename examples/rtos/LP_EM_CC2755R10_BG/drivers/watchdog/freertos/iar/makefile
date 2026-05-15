SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/imports.mak

ASM = "$(IAR_ARMCOMPILER)/bin/iasmarm"
CC = "$(IAR_ARMCOMPILER)/bin/iccarm"
LNK = "$(IAR_ARMCOMPILER)/bin/ilinkarm"

SYSCONFIG_GUI_TOOL = $(dir $(SYSCONFIG_TOOL))sysconfig_gui$(suffix $(SYSCONFIG_TOOL))
SYSCFG_CMD_STUB = $(SYSCONFIG_TOOL) --compiler iar --product $(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_GUI_CMD_STUB = $(SYSCONFIG_GUI_TOOL) --compiler iar --product $(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_FILES := $(shell $(SYSCFG_CMD_STUB) --listGeneratedFiles --listReferencedFiles --output . ../../freertos/watchdog.syscfg)

SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))
SYSCFG_OPT_FILES = $(filter %.opt,$(SYSCFG_FILES))

# Enable verbose output by setting VERBOSE=1
V := @
ifeq ($(VERBOSE), 1)
  V :=
endif

OBJECTS = watchdog.obj freertos_main_freertos.obj $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
SYSCFG_ASM_FILES = $(filter %.s,$(SYSCFG_FILES))
OBJECTS += $(patsubst %.s,%.obj,$(notdir $(SYSCFG_ASM_FILES)))


NAME = watchdog

AFLAGS += -I../.. \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/freertos/include" \
    -S \
    --endian little \
    --thumb \
    "-I$(IAR_ARMCOMPILER)/inc/c" \
    --cpu Cortex-M33 \
    --fpu VFPv5_sp

DFLAGS += --debug

CFLAGS += -I../.. \
    -I. \
    $(addprefix -f,$(SYSCFG_OPT_FILES)) \
    -Oh \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/tfm_s/build/cc27xx/production_full/Release/export/tfm/inc" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/tfm/config" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/tfm/interface/include" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/tfm/secure_fw/include" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/psa_crypto/include" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/mbedtls/include" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/mbedtls/ti" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/mbedtls/ti/port" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/kernel/freertos" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/ti/posix/iar" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/freertos/include" \
    "-I$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source/third_party/freertos/portable/IAR/ARM_CM33_NTZ/non_secure" \
    -DMBEDTLS_CONFIG_FILE="<configs/config-hsm.h>" \
    -DMBEDTLS_PSA_CRYPTO_CONFIG_FILE="<configs/config-psa-crypto-hsm.h>" \
    --silent \
    -e \
    --aeabi \
    --thumb \
    --diag_suppress=Pa050 \
    --cpu Cortex-M33 \
    --fpu VFPv5_sp \
    --vla


ifneq ($(CONFIG),RELEASE)
# Add debug flags by default (excluded only when CONFIG=RELEASE)
CFLAGS += $(DFLAGS)
endif

LFLAGS += "-L$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/source" \
    "-L$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/tfm_s" \
    --diag_suppress=Li065 \
    --threaded_lib \
    "-L$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/kernel" \
    -fti_utils_build_linker.cmd.genlibs \
    --config ../../freertos/iar/lpf3_freertos.icf \
    --map "$(NAME).map" \
    --silent \
    "$(if $(filter RELEASE,$(CONFIG)),,--semihosting=iar_breakpoint)" \
    --cpu=Cortex-M33 \
    --fpu VFPv5_sp

all: postbuild

.PHONY: postbuild
postbuild: $(NAME).out
	"$(SIMPLELINK_LOWPOWER_F3_SDK_INSTALL_DIR)/tools/common/crc_tool/crc_tool" patch-image --elf "./$(NAME).out" --symbol-prefix ti_utils_build_GenMap_sym_CRC --output "./$(NAME).out"
	"$(IAR_ARMCOMPILER)/bin/ielftool" --ihex "./$(NAME).out" "./$(NAME).hex"

.INTERMEDIATE: syscfg
$(SYSCFG_FILES): syscfg
	@ echo generation complete

syscfg: ../../freertos/watchdog.syscfg
	@ echo Generating configuration files...
	$(V) $(SYSCFG_CMD_STUB) --output $(@D) $<


# Helpful hint that the user needs to use a standalone SysConfig installation
$(SYSCONFIG_GUI_TOOL):
	$(error $(dir $(SYSCONFIG_TOOL)) does not contain the GUI framework \
        necessary to launch the SysConfig GUI.  Please set SYSCONFIG_TOOL \
        (in your SDK's imports.mak) to a standalone SysConfig installation \
        rather than one inside CCS)

syscfg-gui: ../../freertos/watchdog.syscfg $(SYSCONFIG_GUI_TOOL)
	@ echo Opening SysConfig GUI
	$(V) $(SYSCFG_GUI_CMD_STUB) $<

define ASM_RULE
$(basename $(notdir $(1))).obj: $(1)
	@ echo Building $$@
	$(V) $(ASM) $(AFLAGS) $$< -o $$@
endef

$(foreach asm_file,$(SYSCFG_ASM_FILES),$(eval $(call ASM_RULE,$(asm_file))))

define C_RULE
$(basename $(notdir $(1))).obj: $(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	$(V) $(CC) $(CFLAGS) $$< -o $$@
endef

$(foreach c_file,$(SYSCFG_C_FILES),$(eval $(call C_RULE,$(c_file))))

watchdog.obj: ../../watchdog.c $(SYSCFG_H_FILES)
	@ echo Building $@
	$(V) $(CC) $(CFLAGS) $< -o $@

freertos_main_freertos.obj: ../../freertos/main_freertos.c $(SYSCFG_H_FILES)
	@ echo Building $@
	$(V) $(CC) $(CFLAGS) $< -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	$(V) $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	$(V) $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	$(V) $(RM) $(NAME).out > $(DEVNULL) 2>&1
	$(V) $(RM) $(NAME).map > $(DEVNULL) 2>&1
	$(V) $(RM) $(call SLASH_FIXUP,$(SYSCFG_FILES)) > $(DEVNULL) 2>&1
