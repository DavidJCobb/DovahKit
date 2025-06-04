#include "IdleAnimation.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/idle/event_name_too_long.h"
#include "../notices/form_load_warnings/by_form_type/idle/filename_too_long.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::idle;
   }
}

namespace dovah::loaded_forms {
   void IdleAnimation::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      //
      if (!intfc.is_winning_record)
         return;
      //
      bool content_loaded = false;
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
            case 'OBND':
               break;
            case 'CTDA':
               this->conditions.read_next(subrecord.get_containing_record(), intfc);
               break;
            case 'DNAM':
               if (subrecord.read(this->filename)) {
                  auto size = this->filename.size();
                  if (size > max_filename_length) {
                     specific_load_warnings::filename_too_long notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'ENAM':
               if (subrecord.read(this->animation_event)) {
                  auto size = this->animation_event.size();
                  if (size > max_event_name_length) {
                     specific_load_warnings::event_name_too_long notice(
                        this->stub,
                        size
                     );
                     intfc.log_load_warning(notice);
                  }
               }
               break;
            case 'ANAM':
               if (auto& form = this->parent; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, std::array{ form_type::idle, form_type::action }, subrecord.signature());
               if (auto& form = this->previous_sibling; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::idle, subrecord.signature());
               break;
            case 'DATA':
               subrecord.read(this->data.loop_time_range.min);
               subrecord.read(this->data.loop_time_range.max);
               subrecord.read(this->data.flags);
               subrecord.read(this->data.anim_group_section);
               subrecord.read(this->data.replay_delay);
               break;
            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void IdleAnimation::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         //
         // There is no data in this form type that is coalesced across multiple files.
         //
         return;

      form_id_t parent;
      form_id_t sibling;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'VMAD':
               components::papyrus_attachment_data::generate_use_info(subrecord, uib);
               break;
            case 'CTDA':
               components::condition::generate_use_info(record, uib);
               break;
            case 'ANAM':
               subrecord.read(parent);
               subrecord.read(sibling);
               break;
         }
      }
      uib.add_outbound_reference(parent);
      uib.add_outbound_reference(sibling);
   }
   void IdleAnimation::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (IdleAnimation*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->animation_event = this->animation_event;
      copy->filename = this->filename;
      copy->data = this->data;

      copy->parent.set(*copy, this->parent);
      copy->previous_sibling.set(*copy, this->previous_sibling);
   }
   void IdleAnimation::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);
      record.write_string_subrecord('DNAM', this->filename);
      record.write_string_subrecord('ENAM', this->animation_event);
      {
         auto& subrecord = record.open_next_subrecord('ANAM');
         subrecord.write(this->parent);
         subrecord.write(this->previous_sibling);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data.loop_time_range.min);
         subrecord.write(this->data.loop_time_range.max);
         subrecord.write(this->data.flags);
         subrecord.write(this->data.anim_group_section);
         subrecord.write(this->data.replay_delay);
         subrecord.close();
      }
   }
   void IdleAnimation::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);

      this->animation_event.clear();
      this->filename.clear();

      this->data = {};

      this->parent.set(*this, nullptr);
      this->previous_sibling.set(*this, nullptr);
   }
   void IdleAnimation::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      this->parent.clear_if(*this, other);
      this->previous_sibling.clear_if(*this, other);
   }
}