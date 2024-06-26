SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ../../../../../../..)

include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

CC = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"
LNK = "$(GCC_ARMCOMPILER)/bin/arm-none-eabi-gcc"

SYSCFG_CMD_STUB = $(SYSCONFIG_TOOL) --compiler gcc --product $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/.metadata/product.json
SYSCFG_FILES := $(shell $(SYSCFG_CMD_STUB) --listGeneratedFiles --listReferencedFiles --output . ../../tirtos7/aesKeyAgreement.syscfg)

SYSCFG_C_FILES = $(filter %.c,$(SYSCFG_FILES))
SYSCFG_H_FILES = $(filter %.h,$(SYSCFG_FILES))
SYSCFG_OPT_FILES = $(filter %.opt,$(SYSCFG_FILES))

OBJECTS = rxTask.obj txTask.obj common.obj tirtos7_main_tirtos.obj $(patsubst %.c,%.obj,$(notdir $(SYSCFG_C_FILES)))
NAME = aesKeyAgreement

CFLAGS += -I../.. \
    -I. \
    $(addprefix @,$(SYSCFG_OPT_FILES)) \
    -D__STRICT_ANSI__ \
    -O3 \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    "-I$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source/ti/posix/gcc" \
    -mcpu=cortex-m33 \
    -mthumb \
    -std=c99 \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16 \
    -ffunction-sections \
    -fdata-sections \
    -g \
    -gstrict-dwarf \
    -Wall \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include/newlib-nano" \
    "-I$(GCC_ARMCOMPILER)/arm-none-eabi/include"

LFLAGS += -Wl,-T,../../tirtos7/gcc/cc13x4_cc26x4_tirtos7.lds \
    "-Wl,-Map,$(NAME).map" \
    -mcpu=cortex-m33 \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/source" \
    "-L$(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/kernel/tirtos7/packages" \
    ti_utils_build_linker.cmd.genlibs \
    -mthumb \
    -mfloat-abi=hard \
    -mfpu=fpv5-sp-d16 \
    -nostartfiles \
    -static \
    -Wl,--gc-sections \
    -lgcc \
    -lc \
    -lm \
    -lnosys \
    --specs=nano.specs

all: $(NAME).out

.INTERMEDIATE: syscfg
$(SYSCFG_FILES): syscfg
	@ echo generation complete

syscfg: ../../tirtos7/aesKeyAgreement.syscfg
	@ echo Generating configuration files...
	@ $(SYSCFG_CMD_STUB) --output $(@D) $<

define C_RULE
$(basename $(notdir $(1))).obj: $(1) $(SYSCFG_H_FILES)
	@ echo Building $$@
	@ $(CC) $(CFLAGS) $$< -c -o $$@
endef

$(foreach c_file,$(SYSCFG_C_FILES),$(eval $(call C_RULE,$(c_file))))

rxTask.obj: ../../rxTask.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

txTask.obj: ../../txTask.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

common.obj: ../../common.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

tirtos7_main_tirtos.obj: ../../tirtos7/main_tirtos.c $(SYSCFG_H_FILES)
	@ echo Building $@
	@ $(CC) $(CFLAGS) $< -c -o $@

$(NAME).out: $(OBJECTS)
	@ echo linking $@
	@ $(LNK)  $(OBJECTS)  $(LFLAGS) -o $(NAME).out

clean:
	@ echo Cleaning...
	@ $(RM) $(OBJECTS) > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).out > $(DEVNULL) 2>&1
	@ $(RM) $(NAME).map > $(DEVNULL) 2>&1
	@ $(RM) $(SYSCFG_FILES)> $(DEVNULL) 2>&1
