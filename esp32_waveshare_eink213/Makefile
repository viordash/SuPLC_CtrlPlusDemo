SHELL := /bin/bash

export ROOT_DIR := $(abspath $(PWD)/../)
export TOOLS_DIR := $(ROOT_DIR)/Tools
export TOOLS_BINDIR := $(TOOLS_DIR)/bin

export DEVICE_VERSION:=1

export IDF_PATH:=$(abspath $(PWD)/../SDK/esp-idf/)
export IDF_PATH_FORCE=1
export IDF_TOOLS_PATH:=$(abspath $(PWD)/../SDK/.espressif/)
export IDF_CCACHE_ENABLE=1

$(info ----------      IDF_PATH: $(IDF_PATH))
$(info ----------IDF_TOOLS_PATH: $(IDF_TOOLS_PATH))

TARGETS:=esp32
export IDF_TARGET=esp32

SUPLC_LIB_PATH:=$(PWD)/../SDK/SuPLC-lib
PLARFORMS_PATH:=$(PWD)/../SDK/Platforms

LIB_SYM_LINKS:=$(SUPLC_LIB_PATH)/Buttons/ \
	$(SUPLC_LIB_PATH)/Datetime/ \
	$(SUPLC_LIB_PATH)/Display/ \
	$(SUPLC_LIB_PATH)/Hashing/ \
	$(SUPLC_LIB_PATH)/Hotreload/ \
	$(SUPLC_LIB_PATH)/HttpServer/ \
	$(SUPLC_LIB_PATH)/LogicProgram/ \
	$(SUPLC_LIB_PATH)/Maintenance/ \
	$(SUPLC_LIB_PATH)/Settings/ \
	$(SUPLC_LIB_PATH)/Storage/ \
	$(SUPLC_LIB_PATH)/Utils/ \
	$(SUPLC_LIB_PATH)/WiFi/

PLARFORM_SYM_LINKS:=$(PLARFORMS_PATH)/display/EPD_2in13_V4/ \
	$(PLARFORMS_PATH)/gpio/esp32_waveshare_eink213/ \
	$(PLARFORMS_PATH)/network/esp32/ \
	$(PLARFORMS_PATH)/os/esp32/ \
	$(PLARFORMS_PATH)/storage/esp32/ \
	$(PLARFORMS_PATH)/system/esp32/ \
	$(PLARFORMS_PATH)/wifi/esp32/

#-------------- Icons
#Small=0 | Medium=1 | Large=2 | XLarge=3
export ICON_CALIBER := 0

RM 	:= rm -rf
MD	:= mkdir -p
BUILD	:= build


include $(TOOLS_DIR)/tools.mk
include $(ROOT_DIR)/icons.mk
include $(ROOT_DIR)/build_version.mk 
include $(ROOT_DIR)/web_files.mk 

COMMON_FLAGS := \
	-DDEVICE_SETTINGS_VERSION=0x${DEVICE_VERSION} \
	-DBUILD_NUMBER=${BUILD_NUMBER} \
	-DLOG_LEVEL=1 \
	-DSCRATCH_BUFSIZE=16384

COMMON_DEFS := $(patsubst -D%,%,$(COMMON_FLAGS))
WEB_CMAKE_PARAMS+=-DCOMMON_DEFS="$(COMMON_DEFS)"

all: build

project_prepare: .project_prepare clean-apply-patches .apply-patches 

lib_sym_links:
# 	@ln -sf $(PWD)/../SDK/MigrateAnyCppData/MigrateAnyData/ $(PWD)/main/
# 	@ln -sf $(LIB_SYM_LINKS) $(PWD)/main/

clean_lib_sym_links:
# 	@$(RM) $(PWD)/main/MigrateAnyData
# 	@$(RM) $(abspath $(patsubst $(SUPLC_LIB_PATH)/%,$(PWD)/main/%,$(LIB_SYM_LINKS)))

platform_sym_links:
# 	@mkdir -p $(abspath $(patsubst $(PLARFORMS_PATH)/%,$(PWD)/main/BSP/%/..,$(PLARFORM_SYM_LINKS)))
# 	@$(foreach platform,$(PLARFORM_SYM_LINKS),\
# 		ln -sf $(platform) $(abspath $(patsubst $(PLARFORMS_PATH)/%,$(PWD)/main/BSP/%/..,$(platform)));\
# 	)

clean_platform_sym_links:
# 	@$(RM) $(abspath $(patsubst $(PLARFORMS_PATH)/%,$(PWD)/main/BSP/%/..,$(PLARFORM_SYM_LINKS)))

.apply-patches:
# 

clean-apply-patches:
# 

.project_prepare: lib_sym_links platform_sym_links .apply-patches
	$(IDF_PATH)/install.sh $(TARGETS)
	touch "$@"

.PHONY: distclean clean build tools

tools: $(BUILD_ICONS_DIR)

build: .project_prepare tools
	. $(IDF_PATH)/export.sh && idf.py build size $(WEB_CMAKE_PARAMS)

flash: .project_prepare
	. $(IDF_PATH)/export.sh && idf.py flash -b 921600 $(WEB_CMAKE_PARAMS)

menuconfig: .project_prepare
	. $(IDF_PATH)/export.sh && idf.py menuconfig $(WEB_CMAKE_PARAMS)

clean: clean_lib_sym_links clean_platform_sym_links clean-apply-patches
	(. $(IDF_PATH)/export.sh && idf.py clean $(WEB_CMAKE_PARAMS)) || true
	@$(RM) build
	@$(RM) $(TOOLS_BINDIR)
	
distclean: clean
	@$(RM) $(PWD)/main/MigrateAnyData
	@$(RM) $(IDF_TOOLS_PATH)
	@$(RM) .project_prepare
