#include "./perk_entry_point_data.h"
#include "../_common_cpp.h"
#include "../../data/actor_values.h"

#include "../../notices/form_load_warnings/by_form_type/perk/effect_entry_point_has_mismatched_data_for_type.h"
#include "../../notices/form_load_warnings/by_form_type/perk/effect_entry_point_has_mismatched_type_for_function.h"
#include "../../notices/form_load_warnings/by_form_type/perk/effect_entry_point_params_specify_an_invalid_av.h"

namespace {
   namespace specific_load_warnings {
      using namespace dovah::notices::form_load_warnings::by_type::perk;
   }
}

namespace dovah::loaded_forms::structs {
   void perk_entry_point_data::_set_data_variant_to_type(entry_point_function_type t) {
      this->data = std::monostate{};
      switch (t) {
         case entry_point_function_type::activate_choice:
            this->data.emplace<data_types::activate_choice>();
            break;
         case entry_point_function_type::animation_graph_var:
            this->data.emplace<std::string>();
            break;
         case entry_point_function_type::leveled_item:
            this->data.emplace<data_types::leveled_item>();
            break;
         case entry_point_function_type::localized_string:
            this->data.emplace<localized_string>();
            break;
         case entry_point_function_type::none:
            break;
         case entry_point_function_type::one_float:
            this->data.emplace<data_types::one_float>();
            break;
         case entry_point_function_type::spell:
            this->data.emplace<data_types::spell>();
            break;
         case entry_point_function_type::two_floats:
            this->data.emplace<data_types::two_floats>();
            break;
      }
   }

   bool perk_entry_point_data::first_float_is_actor_value() const {
      switch (this->function) {
         case entry_point_function::add_actor_value_mult:
         case entry_point_function::set_to_actor_value_mult:
         case entry_point_function::multiply_actor_value_mult:
         case entry_point_function::multiply_one_plus_av_mult:
            return true;
      }
      return false;
   }

   entry_point_function_type perk_entry_point_data::type() const {
      return (entry_point_function_type)this->data.index();
   }
   void perk_entry_point_data::set_type(loaded_forms::Form& my_owner, entry_point_function_type t) {
      if ((size_t)t == this->data.index())
         return;
      switch ((entry_point_function_type)this->data.index()) {
         case entry_point_function_type::activate_choice:
            {
               auto& casted = std::get<data_types::activate_choice>(this->data);
               casted.spell.set(my_owner, nullptr);
               casted.text.reset();
            }
            break;
         case entry_point_function_type::leveled_item:
            std::get<data_types::leveled_item>(this->data).form.set(my_owner, nullptr);
            break;
         case entry_point_function_type::spell:
            std::get<data_types::spell>(this->data).form.set(my_owner, nullptr);
            break;
      }
      this->_set_data_variant_to_type(t);
   }

