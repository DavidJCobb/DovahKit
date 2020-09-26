#pragma once
#include "_templates.h"

namespace dovah::loaded_forms::components::extra {
   class merchant_container : public formID_extra_data<'XMRC', extra_data_type::merchant_container> {
      //
      // The form should be a REFR.
      //
      // This type is deprecated. The game's CELL and REFR loaders check for it and call into the 
      // extra-data loader, but the extra-data loader doesn't check for it and would therefore 
      // skip it. There also aren't any internal BSExtraData classes that appear to map to this 
      // (e.g. a hypothetical ExtraMerchantContainer).
      //
   };
}