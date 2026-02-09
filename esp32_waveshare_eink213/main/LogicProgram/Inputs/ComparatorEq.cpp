#include "LogicProgram/Inputs/ComparatorEq.h"
#include "icons/cmp_equal.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorEq::ComparatorEq() : CommonComparator() {
}

ComparatorEq::ComparatorEq(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorEq::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorEq::CompareFunction() {
    return Input->ReadValue() == ref_percent04;
}

TvElementType ComparatorEq::GetElementType() {
    return TvElementType::et_ComparatorEq;
}

ComparatorEq *ComparatorEq::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorEq:
            return static_cast<ComparatorEq *>(common_comparator);

        default:
            return NULL;
    }
}