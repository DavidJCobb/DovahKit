#include "package_event_dialogue.h"
#include "../_common_cpp.h"
#include "./legacy_script.h"

#include "../../notices/form_load_warnings/by_form_component/package_event_dialogue/unrecognized_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_component::package_event_dialogue;
   }
}

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
               intfc.warn_if_ref_is_wrong_type(this->idle, form_type::idle, subrecord.signature());
               break;
            case components::legacy_script::subrecord_signature_header:
            case components::legacy_script::subrecord_signature_compiled_data:
            case components::legacy_script::subrecord_signature_source_code:
            case components::legacy_script::subrecord_signature_quest:
            case components::legacy_script::subrecord_signature_ref_objects:
            case components::legacy_script::subrecord_signature_ref_variables:
               // TODO: load and retain legacy script data, in the future, if we ever care about that.
               // NOTE: per xEdit, only SCHR and maybe SCTX have been seen. Does the CK or game load the others?
               break;
            case 'TNAM':
               this->type = topic_type::ref;
               subrecord.read(this->topic);
               intfc.warn_if_ref_is_wrong_type(this->topic, form_type::topic, subrecord.signature());
               return true; // TESPackage::Data::Load aborts after reading TNAM
            case 'PDTO':
               {
                  subrecord.read(this->type);
                  if (this->type == topic_type::ref) {
                     subrecord.read(this->topic);
                     intfc.warn_if_ref_is_wrong_type(this->topic, form_type::topic, subrecord.signature());
                  } else if (this->type == topic_type::subtype)
                     subrecord.read(this->topic_subtype);
               }
               return true; // TESPackage::Data::Load aborts after reading PDTO
            default:
               {
                  specific_load_warnings::unrecognized_subrecord notice(
                     const_cast<form_stub&>(intfc.target_stub),
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
         }
      }
      return false;
   }
   void package_event_dialogue::save(tes_record_writer& record) {
      record.write_formID_subrecord('INAM', this->idle, false);
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
   void package_event_dialogue::clone_from(const package_event_dialogue& other, loaded_forms::Form& my_owner) noexcept {
      this->idle.set(my_owner, other.idle);
      this->type = other.type;
      this->topic.set(my_owner, other.topic);
      this->topic_subtype = other.topic_subtype;
   }
   void package_event_dialogue::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->idle.clear_if(my_owner, target);
      this->topic.clear_if(my_owner, target);
   }
   void package_event_dialogue::clear(loaded_forms::Form& my_owner) {
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

   #pragma region package_event_dialogue::use_info_state
   void package_event_dialogue::use_info_state::generate_use_info(tes_record_reader& record) {
      topic_type type;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'INAM':
               subrecord.read(this->idle);
               break;
            case 'TNAM':
               subrecord.read(this->topic);
               return; // TESPackage::Data::Load aborts after reading TNAM
            case 'PDTO':
               subrecord.read(type);
               if (subrecord.read(type) && type == topic_type::ref)
                  subrecord.read(this->topic);
               return; // TESPackage::Data::Load aborts after reading PDTO
         }
      }
   }
   void package_event_dialogue::use_info_state::clear() {
      this->idle  = 0;
      this->topic = 0;
   }
   void package_event_dialogue::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->idle)
         uib.add_outbound_reference(this->idle);
      if (this->topic)
         uib.add_outbound_reference(this->topic);
   }
   #pragma endregion
}