#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class horse : public formID_extra_data<'XHOR', extra_data_type::horse> { // usually appears on ACHR, not other REFRs
      // The form should be an ACHR.
   };
}