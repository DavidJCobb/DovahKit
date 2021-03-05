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
      public:
         static void generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
            record.get_current_subrecord().read(state.by_name.merchant_container);
         }
   };
}