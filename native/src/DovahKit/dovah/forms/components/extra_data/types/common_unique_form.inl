#pragma once
#include "./common_unique_form.h"
#include "../../../_common_cpp.h"

#define TEMPLATE_PARAMS template<typename Self, uint32_t Signature, auto Allowed, ::dovah::use_info::entry_flags::base_extra_data UseInfoEntryFlag> requires impl::form_type_or_array_thereof<Allowed>
#define CLASS_NAME common_unique_form<Self, Signature, Allowed, UseInfoEntryFlag>

namespace dovah::loaded_forms::components::extra_data_types {
   TEMPLATE_PARAMS
   /*virtual*/ CLASS_NAME::subrecord_load_result CLASS_NAME::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() == signature) {
         if (subrecord.read(this->form)) {
            if constexpr (!allowed_form_types.empty()) {
               intfc.warn_if_ref_is_wrong_type(this->form, allowed_form_types, subrecord.signature());
            }
            return subrecord_load_result::succeeded;
         }
         return subrecord_load_result::failed;
      }
      return subrecord_load_result::unrecognized;
   }

   TEMPLATE_PARAMS
   /*virtual*/ CLASS_NAME::record_load_result CLASS_NAME::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      record.write_formID_subrecord(signature, this->form);
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
      this->form.set(my_owner, nullptr);
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
      this->form.clear_if(my_owner, target);
   }

   TEMPLATE_PARAMS
   /*virtual*/ extra_data* CLASS_NAME::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* copy = new Self;
      copy->form.set(clone_owner, this->form);
      return copy;
   }
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME