#include "LogicProgram/Inputs/ComparatorLs.h"
#include "icons/cmp_less.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLs::ComparatorLs() : CommonComparator() {
}

ComparatorLs::ComparatorLs(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorLs::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorLs::CompareFunction() {
    return Input->ReadValue() < ref_percent04;
}

TvElementType ComparatorLs::GetElementType() {
    return TvElementType::et_ComparatorLs;
}

ComparatorLs *ComparatorLs::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorLs:
            return static_cast<ComparatorLs *>(common_comparator);

        default:
            return NULL;
    }
}