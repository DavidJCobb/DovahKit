#include "./collision_data.h"

namespace {
   using extra_data = dovah::loaded_forms::components::extra_data_types::extra_data;
   using subrecord_load_result = extra_data::subrecord_load_result;
   using record_load_result = extra_data::record_load_result;
}

#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/collision_layer_insensible_uid.h"
#include "../../../../../notices/form_load_warnings/by_form_component/extra_data/collision_layer_invalid_uid.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::extra_data;
   }
}

namespace dovah::loaded_forms::components::extra_data_types {
   /*virtual*/ subrecord_load_result collision_data::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      auto result = common_fundamental::load(subrecord, intfc);
      if (result == subrecord_load_result::succeeded) {
         if (this->value > max_functional_collision_layer_uid) {
            specific_load_warnings::collision_layer_invalid_uid notice(intfc.target_stub, this->value);
            intfc.log_load_warning(notice);
         }
         switch ((collision_layer)this->value) {
            case collision_layer::custom_pick_1:
            case collision_layer::custom_pick_2:
            case collision_layer::spell_explosion:
               specific_load_warnings::collision_layer_insensible_uid notice(intfc.target_stub, this->value);
               intfc.log_load_warning(notice);
               break;
         }
      }
      return result;
   }
}