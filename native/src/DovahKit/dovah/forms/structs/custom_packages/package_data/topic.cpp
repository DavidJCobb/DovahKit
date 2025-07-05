#include "./topic.h"
#include <memory>
#include "../../../_common_cpp.h"

#include "../../../../notices/form_load_warnings/by_form_type/package/package_data_unexpected_value_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::package;
   }
}

namespace {
   enum class serialized_type : uint32_t {
      topic_form,
      topic_subtype,
   };
}

namespace dovah::loaded_forms::structs::custom_packages {
   /*virtual*/ void package_data_topic::load_value(tes_record_reader& record, load_order_interfaces::form_load& intfc, const load_context& context) /*override*/ {
      auto& subrecord = record.get_current_subrecord();
      switch (subrecord.signature()) {
         case subrecord_legacy:
         case subrecord_modern:
            this->data.load(subrecord, intfc);
            break;
         default:
            specific_load_warnings::package_data_unexpected_value_subrecord notice(
               intfc.target_stub,
               context.which,
               this->get_type(),
               subrecord.signature()
            );
            intfc.log_load_warning(notice);
            return;
      }
      record.next_subrecord();
   }
   /*static*/ void package_data_topic::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      switch (record.get_current_subrecord().signature()) {
         case subrecord_legacy:
         case subrecord_modern:
            package_topic::generate_use_info(record.get_current_subrecord(), uib);
            break;
      }
   };
   /*virtual*/ void package_data_topic::save_value(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      this->data.save(record, intfc);
   }
   /*virtual*/ package_data* package_data_topic::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<package_data_topic>();
      auto* copy = copy_ptr.get();

      copy->data.clone_from(this->data, owner_of_clone);

      return copy_ptr.release();
   }
   /*virtual*/ void package_data_topic::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      this->data.sever_outbound_references_to(other, my_owner);
   }
   /*virtual*/ void package_data_topic::clear(loaded_forms::Form& my_owner) /*override*/ {
      this->data.clear(my_owner);
   }
}