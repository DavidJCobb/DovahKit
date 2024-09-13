#include "Topic.h"
#include "_common_cpp.h"
#include "../form_stub_addenda.h"
#include "../data/dialogue/topic_subtype.h"

namespace dovah::loaded_forms {
   void Topic::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      // DIAL records coalesce all data except for their full-name data; the functions 
      // to clear form-specific data are no-ops, but the function to clear all form 
      // component data still runs and empties out the TESFullName.
      //
      bool loaded_snam = false;
      form_reference_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'TIFC':
               //
               // Used for the equivalent of a std::vector::reserve call on the topic's info list.
               //
               break;
            case 'DATA':
               if (subrecord.is_in_bounds(4)) {
                  subrecord.unchecked_read(this->data.flags);
                  subrecord.unchecked_read(this->data.category);
                  subrecord.unchecked_read(this->data.subtype);
               }
               break;
            case 'FULL':
               if (!intfc.is_winning_record)
                  break;
               subrecord.read(this->text);
               break;
            case 'PNAM':
               if (subrecord.read(this->priority)) {
                  this->priority = std::clamp(this->priority, 0.0F, 100.0F);
               }
               break;
            case 'XIDX':
               break;
            case 'BNAM':
               if (subrecord.read(this->owning_forms.branch))
                  intfc.warn_if_ref_is_wrong_type(this->owning_forms.branch, form_type::dialogue_branch, subrecord.signature());
               break;
            case 'QNAM':
               if (subrecord.read(this->owning_forms.quest))
                  intfc.warn_if_ref_is_wrong_type(this->owning_forms.quest, form_type::quest, subrecord.signature());
               break;
            case 'SNAM':
               if (subrecord.read_signature(this->subtype)) {
                  loaded_snam = true;
                  auto idx = dialogue::topic_subtype_signature_to_index(this->subtype);
                  if (idx != (size_t)-1) {
                     this->data.category = dialogue::all_topic_subtypes[idx].category;
                     this->data.subtype  = idx;
                  }
               }
               break;
            case 'OBND':
               //
               // The loader checks for this and passes it to a virtual function on TESForm 
               // that's responsible for loading it. However, this form doesn't derive from 
               // TESBoundObject, so the TESForm implementation of that virtual function (a 
               // no-op) isn't overridden and therefore the data is not retained in memory.
               //
               break;
            case 'VMAD':
               if (!intfc.is_winning_record)
                  //
                  // TODO: How the hell do we handle coalescing for Papyrus data?! Whatever 
                  // we do, make sure to upload the use info builder, too.
                  //
                  break;
               this->script_data.load(subrecord, intfc);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (!loaded_snam) {
         //
         // Hope and pray this form wasn't made pre-Dragonborn lol.
         //
         const auto& list = dialogue::all_topic_subtypes;
         if (this->data.subtype < list.size())
            this->subtype = list[this->data.subtype].signature;
      }
   }
   /*static*/ void Topic::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      //
      // Because DIAL coalesces data across all records, we need to be careful in how we 
      // manage its use info.
      //
      constexpr int stored_qnam = 0;
      constexpr int stored_bnam = 1;
      //
      form_id_t formID;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case 'VMAD':
               if (uib.is_final_file())
                  //
                  // TODO: How the hell do we handle coalescing for Papyrus data?! Whatever 
                  // we do, make sure to upload the loader, too.
                  //
                  components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'BNAM':
               if (subrecord.read(formID))
                  uib.extra_form_ids[stored_bnam] = formID;
               break;
            case 'QNAM':
               if (subrecord.read(formID))
                  uib.extra_form_ids[stored_qnam] = formID;
               break;
            case 'TIFC':
            case 'FULL':
            case 'XIDX':
            case 'DATA':
            case 'SNAM':
            case 'OBND':
               break;
         }
      }
      if (uib.is_final_file()) {
         if (auto formID = uib.extra_form_ids[stored_bnam])
            uib.add_outbound_reference(formID, use_info_entry::flag::dialogue_branch);
         if (auto formID = uib.extra_form_ids[stored_qnam])
            uib.add_outbound_reference(formID, use_info_entry::flag::dialogue_quest);
      }
   }
   /*virtual*/ void Topic::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Topic*)out;
      
      bool committing_to_self = (&out->stub == &this->stub) && this->is_working_copy;
      
      copy->owning_forms.branch.set(*copy, this->owning_forms.branch);
      copy->owning_forms.quest.set(*copy, this->owning_forms.quest);
      copy->text = this->text;
      copy->data.flags        = this->data.flags;
      copy->data.category = this->data.category;
      copy->data.subtype      = this->data.subtype;
      copy->priority = this->priority;
      copy->subtype  = this->subtype;
      copy->script_data.clone_from(this->script_data, *copy);
   }
   /*virtual*/ void Topic::_save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      auto& FULL = record.open_next_subrecord('FULL');
      FULL.write(this->text);
      FULL.close();
      auto& PNAM = record.open_next_subrecord('PNAM');
      PNAM.write(this->priority);
      PNAM.close();
      record.write_formID_subrecord('BNAM', this->owning_forms.branch);
      record.write_formID_subrecord('QNAM', this->owning_forms.quest);
      auto& DATA = record.open_next_subrecord('DATA');
      DATA.write(this->data.flags);
      DATA.write(this->data.category);
      DATA.write(this->data.subtype);
      DATA.close();
      auto& SNAM = record.open_next_subrecord('SNAM');
      SNAM.write_signature(this->subtype);
      SNAM.close();
      //
      auto& TIFC = record.open_next_subrecord('TIFC');
      uint32_t count = 0;
      if (auto* addenda = this->stub.addenda)
         count = addenda->ordered_children.size();
      TIFC.write(count);
      TIFC.close();
   }
   /*virtual*/ void Topic::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->owning_forms.branch.clear_if(*this, other);
      this->owning_forms.quest.clear_if(*this, other);
      //
      this->script_data.sever_outbound_references_to(other, *this);
   }
   /*virtual*/ void Topic::_clear_impl() noexcept {
      this->owning_forms.branch.set(*this, nullptr);
      this->owning_forms.quest.set(*this, nullptr);
      this->data.flags        = 0;
      this->data.category = category::topic;
      this->data.subtype      = 3;
      this->text.reset();
      this->subtype = 0;
      this->script_data.clear(*this);
   }
}