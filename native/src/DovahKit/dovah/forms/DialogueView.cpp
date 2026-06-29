#include "DialogueView.h"
#include "_common_cpp.h"

namespace {
   //
   // TNAM[] is coalesced across all loaded files, but Bethesda writes all loaded 
   // entries into a file when saving. So, DLVW/TNAM would just keep piling up with 
   // more and more duplicated entries from the loaded masters, the more times you 
   // save and reload a file.
   // 
   // Setting this variable to `false` will disable coalescing for this subrecord, 
   // so we can ensure sane behavior if it is present.
   //
   constexpr const bool coalesce_topic_lists_as_per_bethesda = false;
}

namespace dovah::loaded_forms {
   void DialogueView::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      
      if (!intfc.is_winning_record) {
         if constexpr (coalesce_topic_lists_as_per_bethesda) {
            while (auto& subrecord = record.next_subrecord()) {
               if (subrecord.signature() == 'TNAM') {
                  if (auto& form = this->topics.emplace_back(); subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::topic, subrecord.signature());
               }
            }
         }
         return;
      }
      
      form_reference_t form_id;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'QNAM':
               if (auto& form = this->quest; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::quest, subrecord.signature());
               break;
            case 'BNAM':
               if (auto& form = this->branches.emplace_back(); subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::dialogue_branch, subrecord.signature());
               break;
            case 'TNAM':
               if (auto& form = this->topics.emplace_back(); subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::topic, subrecord.signature());
               break;
            case 'ENAM':
               {
                  uint32_t value = 0;
                  subrecord.read(value);
                  this->category = (dialogue::category)value;
               }
               break;
            case 'DNAM':
               subrecord.read(this->show_all_text);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void DialogueView::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file()) {
         if constexpr (coalesce_topic_lists_as_per_bethesda) {
            while (auto& subrecord = record.next_subrecord()) {
               if (subrecord.signature() == 'TNAM') {
                  form_id_t id;
                  if (subrecord.read(id) && id)
                     uib.add_outbound_reference(id);
               }
            }
         }
         return;
      }
      
      form_id_t quest = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'QNAM':
               subrecord.read(quest);
               break;
            case 'BNAM':
            case 'TNAM':
               {
                  form_id_t id;
                  if (subrecord.read(id) && id)
                     uib.add_outbound_reference(id);
               }
               break;
         }
      }
      uib.add_outbound_reference(quest);
   }
   void DialogueView::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (DialogueView*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->quest.set(*copy, this->quest);
      copy_form_reference_list(*copy, copy->branches, this->branches);
      copy_form_reference_list(*copy, copy->topics,   this->topics);
      copy->category      = this->category;
      copy->show_all_text = this->show_all_text;
   }
   void DialogueView::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      record.write_formID_subrecord('QNAM', this->quest);
      for (auto& item : this->branches)
         record.write_formID_subrecord('BNAM', item, true);
      for (auto& item : this->topics)
         record.write_formID_subrecord('TNAM', item, true);
      {
         auto v = (uint32_t)this->category;
         auto& subrecord = record.open_next_subrecord('ENAM');
         subrecord.write(v);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->show_all_text);
         subrecord.close();
      }
   }
   void DialogueView::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->quest.set(*this, nullptr);
      clear_form_reference_list(this->branches, *this);
      clear_form_reference_list(this->topics, *this);
      this->category      = {};
      this->show_all_text = false;
   }
   void DialogueView::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      this->quest.clear_if(*this, other);
      remove_form_from_reference_list(this->branches, other, *this);
      remove_form_from_reference_list(this->topics,   other, *this);
   }
}