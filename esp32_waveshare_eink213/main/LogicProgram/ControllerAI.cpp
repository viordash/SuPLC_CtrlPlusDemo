#include "LogicProgram/ControllerAI.h"
#include "DeviceIO.h"
#include "LogicProgram/Controller.h"
#include "LogicProgram/LogicElement.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_ControllerAI = "ControllerAI";

extern CurrentSettings::device_settings settings;

ControllerAI::ControllerAI(adc_input adc, const char *name) : ControllerBaseInput(name) {
    this->adc = adc;
}

void ControllerAI::FetchValue() {
    if (!Controller::RequestWakeupMs((void *)((uintptr_t)&DeviceIO::AnalogInputs + (uintptr_t)adc),
                                     settings.adc.scan_period_ms,
                                     ProcessWakeupRequestPriority::pwrp_Idle)) {
        return;
    }
    required_reading = false;

    uint16_t val_10bit = get_adc_value(adc);
    uint8_t percent04 = val_10bit / 4;
    UpdateValue(percent04);
    log_d(TAG_ControllerAI, "%s, fetch value", name);
}
