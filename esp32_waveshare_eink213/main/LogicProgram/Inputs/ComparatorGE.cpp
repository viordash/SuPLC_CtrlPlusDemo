#include "LogicProgram/Inputs/ComparatorGE.h"
#include "icons/cmp_greater_or_equal.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorGE::ComparatorGE() : CommonComparator() {
}

ComparatorGE::ComparatorGE(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorGE::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorGE::CompareFunction() {
    return Input->ReadValue() >= ref_percent04;
}

TvElementType ComparatorGE::GetElementType() {
    return TvElementType::et_ComparatorGE;
}

ComparatorGE *ComparatorGE::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorGE:
            return static_cast<ComparatorGE *>(common_comparator);

        default:
            return NULL;
    }
}