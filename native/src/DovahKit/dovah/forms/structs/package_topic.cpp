#include "./package_topic.h"
#include "../_common_cpp.h"

namespace {
   enum class serialized_type : uint32_t {
      topic_form,
      topic_subtype,
   };
}

namespace dovah::loaded_forms::structs {
   void package_topic::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc) {
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
      }
   }
   /*static*/ void package_topic::generate_use_info(tes_subrecord_reader& subrecord, form_stub_use_info_builder& uib) {
      form_id_t topic;
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
   };
   void package_topic::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
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
   void package_topic::clone_from(const package_topic& other, loaded_forms::Form& my_owner) noexcept {
      if (std::holds_alternative<form_reference_t>(other.data)) {
         this->data.emplace<form_reference_t>().set(my_owner, std::get<form_reference_t>(other.data));
      } else {
         this->data.emplace<uint32_t>() = std::get<uint32_t>(other.data);
      }
   }
   void package_topic::sever_outbound_references_to(form_stub& other, loaded_forms::Form& my_owner) noexcept {
      if (std::holds_alternative<form_reference_t>(this->data)) {
         std::get<form_reference_t>(this->data).clear_if(my_owner, other);
      }
   }
   void package_topic::clear(loaded_forms::Form& my_owner) {
      if (std::holds_alternative<form_reference_t>(this->data)) {
         std::get<form_reference_t>(this->data).set(my_owner, nullptr);
      }
      this->data.emplace<uint32_t>() = 0;
   }
}