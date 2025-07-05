#include "./package_event_addon.h"
#include "../_common_cpp.h"

namespace dovah::loaded_forms::structs {
   void package_event_addon::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      do {
         auto& subrecord = record.next_subrecord();
         bool  exit      = false;
         switch (subrecord.signature()) {
            case 'INAM':
               if (auto& form = this->idle; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, dovah::form_type::idle, subrecord);
               break;
            case package_topic::subrecord_modern: // the legacy subrecord is not supported here
               this->topic.load(subrecord, intfc);
               exit = true;
               break;
            case 'TNAM':
               exit = true;
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord); // TODO: Warning specific to this reader loop
               break;
         }
         if (exit)
            break;
      } while (true);
   }
   void package_event_addon::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      record.write_formID_subrecord('INAM', this->idle, false);
      this->topic.save(record, intfc);
   }

   void package_event_addon::clone_from(const package_event_addon& src, Form& my_owner) noexcept {
      this->idle.set(my_owner, src.idle);
      this->topic.clone_from(src.topic, my_owner);
   }
   void package_event_addon::clear(Form& my_owner) noexcept {
      this->idle.set(my_owner, nullptr);
      this->topic.clear(my_owner);
   }
   void package_event_addon::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      this->idle.clear_if(my_owner, other);
      this->topic.sever_outbound_references_to(other, my_owner);
   }

   #pragma region package_event_addon::use_info_state
   void package_event_addon::use_info_state::generate_use_info(tes_record_reader& record) {
      do {
         auto& subrecord = record.next_subrecord();
         bool  exit      = false;
         switch (subrecord.signature()) {
            case 'INAM':
               subrecord.read(this->idle);
               break;
            case package_topic::subrecord_modern:
               {
                  uint32_t type = 0;
                  subrecord.read(type);
                  if (type == 0)
                     subrecord.read(this->topic);
               }
               exit = true;
               break;
            case 'TNAM':
               exit = true;
               break;
         }
         if (exit)
            break;
      } while (true);
   }
   void package_event_addon::use_info_state::commit_to(form_stub_use_info_builder& uib) {
      if (this->idle)
         uib.add_outbound_reference(this->idle);
      if (this->topic)
         uib.add_outbound_reference(this->topic);
   }
   #pragma endregion
}