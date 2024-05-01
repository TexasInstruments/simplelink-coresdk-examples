SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR ?= $(abspath ./)
include $(SIMPLELINK_CC13XX_CC26XX_SDK_INSTALL_DIR)/imports.mak

# Find all subdirectories
EXAMPLE-SUBDIRS = $(wildcard examples/*/*/.)
CLEAN-SUBDIRS = $(wildcard examples/*/*/.)

# Add the SDK subdirectories 
F2-SDK-SUBMODULE-DIR=cc13xx_cc26xx_sdk
F3-SDK-SUBMODULE-DIR=simplelink-lowpower-f3-sdk

.PHONY: $(F3-SDK-SUBMODULE-DIR) $(F2-SDK-SUBMODULE-DIR) $(EXAMPLE-SUBDIRS)

all: $(F3-SDK-SUBMODULE-DIR) $(F2-SDK-SUBMODULE-DIR) $(EXAMPLE-SUBDIRS)

$(F3-SDK-SUBMODULE-DIR):
	@ $(MAKE) -C $@

$(F2-SDK-SUBMODULE-DIR):
	@ $(MAKE) -C $@

$(EXAMPLE-SUBDIRS):
	@ $(MAKE) -C $@


clean: $(CLEAN-SUBDIRS)

$(CLEAN-SUBDIRS):
	@ $(MAKE) clean -C $@

