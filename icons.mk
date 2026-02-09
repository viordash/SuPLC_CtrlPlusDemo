
export ICONS_DIR := $(ROOT_DIR)/Resources/Icons

export REQUIRED_ICONS := $(wildcard $(ICONS_DIR)/*.ico)
export BUILD_ICONS_DIR	:= $(BUILD)/icons

$(BUILD_ICONS_DIR): $(ICO2CODE)
	@echo "----------------------- Create icons" 
	@$(MD) $(BUILD_ICONS_DIR)
	@$(foreach icon_file,$(REQUIRED_ICONS), \
		$(ICO2CODE) $(icon_file) $(patsubst $(ICONS_DIR)/%.ico,$(BUILD_ICONS_DIR)/%.h,$(icon_file)) $(ICON_CALIBER) \
		|| { $(RM) $(BUILD_ICONS_DIR); exit 1; }; \
	)	