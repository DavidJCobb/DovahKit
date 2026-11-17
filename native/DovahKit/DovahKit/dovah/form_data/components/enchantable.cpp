#include "./enchantable.h"
#include <cassert>
#include "dovah/load_order_interfaces/form_load.h"
#include "dovah/load_order_processes/use_info_build/use_info_builder.h"
#include "dovah/tes/reading/subrecord_view.h"

namespace dovah::form_data::serialization {
   #define COMPONENT_TYPENAME components::enchantable

   #pragma region pending_uses
      #define CLASSNAME pending_uses<COMPONENT_TYPENAME>

      void CLASSNAME::read(tes::subrecord_view& subrecord) {
         using self_type = COMPONENT_TYPENAME<managed_form_params>;
         switch (subrecord.signature()) {
            case self_type::subrecord_signature_effect:
            case self_type::subrecord_signature_effect_legacy:
               subrecord.read(this->effect);
               break;
         }
      }

      void CLASSNAME::commit_to(load_order_processes::use_info_build::use_info_builder& uib) {
         uib.add_outbound_form_use(this->effect);
      }
   #pragma endregion

   extern void load(COMPONENT_TYPENAME<managed_form_params>& self, tes::subrecord_view& subrecord, load_order_interfaces::form_load& intfc) {
      using self_type = COMPONENT_TYPENAME<managed_form_params>;

      const auto signature = subrecord.signature();
      #if _DEBUG
         switch (signature) {
            case self_type::subrecord_signature_effect:
            case self_type::subrecord_signature_effect_legacy:
            case self_type::subrecord_signature_charge:
            case self_type::subrecord_signature_charge_legacy:
               break;
            default: // invalid
               assert(false && "Why was the loader for `enchantable` called on a subrecord it's not built to handle?");
         }
      #endif

      if (signature == self_type::subrecord_signature_effect || signature == self_type::subrecord_signature_effect_legacy) {
         if (auto& dst = self.effect; subrecord.read(dst)) {
            intfc.warn_if_use_is_wrong_type(dst, std::array{ form_type::enchantment, form_type::spell }, subrecord);
         }
         return;
      }
      if (signature == self_type::subrecord_signature_charge || signature == self_type::subrecord_signature_charge_legacy) {
         subrecord.read(self.charge);
         return;
      }
   }
}