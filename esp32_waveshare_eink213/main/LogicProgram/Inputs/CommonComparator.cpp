#include "LogicProgram/Inputs/CommonComparator.h"
#include "LogicProgram/Inputs/ComparatorEq.h"
#include "LogicProgram/Inputs/ComparatorGE.h"
#include "LogicProgram/Inputs/ComparatorGr.h"
#include "LogicProgram/Inputs/ComparatorLE.h"
#include "LogicProgram/Inputs/ComparatorLs.h"
#include "LogicProgram/Serializer/Record.h"
#include "board.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *TAG_CommonComparator = "CommonComparator";

CommonComparator::CommonComparator() : CommonInput() {
    str_reference[0] = 0;
    ref_percent04 = 0;
    str_size = 0;
}

CommonComparator::CommonComparator(uint8_t ref_percent04, const MapIO io_adr) : CommonInput() {
    SetReference(ref_percent04);
    SetIoAdr(io_adr);
}

CommonComparator::~CommonComparator() {
}

void CommonComparator::SetReference(uint8_t ref_percent04) {
    if ((int)ref_percent04 > (int)LogicElement::MaxValue) {
        ref_percent04 = LogicElement::MaxValue;
    }
    this->ref_percent04 = ref_percent04;
    str_size = sprintf(this->str_reference, "%d", ref_percent04);
}

uint8_t CommonComparator::GetReference() {
    return ref_percent04;
}

bool CommonComparator::DoAction(bool prev_elem_changed, LogicItemState prev_elem_state) {
    if (!prev_elem_changed && prev_elem_state != LogicItemState::lisActive) {
        return false;
    }

    bool any_changes = false;
    std::lock_guard<std::mutex> lock(lock_mutex);
    LogicItemState prev_state = state;

    state = LogicItemState::lisPassive;
    if (prev_elem_changed && prev_elem_state == LogicItemState::lisPassive) {
        Input->CancelReadingProcess();
    } else if (prev_elem_state == LogicItemState::lisActive //
               && this->CompareFunction()) {
        state = LogicItemState::lisActive;
    }

    if (state != prev_state) {
        any_changes = true;
    }
    return any_changes;
}

IRAM_ATTR void
CommonComparator::Render(FrameBuffer *fb, LogicItemState prev_elem_state, Point *start_point) {

    CommonInput::Render(fb, prev_elem_state, start_point);

    std::lock_guard<std::mutex> lock(lock_mutex);
    auto bitmap = GetCurrentBitmap(state);
    uint16_t bitmap_left = start_point->x - bitmap->size.width;
    uint16_t bitmap_right = start_point->x - 1;
    uint16_t bitmap_top = start_point->y - (bitmap->size.height / 2) + 1;
    uint16_t bitmap_bottom = bitmap_top + bitmap->size.height;

    bool blink_value_on_editing =
        editable_state == EditableElement::ElementState::des_Editing
        && (CommonComparator::EditingPropertyId)editing_property_id
               == CommonComparator::EditingPropertyId::ccepi_ConfigureReference
        && Blinking_50(fb);

    switch (str_size) {
        case 1:
            if (!blink_value_on_editing) {
                ASSERT(
                    reference_font_1.DrawText(
                        fb,
                        bitmap_left + reference_font_1.GetRightLeftMargin(),
                        bitmap_right - reference_font_1.GetRightLeftMargin(),
                        bitmap_bottom
                            - (reference_font_1.GetHeight() - reference_font_1.GetBottomMargin()),
                        str_reference,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        case 2:
            if (!blink_value_on_editing) {
                ASSERT(
                    reference_font_2.DrawText(
                        fb,
                        bitmap_left + reference_font_2.GetRightLeftMargin(),
                        bitmap_right - reference_font_2.GetRightLeftMargin(),
                        bitmap_bottom
                            - (reference_font_2.GetHeight() - reference_font_2.GetBottomMargin()),
                        str_reference,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
        default:
            if (!blink_value_on_editing) {
                ASSERT(
                    reference_font_3.DrawText(
                        fb,
                        bitmap_left + reference_font_3.GetRightLeftMargin(),
                        bitmap_right - reference_font_3.GetRightLeftMargin(),
                        bitmap_bottom
                            - (reference_font_3.GetHeight() - reference_font_3.GetBottomMargin()),
                        str_reference,
                        FontBase::Align::Center)
                    > 0);
            }
            break;
    }
}

size_t CommonComparator::Serialize(uint8_t *buffer, size_t buffer_size) {
    size_t writed = 0;
    TvElement tvElement;
    tvElement.type = GetElementType();
    if (!Record::Write(&tvElement, sizeof(tvElement), buffer, buffer_size, &writed)) {
        return 0;
    }
    if (!Record::Write(&ref_percent04, sizeof(ref_percent04), buffer, buffer_size, &writed)) {
        return 0;
    }
    auto io_adr = GetIoAdr();
    if (!Record::Write(&io_adr, sizeof(io_adr), buffer, buffer_size, &writed)) {
        return 0;
    }
    return writed;
}

size_t CommonComparator::Deserialize(uint8_t *buffer, size_t buffer_size) {
    size_t readed = 0;
    uint8_t _ref_percent04;
    if (!Record::Read(&_ref_percent04, sizeof(_ref_percent04), buffer, buffer_size, &readed)) {
        return 0;
    }
    if ((int)_ref_percent04 > (int)LogicElement::MaxValue) {
        return 0;
    }

    MapIO _io_adr;
    if (!Record::Read(&_io_adr, sizeof(_io_adr), buffer, buffer_size, &readed)) {
        return 0;
    }
    if (!ValidateMapIO(_io_adr)) {
        return 0;
    }
    SetIoAdr(_io_adr);
    SetReference(_ref_percent04);
    return readed;
}

CommonComparator *CommonComparator::TryToCast(LogicElement *logic_element) {
    switch (logic_element->GetElementType()) {
        case TvElementType::et_ComparatorEq:
        case TvElementType::et_ComparatorNe:
        case TvElementType::et_ComparatorGE:
        case TvElementType::et_ComparatorGr:
        case TvElementType::et_ComparatorLE:
        case TvElementType::et_ComparatorLs:
            return static_cast<CommonComparator *>(logic_element);

        default:
            return NULL;
    }
}

void CommonComparator::SelectPrior() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            CommonInput::SelectPrior();
            return;

        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref <= LogicElement::MaxValue - step_ref) {
                SetReference(ref + step_ref);
            } else {
                SetReference(LogicElement::MaxValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::SelectNext() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            CommonInput::SelectNext();
            return;

        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref >= LogicElement::MinValue + step_ref) {
                SetReference(ref - step_ref);
            } else {
                SetReference(LogicElement::MinValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::PageUp() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref <= LogicElement::MaxValue - faststep_ref) {
                SetReference(ref + faststep_ref);
            } else {
                SetReference(LogicElement::MaxValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::PageDown() {
    uint8_t ref;
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureReference:
            ref = GetReference();
            if (ref >= LogicElement::MinValue + faststep_ref) {
                SetReference(ref - faststep_ref);
            } else {
                SetReference(LogicElement::MinValue);
            }
            break;

        default:
            break;
    }
}

void CommonComparator::Change() {
    switch (editing_property_id) {
        case CommonComparator::EditingPropertyId::ccepi_ConfigureIoAdr:
            log_i(TAG_CommonComparator, "Change");
            editing_property_id = CommonComparator::EditingPropertyId::ccepi_ConfigureReference;
            break;

        default:
            CommonInput::Change();
            return;
    }
}

void CommonComparator::Option() {
}