#include "./perk_effect.h"
#include "../_common_cpp.h"

#include "../../notices/form_load_warnings/by_form_type/perk/effect_header_has_invalid_size.h"
#include "../../notices/form_load_warnings/by_form_type/perk/entry_point_condition_group_out_of_bounds.h"
#include "../../notices/form_load_warnings/by_form_type/perk/entry_point_data_for_effect_of_other_type.h"
#include "../../notices/form_load_warnings/by_form_type/perk/invalid_effect_type.h"
#include "../../notices/form_load_warnings/by_form_type/perk/invalid_entry_point.h"
#include "../../notices/form_load_warnings/by_form_type/perk/orphaned_entry_point_conditions.h"
#include "../../notices/form_load_warnings/by_form_type/perk/unterminated_perk_effect.h"
#include "../../notices/form_load_warnings/by_form_type/perk/wrong_condition_group_count_for_entry_point.h"
#include "../../notices/form_save_errors/by_form_type/perk/too_many_condition_groups_for_perk_entry.h"

#include "../../data/perk_entry_points.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::perk;
   }
   namespace specific_save_errors {
      using namespace dovah::notices::form_save_errors::by_type::perk;
   }
}

namespace dovah::loaded_forms::structs {
   perk_effect::type perk_effect::get_type() const {
      return (type)this->data.index();
   }
   void perk_effect::set_type(Form& my_owner, type t) {
      auto prior = this->get_type();
      if (prior == t)
         return;
      switch (prior) {
         case type::quest_and_stage:
            std::get<data_types::quest>(this->data).quest.set(my_owner, nullptr);
            break;
         case type::ability:
            std::get<data_types::ability>(this->data).spell.set(my_owner, nullptr);
            break;
         case type::entry_point:
            std::get<data_types::entry_point>(this->data).function.clear(my_owner);
            break;
      }
      switch (t) {
         case type::quest_and_stage:
            this->data.emplace<data_types::quest>();
            break;
         case type::ability:
            this->data.emplace<data_types::ability>();
            break;
         case type::entry_point:
            this->data.emplace<data_types::entry_point>();
            break;
      }
   }

