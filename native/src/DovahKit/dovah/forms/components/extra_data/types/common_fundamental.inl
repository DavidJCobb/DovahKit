#pragma once
#include "./common_fundamental.h"

#define TEMPLATE_PARAMS template<typename Self, uint32_t Signature, typename ValueType, ValueType DefaultValue>
#define CLASS_NAME common_fundamental<Self, Signature, ValueType, DefaultValue>

namespace dovah::loaded_forms::components::extra_data_types {
   TEMPLATE_PARAMS
   /*virtual*/ CLASS_NAME::subrecord_load_result CLASS_NAME::load(tes_file_reading::subrecord& subrecord, load_interface_t& intfc) /*override*/ {
      if (subrecord.signature() != signature)
         return subrecord_load_result::unrecognized;
      if (subrecord.read(this->value))
         return subrecord_load_result::succeeded;
      return subrecord_load_result::failed;
   }

   TEMPLATE_PARAMS
   /*virtual*/ CLASS_NAME::record_load_result CLASS_NAME::load(tes_file_reading::record& record, load_interface_t& intfc) /*override*/ {
      return record_load_result::complete;
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::save(tes_file_writing::record& record, save_interface_t& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(signature);
      subrecord.write(this->value);
      subrecord.close();
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::clear_contained_formIDs(loaded_forms::Form& my_owner) /*override*/ {
   }

   TEMPLATE_PARAMS
   /*virtual*/ void CLASS_NAME::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) /*override*/ {
   }

   TEMPLATE_PARAMS
   /*virtual*/ extra_data* CLASS_NAME::clone(loaded_forms::Form& clone_owner) const noexcept /*override*/ {
      auto* copy = new Self;
      copy->value = this->value;
      return copy;
   }
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
