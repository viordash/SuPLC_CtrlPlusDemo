#include "LogicProgram/Inputs/ComparatorLE.h"
#include "icons/cmp_less_or_equal.h"
#include "os.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

ComparatorLE::ComparatorLE() : CommonComparator() {
}

ComparatorLE::ComparatorLE(uint8_t ref_percent04, const MapIO io_adr)
    : CommonComparator(ref_percent04, io_adr) {
}

const Bitmap *ComparatorLE::GetCurrentBitmap(LogicItemState state) {
    switch (state) {
        case LogicItemState::lisActive:
            return &icon_active;
        default:
            return &icon_passive;
    }
}

bool ComparatorLE::CompareFunction() {
    return Input->ReadValue() <= ref_percent04;
}

TvElementType ComparatorLE::GetElementType() {
    return TvElementType::et_ComparatorLE;
}

ComparatorLE *ComparatorLE::TryToCast(CommonComparator *common_comparator) {
    switch (common_comparator->GetElementType()) {
        case TvElementType::et_ComparatorLE:
            return static_cast<ComparatorLE *>(common_comparator);

        default:
            return NULL;
    }
}