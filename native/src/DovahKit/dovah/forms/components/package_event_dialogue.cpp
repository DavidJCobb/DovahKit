#include "package_event_dialogue.h"
#include "../_common_cpp.h"
#include "../../notice_code_list.h"

namespace dovah::loaded_forms::components {
   bool package_event_dialogue::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      //
      // The game reads subrecords indefinitely, stopping only after reading TNAM or 
      // PDTO. It will simply skip any unrecognized subrecords up to that point.
      //
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'INAM':
               subrecord.read(this->idle);
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::idle, intfc.target_stub, this->idle)
               );
               break;
            case 'TNAM':
               this->type = topic_type::ref;
               subrecord.read(this->topic);
               intfc.log_load_warning(
                  detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::topic, intfc.target_stub, this->topic)
               );
               return true; // TESPackage::Data::Load aborts after reading TNAM
            case 'PDTO':
               {
                  subrecord.read(this->type);
                  if (this->type == topic_type::ref) {
                     subrecord.read(this->topic);
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::topic, intfc.target_stub, this->topic)
                     );
                  } else if (this->type == topic_type::subtype)
                     subrecord.read(this->topic_subtype);
               }
               return true; // TESPackage::Data::Load aborts after reading PDTO
            default:
               {
                  detailed_notice warning;
                  warning.code = notice_code::package_event_dialogue_unrecognized_subrecord;
                  warning.set_cause_form(intfc.target_stub);
                  warning.set_cause_subrecord(subrecord.signature());
                  //
                  intfc.log_load_warning(warning);
               }
               break;
         }
      }
      return false;
   }
   /*static*/ void package_event_dialogue::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      form_id_t  formID;
      topic_type type;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'INAM':
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               break;
            case 'TNAM':
               if (subrecord.read(formID))
                  uib.add_outbound_reference(formID);
               return; // TESPackage::Data::Load aborts after reading TNAM
            case 'PDTO':
               {
                  subrecord.read(type);
                  if (type == topic_type::ref)
                     if (subrecord.read(formID))
                        uib.add_outbound_reference(formID);
               }
               return; // TESPackage::Data::Load aborts after reading PDTO
         }
      }
   }
   void package_event_dialogue::save(tes_record_writer& record) {
      if (this->idle)
         record.write_formID_subrecord('INAM', this->idle);
      auto& PDTO = record.open_next_subrecord('PDTO');
      PDTO.write(this->type);
      switch (this->type) {
         case topic_type::ref:
            PDTO.write(this->topic);
            break;
         case topic_type::subtype:
            PDTO.write(this->topic_subtype);
            break;
      }
      PDTO.close();
   }
   void package_event_dialogue::clone_from(const package_event_dialogue& other, form_stub& my_owner) noexcept {
      this->idle.set(my_owner, other.idle);
      this->type = other.type;
      this->topic.set(my_owner, other.topic);
      this->topic_subtype = other.topic_subtype;
   }
   void package_event_dialogue::sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept {
      this->idle.clear_if(my_owner, target);
      this->topic.clear_if(my_owner, target);
   }
   void package_event_dialogue::clear(form_stub& my_owner) {
      this->idle.set(my_owner, nullptr);
      this->topic.set(my_owner, nullptr);
      this->topic_subtype = 0;
   }

   bool package_event_dialogue::empty() const noexcept {
      if (this->idle)
         return false;
      if (this->topic || this->topic_subtype)
         return false;
      return true;
   }
}