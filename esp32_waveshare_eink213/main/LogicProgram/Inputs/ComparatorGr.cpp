#include "LogicProgram/Inputs/ComparatorGr.h"
#include "icons/cmp_greater.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGr::ComparatorGr() : CommonComparator() {
}

ComparatorGr::ComparatorGr(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorGr::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorGr::CompareFunction() {
    return Input->ReadValue() > ref_percent04;
}

TvElementType ComparatorGr::GetElementType() {
    return TvElementType::et_ComparatorGr;
}

ComparatorGr *ComparatorGr::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorGr:
            return static_cast<ComparatorGr *>(common_comparator);

        default:
            return NULL;
    }
}