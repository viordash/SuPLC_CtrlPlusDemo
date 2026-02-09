#include "LogicProgram/Inputs/ComparatorNe.h"
#include "icons/cmp_not_equal.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorNe::ComparatorNe() : CommonComparator() {
}

ComparatorNe::ComparatorNe(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorNe::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorNe::CompareFunction() {
    return Input->ReadValue() != ref_percent04;
}

TvElementType ComparatorNe::GetElementType() {
    return TvElementType::et_ComparatorNe;
}

ComparatorNe *ComparatorNe::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorNe:
            return static_cast<ComparatorNe *>(common_comparator);

        default:
            return NULL;
    }
}