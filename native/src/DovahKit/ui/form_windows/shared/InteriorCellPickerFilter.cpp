#include "./InteriorCellPickerFilter.h"
#include "dovah/form_stub.h"

/*virtual*/ bool InteriorCellPickerFilter::form_matches(dovah::form_stub& stub) const noexcept /*override*/ {
   return !stub.is_exterior_cell();
}