#include "Perk.h"
#include "_common_cpp.h"

#include "../notices/form_load_warnings/by_form_type/perk/no_ranks.h"
#include "../notices/form_load_warnings/by_form_type/perk/orphaned_effect_subrecord.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::perk;
   }
}

namespace dovah::loaded_forms {
   void Perk::load(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      Form::load(record, intfc);
      if (!intfc.is_winning_record)
         return;

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

            case 'ICON':
               subrecord.read(this->icon);
               break;
            case 'CTDA':
               this->conditions.read_next(record, intfc);
               break;

            case 'DATA':
               subrecord.read(this->data.is_trait);
               subrecord.read(this->data.level);
               subrecord.read(this->data.rank_count);
               subrecord.read(this->data.playable);
               subrecord.read(this->data.hidden);
               if (this->data.rank_count == 0) {
                  specific_load_warnings::no_ranks notice(intfc.target_stub);
                  intfc.log_load_warning(notice);
               }
               break;
            case 'NNAM':
               if (auto& form = this->next_perk; subrecord.read(form))
                  intfc.warn_if_ref_is_wrong_type(form, form_type::perk, subrecord.signature());
               break;

            case 'PRKE': // PRKE+DATA+...+PRKF
               {
                  auto&  list  = this->effects;
                  size_t which = list.size();
                  list.emplace_back().load(record, intfc, which);
               }
               break;

            case structs::perk_effect::subrecord_end:
            case structs::perk_effect::subrecord_conditions:
            case structs::perk_entry_point_data::subrecord_function_type:
            case structs::perk_entry_point_data::subrecord_function_data_1:
            case structs::perk_entry_point_data::subrecord_function_data_2:
            case structs::perk_entry_point_data::subrecord_function_data_3:
               {
                  specific_load_warnings::orphaned_effect_subrecord notice(
                     intfc.target_stub,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;

            default:
               intfc.warn_on_unrecognized_subrecord(subrecord);
               break;
         }
      }
   }
   /*static*/ void Perk::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      if (!uib.is_final_file())
         return;
      
      form_id_t next;

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
            case 'NNAM':
               subrecord.read(next);
               break;

            case structs::perk_effect::subrecord_start:
               structs::perk_effect::generate_use_info(record, uib);
               break;
         }
      }
      uib.add_outbound_reference(next);
   }
   void Perk::_clone_impl(Form* out) const noexcept {
      assert(out->type == form_type);
      auto copy = (Perk*)out;
      
      copy->script_data.clone_from(this->script_data, *copy);
      copy->conditions.clear(*copy);
      copy->conditions.append_all_of(*copy, this->conditions);

      copy->name = this->name;
      copy->description = this->description;

      copy->data = this->data;
      copy->icon = this->icon;
      copy->next_perk.set(*copy, this->next_perk);
      {
         auto& src_list = this->effects;
         auto& dst_list = copy->effects;
         for (auto& dst_item : dst_list)
            dst_item.clear(*copy);
         dst_list.clear();

         size_t size = src_list.size();
         dst_list.resize(size);
         for (size_t i = 0; i < size; ++i) {
            auto& src_item = src_list[i];
            auto& dst_item = dst_list[i];
            dst_item.clone_from(src_item, *copy);
         }
      }
   }
   void Perk::_save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      this->script_data.save(record, intfc);
      {
         auto& subrecord = record.open_next_subrecord('FULL');
         subrecord.write(this->name);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('DESC');
         subrecord.write(this->description);
         subrecord.close();
      }
      {
         auto& subrecord = record.open_next_subrecord('ICON');
         subrecord.write(this->icon);
         subrecord.close();
      }
      for (auto& cnd : this->conditions)
         cnd.save(record, intfc);

      {
         auto& subrecord = record.open_next_subrecord('DATA');
         subrecord.write(this->data.is_trait);
         subrecord.write(this->data.level);
         subrecord.write(this->data.rank_count);
         subrecord.write(this->data.playable);
         subrecord.write(this->data.hidden);
         subrecord.close();
      }
      record.write_formID_subrecord('NNAM', this->next_perk, true);
      for (size_t i = 0; i < this->effects.size(); ++i) {
         this->effects[i].save(record, intfc, i);
      }
   }
   void Perk::_clear_impl() noexcept {
      this->script_data.clear(*this);
      this->conditions.clear(*this);
      
      this->name.reset();
      this->description.reset();

      this->data = {};
      this->icon = {};
      this->next_perk.set(*this, nullptr);

      for (auto& item : this->effects)
         item.clear(*this);
      this->effects.clear();
   }
   void Perk::_sever_outbound_references_impl(form_stub& other) noexcept {
      this->script_data.sever_outbound_references_to(other, *this);
      for (auto& cnd : this->conditions)
         cnd.sever_outbound_references_to(other, *this);

      this->next_perk.clear_if(*this, other);

      for (auto& item : this->effects)
         item.sever_outbound_references_to(other, *this);
   }
}