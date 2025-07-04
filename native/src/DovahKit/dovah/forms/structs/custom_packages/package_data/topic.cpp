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
            {
               auto& form = this->data.emplace<form_reference_t>();
               if (subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::topic, subrecord);
            }
            break;
         case subrecord_modern:
            {
               serialized_type type = serialized_type::topic_form;
               subrecord.read(type);
               switch (type) {
                  case serialized_type::topic_form:
                     {
                        auto& form = this->data.emplace<form_reference_t>();
                        if (subrecord.read(form))
                           intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::topic, subrecord);
                     }
                     break;
                  case serialized_type::topic_subtype:
                     subrecord.read(this->data.emplace<uint32_t>());
                     break;
               }
            }
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
      form_id_t topic;

      auto& subrecord = record.get_current_subrecord();
      if (subrecord.signature() == subrecord_legacy) {
         subrecord.read(topic);
      } else if (subrecord.signature() == subrecord_modern) {
         serialized_type type = serialized_type::topic_form;
         subrecord.read(type);
         switch (type) {
            case serialized_type::topic_form:
               subrecord.read(topic);
               break;
         }
      } else {
         return;
      }
      if (topic)
         uib.add_outbound_reference(topic);
      record.next_subrecord();
   };
   /*virtual*/ void package_data_topic::save_value(tes_record_writer& record, load_order_interfaces::form_save& intfc) /*override*/ {
      auto& subrecord = record.open_next_subrecord(subrecord_modern);
      if (std::holds_alternative<form_reference_t>(this->data)) {
         subrecord.write(serialized_type::topic_form);
         subrecord.write(std::get<form_reference_t>(this->data));
      } else {
         subrecord.write(serialized_type::topic_subtype);
         subrecord.write(std::get<uint32_t>(this->data));
      }
      subrecord.close();
   }
   /*virtual*/ package_data* package_data_topic::clone(loaded_forms::Form& owner_of_clone) const noexcept /*override*/ {
      auto  copy_ptr = std::make_unique<package_data_topic>();
      auto* copy = copy_ptr.get();

      if (std::holds_alternative<form_reference_t>(this->data)) {
         copy->data.emplace<form_reference_t>().set(owner_of_clone, std::get<form_reference_t>(this->data));
      } else {
         copy->data.emplace<uint32_t>() = std::get<uint32_t>(this->data);
      }

      return copy_ptr.release();
   }
   /*virtual*/ void package_data_topic::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept /*override*/ {
      if (std::holds_alternative<form_reference_t>(this->data)) {
         std::get<form_reference_t>(this->data).clear_if(my_owner, other);
      }
   }
   /*virtual*/ void package_data_topic::clear(loaded_forms::Form& my_owner) /*override*/ {
      if (std::holds_alternative<form_reference_t>(this->data)) {
         std::get<form_reference_t>(this->data).set(my_owner, nullptr);
      }
      this->data.emplace<uint32_t>() = 0;
   }
}