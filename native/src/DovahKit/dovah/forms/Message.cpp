#include "Message.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/message/orphaned_conditions.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::message;
   }
}

namespace {
   constexpr const bool retain_orphaned_conditions = false;
}

namespace dovah::loaded_forms {
   void Message::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

      size_t orphaned_condition_count = 0;
      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'EDID': // already read by the FormStub
               break;
            case 'VMAD':
               this->script_data.load(subrecord, intfc);
               break;
            case 'FULL':
               subrecord.read(this->name);
               break;
            case 'DESC':
               subrecord.read(this->description);
               break;
            case 'INAM':
               if (auto& form = this->icon; subrecord.read(icon))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::menu_icon, subrecord.signature());
               break;
            case 'QNAM':
               if (auto& form = this->owning_quest; subrecord.read(icon))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::quest, subrecord.signature());
               break;
            case 'DNAM':
               subrecord.read(this->flags);
               break;
            case 'TNAM':
               subrecord.read(this->display_time);
               break;
            case 'ITXT':
               {
                  auto& b = this->buttons.emplace_back();
                  subrecord.read(b.text);
               }
               break;
            case 'CTDA':
               if (this->buttons.empty()) {
                  ++orphaned_condition_count;
                  if constexpr (retain_orphaned_conditions) {
                     this->buttons.emplace_back();
                  } else {
                     break;
                  }
               }
               this->buttons.back().conditions.read_next(record, intfc);
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
      if (orphaned_condition_count > 0) {
         specific_load_warnings::orphaned_conditions notice(
            this->stub,
            orphaned_condition_count,
            retain_orphaned_conditions
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void Message::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      bool has_any_buttons = false;
      form_id_t icon;
      form_id_t quest;

      while (auto& subrecord = record.next_subrecord()) {
         if (Form::subrecord_is_handled_elsewhere(subrecord.signature()))
            continue;
         switch (subrecord.signature()) {
            case 'INAM':
               subrecord.read(icon);
               break;
            case 'QNAM':
               subrecord.read(quest);
               break;
            case 'ITXT':
               has_any_buttons = true;
               break;
            case 'CTDA':
               if constexpr (retain_orphaned_conditions) {
                  if (!has_any_buttons)
                     break;
               }
               components::condition::generate_use_info(record, uib);
               break;
         }
      }
      uib.add_outbound_reference(icon);
      uib.add_outbound_reference(quest);
   }
   void Message::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Message*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);

      copy->name = this->name;
      copy->description = this->description;

      copy->display_time = this->display_time;
      copy->flags = this->flags;
      copy->icon.set(*copy, this->icon);
      copy->owning_quest.set(*copy, this->owning_quest);
      {
         auto& src_list = this->buttons;
         auto& dst_list = copy->buttons;
         for (auto& dst : dst_list) {
            dst.conditions.clear(*copy);
            dst.text.reset();
         }
         dst_list.clear();

         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src = src_list[i];
            auto& dst = dst_list[i];
            dst.conditions.append_all_of(*copy, src.conditions);
            dst.text = src.text;
         }
      }
   }
   void Message::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      record.write_formID_subrecord('INAM', this->icon, true);
      record.write_formID_subrecord('QNAM', this->owning_quest, true);
      {
         auto& subrecord = record.open_next_subrecord('DNAM');
         subrecord.write(this->flags);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('TNAM');
         subrecord.write(this->display_time);
         subrecord.close();
      }
      for (auto& b : this->buttons) {
         auto& subrecord = record.open_next_subrecord('ITXT');
         subrecord.write(b.text);
         subrecord.close();
         for (auto& cnd : b.conditions)
            cnd.save(record, intfc);
      }
   }
   void Message::_clear_impl() noexcept {
      this->script_data.clear(*this);
      
      this->name.reset();
      this->description.reset();

      this->flags = 0;
      this->display_time = 0;
      this->icon.set(*this, nullptr);
      this->owning_quest.set(*this, nullptr);
      for (auto& b : this->buttons) {
         b.conditions.clear(*this);
         b.text.reset();
      }
   }
   void Message::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);

      this->icon.clear_if(*this, other);
      this->owning_quest.clear_if(*this, other);
      for (auto& b : this->buttons) {
         for (auto& cnd : b.conditions)
            cnd.sever_outbound_references_to(other, *this);
      }
   }
}