   void perk_entry_point_data::load_function_data_type(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, size_t which_effect) {
      assert(subrecord.signature() == subrecord_function_type);
      //
      // NOTE: The game fails loading if the subrecord size isn't exactly 1 byte.
      //
      entry_point_function_type v;
      if (subrecord.read(v)) {
         this->_set_data_variant_to_type(v);
         if (v != expected_type_for_entry_point_function(this->function)) {
            specific_load_warnings::effect_entry_point_has_mismatched_type_for_function notice(
               intfc.target_stub,
               which_effect,
               this->function,
               expected_type_for_entry_point_function(this->function),
               v
            );
            intfc.log_load_warning(notice);
         }
      }
   }
   void perk_entry_point_data::load_function_data(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, size_t which_effect) {
      auto signature = subrecord.signature();
      if (signature == subrecord_function_data_1) {
         switch (this->type()) {
            case entry_point_function_type::none:
               break;
            case entry_point_function_type::one_float:
               if (subrecord.size() != 4) {
                  break; // BGSEntryPointFunctionDataOneValue::Load aborts if the subrecord is an unexpected size (even if it's larger).
               } else {
                  auto& casted = std::get<data_types::one_float>(this->data);
                  subrecord.unchecked_read(casted.value);
               }
               break;
            case entry_point_function_type::two_floats:
               if (subrecord.size() != 8) {
                  break;
               } else {
                  auto& casted = std::get<data_types::two_floats>(this->data);
                  subrecord.read(casted.a);
                  subrecord.read(casted.b);
                  if (this->first_float_is_actor_value()) {
                     bool invalid_av = false;
                     if (casted.a != (int32_t)casted.a || casted.a < 0 || casted.a >= all_actor_value_info.size()) {
                        specific_load_warnings::effect_entry_point_params_specify_an_invalid_av notice(
                           intfc.target_stub,
                           which_effect,
                           casted.a
                        );
                        intfc.log_load_warning(notice);
                     }
                  }
               }
               break;
            case entry_point_function_type::leveled_item:
               if (subrecord.size() != 4) {
                  break;
               } else {
                  auto& casted = std::get<data_types::leveled_item>(this->data);
                  if (auto& form = casted.form; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::leveled_item, subrecord.signature());
               }
               break;
            case entry_point_function_type::activate_choice:
               if (subrecord.size() != 4) {
                  break;
               } else {
                  auto& casted = std::get<data_types::activate_choice>(this->data);
                  if (auto& form = casted.spell; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
               }
               break;
            case entry_point_function_type::spell:
               if (subrecord.size() != 4) {
                  break;
               } else {
                  auto& casted = std::get<data_types::spell>(this->data);
                  if (auto& form = casted.form; subrecord.read(form))
                     intfc.warn_if_ref_is_wrong_type(form, form_type::spell, subrecord.signature());
               }
               break;
            case entry_point_function_type::animation_graph_var:
               {
                  auto& casted = std::get<std::string>(this->data);
                  subrecord.read(casted);
               }
               break;
            case entry_point_function_type::localized_string:
               {
                  auto& casted = std::get<localized_string>(this->data);
                  subrecord.read(casted);
               }
               break;
         }
         return;
      }
      
      if (signature == subrecord_function_data_2) {
         switch (this->type()) {
            case entry_point_function_type::none:
            case entry_point_function_type::one_float:
            case entry_point_function_type::two_floats:
            case entry_point_function_type::leveled_item:
            case entry_point_function_type::spell:
            case entry_point_function_type::animation_graph_var:
            case entry_point_function_type::localized_string:
               {
                  specific_load_warnings::effect_entry_point_has_mismatched_data_for_type notice(
                     intfc.target_stub,
                     which_effect,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
            case entry_point_function_type::activate_choice:
               {
                  auto& casted = std::get<data_types::activate_choice>(this->data);
                  subrecord.read(casted.text);
               }
               break;
         }
         return;
      }

      if (signature == subrecord_function_data_3) {
         switch (this->type()) {
            case entry_point_function_type::none:
            case entry_point_function_type::one_float:
            case entry_point_function_type::two_floats:
            case entry_point_function_type::leveled_item:
            case entry_point_function_type::spell:
            case entry_point_function_type::animation_graph_var:
            case entry_point_function_type::localized_string:
               {
                  specific_load_warnings::effect_entry_point_has_mismatched_data_for_type notice(
                     intfc.target_stub,
                     which_effect,
                     subrecord.signature()
                  );
                  intfc.log_load_warning(notice);
               }
               break;
            case entry_point_function_type::activate_choice:
               {
                  auto& casted = std::get<data_types::activate_choice>(this->data);
                  if (subrecord.get_containing_record().version() < 0x23) {
                     subrecord.read(casted.flags);
                     casted.fragment_index = data_types::activate_choice::no_fragment;
                  } else {
                     if (subrecord.size() != 8)
                        break;
                     subrecord.read(casted.flags);
                     subrecord.read(casted.fragment_index);
                  }
               }
               break;
         }
         return;
      }
   }

   void perk_entry_point_data::load(tes_subrecord_reader& subrecord, load_order_interfaces::form_load& intfc, size_t which_effect) {
      switch (subrecord.signature()) {
         case subrecord_function_type:
            this->load_function_data_type(subrecord, intfc, which_effect);
            break;
         case subrecord_function_data_1:
         case subrecord_function_data_2:
         case subrecord_function_data_3:
            this->load_function_data(subrecord, intfc, which_effect);
            break;
         default: // invalid
            assert(false && "Why was harvestable::load called on a subrecord it's not built to handle?");
      }
   }
   void perk_entry_point_data::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      const auto type = this->type();
      {
         auto& subrecord = record.open_next_subrecord(subrecord_function_type);
         subrecord.write(type);
         subrecord.close();
      }
      if (type == entry_point_function_type::activate_choice) {
         auto& casted = std::get<data_types::activate_choice>(this->data);
         {
            auto& subrecord = record.open_next_subrecord(subrecord_function_data_2);
            subrecord.write(casted.text);
            subrecord.close();
         }
         {
            auto& subrecord = record.open_next_subrecord(subrecord_function_data_3);
            subrecord.write(casted.flags);
            subrecord.write(casted.fragment_index);
            subrecord.close();
         }
      }
      if (type != entry_point_function_type::none) {
         auto& subrecord = record.open_next_subrecord(subrecord_function_data_1);
         switch (type) {
            case entry_point_function_type::one_float:
               {
                  auto& casted = std::get<data_types::one_float>(this->data);
                  subrecord.write(casted.value);
               }
               break;
            case entry_point_function_type::two_floats:
               {
                  auto& casted = std::get<data_types::two_floats>(this->data);
                  subrecord.write(casted.a);
                  subrecord.write(casted.b);
               }
               break;
            case entry_point_function_type::leveled_item:
               {
                  auto& casted = std::get<data_types::leveled_item>(this->data);
                  subrecord.write(casted.form);
               }
               break;
            case entry_point_function_type::activate_choice:
               {
                  auto& casted = std::get<data_types::activate_choice>(this->data);
                  subrecord.write(casted.spell);
               }
               break;
            case entry_point_function_type::spell:
               {
                  auto& casted = std::get<data_types::spell>(this->data);
                  subrecord.write(casted.form);
               }
               break;
            case entry_point_function_type::animation_graph_var:
               {
                  auto& casted = std::get<std::string>(this->data);
                  subrecord.write(casted);
               }
               break;
            case entry_point_function_type::localized_string:
               {
                  auto& casted = std::get<localized_string>(this->data);
                  subrecord.write(casted);
               }
               break;
         }
         subrecord.close();
      }
   }
   void perk_entry_point_data::clone_from(const perk_entry_point_data& original, loaded_forms::Form& my_owner) noexcept {
      this->function = original.function;
      this->set_type(my_owner, original.type());
      this->data = original.data;
   }
   void perk_entry_point_data::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      switch (this->type()) {
         case entry_point_function_type::activate_choice:
            {
               auto& casted = std::get<data_types::activate_choice>(this->data);
               casted.spell.clear_if(my_owner, target);
            }
            break;
         case entry_point_function_type::leveled_item:
            std::get<data_types::leveled_item>(this->data).form.clear_if(my_owner, target);
            break;
         case entry_point_function_type::spell:
            std::get<data_types::spell>(this->data).form.clear_if(my_owner, target);
            break;
      }
   }
   void perk_entry_point_data::clear(loaded_forms::Form& my_owner) {
      this->function = entry_point_function::none;
      this->set_type(my_owner, entry_point_function_type::none);
   }

   void perk_entry_point_data::use_info_state::read(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      switch (signature) {
         case subrecord_function_type:
            subrecord.read(this->type);
            break;
         case subrecord_function_data_1:
            switch (this->type) {
               case entry_point_function_type::leveled_item:
               case entry_point_function_type::spell:
               case entry_point_function_type::activate_choice:
                  if (subrecord.size() != 4)
                     break;
                  subrecord.read(this->form);
                  break;
            }
            break;
      }
   }
   void perk_entry_point_data::use_info_state::commit(form_stub_use_info_builder& uib) {
      uib.add_outbound_reference(this->form);
   }
}