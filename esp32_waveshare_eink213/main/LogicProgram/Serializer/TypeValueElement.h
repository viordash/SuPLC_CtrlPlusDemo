#pragma once

#include <stdint.h>
#include <unistd.h>

enum TvElementType : uint8_t {
    et_Undef = 0,
    et_Wire,
    et_InputNC,
    et_InputNO,
    et_TimerSecs,
    et_TimerMSecs,
    et_ComparatorEq,
    et_ComparatorNe,
    et_ComparatorGE,
    et_ComparatorGr,
    et_ComparatorLE,
    et_ComparatorLs,
    et_DirectOutput,
    et_SetOutput,
    et_ResetOutput,
    et_IncOutput,
    et_DecOutput,
    et_Indicator,
    et_WiFiBinding,
    et_WiFiStaBinding,
    et_WiFiApBinding,
    et_Settings,
    et_DateTimeBinding,
    et_ButtonControl,
    et_ToggleButtonControl,
    et_TrackBarControl,
    et_SquareWaveGenerator,
    et_ContinuationIn,
    et_ContinuationOut,
};

struct __attribute__((packed)) TvElement {
    TvElementType type;
    // uint8_t value[];
};

static inline bool IsInputElement(TvElementType element_type) {
    switch (element_type) {
        case et_InputNC:
        case et_InputNO:
        case et_TimerSecs:
        case et_TimerMSecs:
        case et_ComparatorEq:
        case et_ComparatorNe:
        case et_ComparatorGE:
        case et_ComparatorGr:
        case et_ComparatorLE:
        case et_ComparatorLs:
        case et_SquareWaveGenerator:
            return true;
        default:
            return false;
    }
}

static inline bool IsOutputElement(TvElementType element_type) {
    switch (element_type) {
        case et_DirectOutput:
        case et_SetOutput:
        case et_ResetOutput:
        case et_IncOutput:
        case et_DecOutput:
            return true;
        default:
            return false;
    }
}

static inline bool IsIndicatorElement(TvElementType element_type) {
    switch (element_type) {
        case et_Indicator:
            return true;
        default:
            return false;
    }
}

static inline bool IsBindingElement(TvElementType element_type) {
    switch (element_type) {
        case et_WiFiBinding:
        case et_WiFiStaBinding:
        case et_WiFiApBinding:
        case et_DateTimeBinding:
            return true;
        default:
            return false;
    }
}

static inline bool IsSettingsElement(TvElementType element_type) {
    switch (element_type) {
        case et_Settings:
            return true;
        default:
            return false;
    }
}

static inline bool IsControlsElement(TvElementType element_type) {
    switch (element_type) {
        case et_ButtonControl:
        case et_ToggleButtonControl:
        case et_TrackBarControl:
            return true;
        default:
            return false;
    }
}

static inline bool IsContinuationInElement(TvElementType element_type) {
    switch (element_type) {
        case et_ContinuationIn:
            return true;
        default:
            return false;
    }
}