   void perk_effect::load(tes_record_reader& record, load_order_interfaces::form_load& intfc, size_t which) {
      type t = (type)0;
      {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == subrecord_start);
         if (subrecord.size() != 3) {
            //
            // The game immediately aborts loading this form, returning false from the loader function.
            //
            specific_load_warnings::effect_header_has_invalid_size notice(
               intfc.target_stub,
               which,
               subrecord.size(),
               3
            );
            intfc.log_load_warning(notice);
         }
         subrecord.read(t);
         subrecord.read(this->rank);
         subrecord.read(this->priority);
      }
      bool valid_type = true;
      switch (t) {
         case type::quest_and_stage:
            this->data.emplace<data_types::quest>();
            break;
         case type::ability:
            this->data.emplace<data_types::ability>();
            break;
         case type::entry_point:
            this->data.emplace<data_types::entry_point>();
            break;
         default:
            valid_type = false;
            //
            specific_load_warnings::invalid_effect_type notice(
               intfc.target_stub,
               which,
               (uint8_t)t
            );
            intfc.log_load_warning(notice);
            //
            break;
      }
      bool    terminated = false;
      bool    in_a_condition_tab       = false;
      uint8_t perk_condition_tab_count = 0;
      uint8_t current_condition_tab    = 0;
      size_t  orphaned_condition_count = 0;
      while (auto& subrecord = record.next_subrecord()) {
         switch (subrecord.signature()) {
            case subrecord_end:
               terminated = true;
               break;
            case 'DATA':
               if (!valid_type) {
                  break;
               }
               switch (this->get_type()) {
                  case type::quest_and_stage:
                     {
                        auto& casted = std::get<data_types::quest>(this->data);
                        if (auto& form = casted.quest; subrecord.read(form))
                           intfc.warn_if_ref_is_wrong_type(form, form_type::quest, subrecord.signature());
                        subrecord.read(casted.stage);
                     }
                     break;
                  case type::ability:
                     {
                        auto& casted = std::get<data_types::ability>(this->data);
                        if (auto& form = casted.spell; subrecord.read(form))
                           intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
                     }
                     break;
                  case type::entry_point:
                     {
                        auto& casted = std::get<data_types::entry_point>(this->data);
                        subrecord.read(casted.entry);
                        subrecord.read(casted.function.function);
                        subrecord.read(perk_condition_tab_count);

                        if ((size_t)casted.entry >= all_perk_entry_points.size()) {
                           specific_load_warnings::invalid_entry_point notice(
                              intfc.target_stub,
                              which,
                              (size_t)casted.entry
                           );
                           intfc.log_load_warning(notice);
                        }
                     }
                     break;
               }
               break;
            case subrecord_conditions:
               if (this->get_type() == type::entry_point) {
                  auto& casted = std::get<data_types::entry_point>(this->data);

                  in_a_condition_tab = true;
                  subrecord.read(current_condition_tab);
                  if (current_condition_tab >= perk_condition_tab_count) {
                     specific_load_warnings::entry_point_condition_group_out_of_bounds notice(
                        intfc.target_stub,
                        which,
                        current_condition_tab,
                        perk_condition_tab_count
                     );
                     intfc.log_load_warning(notice);

                     if (current_condition_tab >= casted.condition_groups.size()) {
                        casted.condition_groups.resize(current_condition_tab + 1);
                     }
                  }
               } else {
                  specific_load_warnings::entry_point_data_for_effect_of_other_type notice(
                     intfc.target_stub,
                     which,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
            case 'CTDA':
               if (this->get_type() == type::entry_point) {
                  auto& casted = std::get<data_types::entry_point>(this->data);
                  if (!in_a_condition_tab) {
                     ++orphaned_condition_count;
                     if (casted.condition_groups.empty()) {
                        casted.condition_groups.resize(1);
                     }
                     break;
                  }
                  assert(current_condition_tab <= casted.condition_groups.size());
                  casted.condition_groups[current_condition_tab].read_next(record, intfc);
               } else {
                  specific_load_warnings::entry_point_data_for_effect_of_other_type notice(
                     intfc.target_stub,
                     which,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
            case perk_entry_point_data::subrecord_function_type:
            case perk_entry_point_data::subrecord_function_data_1:
            case perk_entry_point_data::subrecord_function_data_2:
            case perk_entry_point_data::subrecord_function_data_3:
               if (this->get_type() == type::entry_point) {
                  auto& casted = std::get<data_types::entry_point>(this->data);
                  casted.function.load(subrecord, intfc, which);
               } else {
                  specific_load_warnings::entry_point_data_for_effect_of_other_type notice(
                     intfc.target_stub,
                     which,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
         }
         if (terminated)
            break;
      }
      if (orphaned_condition_count > 0) {
         specific_load_warnings::orphaned_entry_point_conditions notice(
            intfc.target_stub,
            which,
            orphaned_condition_count
         );
         intfc.log_load_warning(notice);
      }
      if (this->get_type() == type::entry_point) {
         auto& casted = std::get<data_types::entry_point>(this->data);
         if ((size_t)casted.entry < all_perk_entry_points.size()) {
            const auto&  entry_point_info = all_perk_entry_points[(size_t)casted.entry];
            const size_t actual_count     = casted.condition_groups.size();
            const auto   expected_count   = entry_point_info.arg_count();
            if (perk_condition_tab_count != expected_count || (actual_count > perk_condition_tab_count && actual_count != expected_count)) {
               specific_load_warnings::wrong_condition_group_count_for_entry_point notice(
                  intfc.target_stub,
                  which,
                  casted.entry,
                  actual_count,
                  perk_condition_tab_count,
                  expected_count
               );
               intfc.log_load_warning(notice);
            }
         }
      }
      if (!terminated) {
         specific_load_warnings::unterminated_perk_effect notice(
            intfc.target_stub,
            which
         );
         intfc.log_load_warning(notice);
      }
   }
   /*static*/ void perk_effect::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      type effect_type = (type)-1;
      {
         auto& subrecord = record.get_current_subrecord();
         assert(subrecord.signature() == subrecord_start);
         subrecord.read(effect_type);
      }

      perk_entry_point_data::use_info_state entry_point_uis;
      form_id_t quest_or_ability;

      bool has_any_condition_groups = false;
      while (auto& subrecord = record.next_subrecord()) {
         bool terminated = false;
         switch (subrecord.signature()) {
            case subrecord_end:
               terminated = true;
               break;

            case 'DATA':
               switch (effect_type) {
                  case type::quest_and_stage:
                  case type::ability:
                     subrecord.read(quest_or_ability);
                     break;
                  case type::entry_point:
                     break;
               }
               break;
            case subrecord_conditions:
               if (effect_type != type::entry_point)
                  break;
               has_any_condition_groups = true;
               break;
            case 'CTDA':
               if (effect_type != type::entry_point)
                  break;
               //if (!has_any_condition_groups)
               //   break;
               components::condition::generate_use_info(record, uib);
               break;
            case perk_entry_point_data::subrecord_function_type:
            case perk_entry_point_data::subrecord_function_data_1:
            case perk_entry_point_data::subrecord_function_data_2:
            case perk_entry_point_data::subrecord_function_data_3:
               entry_point_uis.read(subrecord);
               break;
         }
         if (terminated)
            break;
      }
      uib.add_outbound_reference(quest_or_ability);
   }
   void perk_effect::save(tes_record_writer& record, load_order_interfaces::form_save& intfc, size_t which) {
      {
         auto& subrecord = record.open_next_subrecord(subrecord_start);
         subrecord.write(this->get_type());
         subrecord.write(this->rank);
         subrecord.write(this->priority);
         subrecord.close();
      }
      switch (this->get_type()) {
         case type::quest_and_stage:
            {
               auto& casted    = std::get<data_types::quest>(this->data);
               auto& subrecord = record.open_next_subrecord('DATA');
               subrecord.write(casted.quest);
               subrecord.write(casted.stage);
               subrecord.close();

            }
            break;
         case type::ability:
            {
               auto& casted    = std::get<data_types::ability>(this->data);
               auto& subrecord = record.open_next_subrecord('DATA');
               subrecord.write(casted.spell);
               subrecord.close();
            }
            break;
         case type::entry_point:
            {
               auto& casted = std::get<data_types::entry_point>(this->data);
               { // DATA
                  auto& subrecord = record.open_next_subrecord('DATA');
                  subrecord.write(casted.entry);
                  subrecord.write(casted.function.function);

                  if (casted.condition_groups.size() > max_available_condition_groups) {
                     auto notice = specific_save_errors::too_many_condition_groups_for_perk_entry(
                        *intfc.target_stub,
                        which,
                        casted.condition_groups.size()
                     );
                     intfc.throw_save_error(notice);
                  }
                  uint8_t group_count = casted.condition_groups.size();
                  subrecord.write(group_count);
                  subrecord.close();
               }
               for (size_t i = 0; i < casted.condition_groups.size(); ++i) {
                  auto& group     = casted.condition_groups[i];
                  auto& subrecord = record.open_next_subrecord(subrecord_conditions);
                  subrecord.write((uint8_t)i);
                  subrecord.close();
                  for (auto& cnd : group)
                     cnd.save(record, intfc);
               }
               casted.function.save(record, intfc);
            }
            break;
      }
      record.open_next_subrecord(subrecord_end).close();
   }

   void perk_effect::clone_from(const perk_effect& src, Form& my_owner) noexcept {
      this->rank     = src.rank;
      this->priority = src.priority;

      const auto type = src.get_type();
      this->set_type(my_owner, type);
      switch (type) {
         case type::ability:
            std::get<data_types::ability>(this->data).spell.set(my_owner, std::get<data_types::ability>(src.data).spell);
            break;
         case type::quest_and_stage:
            {
               auto& src_data = std::get<data_types::quest>(src.data);
               auto& dst_data = std::get<data_types::quest>(this->data);
               dst_data.quest.set(my_owner, src_data.quest);
               dst_data.stage = src_data.stage;
            }
            break;
         case type::entry_point:
            {
               auto& src_data = std::get<data_types::entry_point>(src.data);
               auto& dst_data = std::get<data_types::entry_point>(this->data);
               dst_data.entry = src_data.entry;
               dst_data.function.clone_from(src_data.function, my_owner);

               size_t size = src_data.condition_groups.size();
               for (auto& group : dst_data.condition_groups)
                  group.clear(my_owner);
               dst_data.condition_groups.clear();
               dst_data.condition_groups.resize(size);
               for (size_t i = 0; i < size; ++i) {
                  auto& src_group = src_data.condition_groups[i];
                  auto& dst_group = dst_data.condition_groups[i];
                  dst_group.append_all_of(my_owner, src_group);
               }
            }
            break;
      }
   }
   void perk_effect::clear(Form& my_owner) noexcept {
      this->rank     = 0;
      this->priority = 0;
      switch (this->get_type()) {
         case type::quest_and_stage:
            std::get<data_types::quest>(this->data).quest.set(my_owner, nullptr);
            break;
         case type::ability:
            std::get<data_types::ability>(this->data).spell.set(my_owner, nullptr);
            break;
         case type::entry_point:
            std::get<data_types::entry_point>(this->data).function.clear(my_owner);
            break;
      }
      this->data.emplace<data_types::quest>();
   }
   void perk_effect::sever_outbound_references_to(form_stub& other, Form& my_owner) noexcept {
      switch (this->get_type()) {
         case type::quest_and_stage:
            std::get<data_types::quest>(this->data).quest.clear_if(my_owner, other);
            break;
         case type::ability:
            std::get<data_types::ability>(this->data).spell.clear_if(my_owner, other);
            break;
         case type::entry_point:
            {
               auto& casted = std::get<data_types::entry_point>(this->data);
               casted.function.sever_outbound_references_to(other, my_owner);
               for (auto& group : casted.condition_groups)
                  for (auto& cnd : group)
                     cnd.sever_outbound_references_to(other, my_owner);
            }
            break;
      }
   }
}