
#use for main/settings.cpp
DEVICE_VERSION_seconds:=$(shell date -d '$(DEVICE_VERSION)' +'%s')
current_seconds:=$(shell date +'%s')
minutes_since=$(shell echo "($(current_seconds) - $(DEVICE_VERSION_seconds)) / 60" | bc)
BUILD_NUMBER:=$(minutes_since)
$(info ----build version-----------------v$(DEVICE_VERSION).$(BUILD_NUMBER)------ )