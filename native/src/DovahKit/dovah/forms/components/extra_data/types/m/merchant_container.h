#pragma once
#include "../common_form.h"

namespace dovah::loaded_forms::components::extra_data_types {
   //
   // This type is deprecated. The game's CELL and REFR loaders check for it and call into the 
   // extra-data loader, but the extra-data loader doesn't check for it and would therefore 
   // skip it. There also aren't any internal BSExtraData classes that appear to map to this 
   // (e.g. a hypothetical ExtraMerchantContainer).
   //
   class merchant_container : public common_form<merchant_container, 'XMRC', form_type::reference> {
      public:
         static void generate_use_info(tes_file_reading::record& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state);
   };
}