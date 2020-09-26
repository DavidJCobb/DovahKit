#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class emittance_source : public formID_extra_data<'XEMI', extra_data_type::emittance_source> {
      // The form should be a LIGH or REGN.
   };
}