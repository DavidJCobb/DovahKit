#include "./entry_point_entry.h"
#include "dovah/form_stub.h"
#include "dovah/form_types.h"

namespace {
   template<typename ValueType, typename Variant>
   void _switch_variant_to_type(Variant& v) {
      if (!std::holds_alternative<ValueType>(v))
         v.emplace<ValueType>();
   }
}

namespace ui::types::perk_entries {
   void entry_point_entry::_set_function_type(dovah::entry_point_value_type vt) {
      auto type_prior = dovah::entry_point_value_type_of(this->function);
      if (type_prior != vt) {
         this->parameters.emplace<std::monostate>();
         //
         // Reset our chosen function.
         //
         switch (vt) {
            case dovah::entry_point_value_type::activate_choice:
               this->function = dovah::entry_point_function::add_activate_choice;
               break;
            case dovah::entry_point_value_type::none:
               this->function = dovah::entry_point_function::none;
               break;
            case dovah::entry_point_value_type::number:
               this->function = dovah::entry_point_function::absolute_value;
               break;
            case dovah::entry_point_value_type::leveled_item:
               this->function = dovah::entry_point_function::add_leveled_list;
               break;
            case dovah::entry_point_value_type::spell:
               this->function = dovah::entry_point_function::select_spell;
               break;
            case dovah::entry_point_value_type::raw_string:
               this->function = dovah::entry_point_function::select_text;
               break;
            case dovah::entry_point_value_type::localized_string:
               this->function = dovah::entry_point_function::set_text;
               break;
         }
      }
      //
      // Update parameters to match function type.
      //
      switch (vt) {
         case dovah::entry_point_value_type::activate_choice:
            _switch_variant_to_type<params::activate_choice>(this->parameters);
            break;
         case dovah::entry_point_value_type::number:
            if (
               !std::holds_alternative<params::one_float>(this->parameters) &&
               !std::holds_alternative<params::one_av_one_float>(this->parameters)&&
               !std::holds_alternative<params::two_floats>(this->parameters)
            ) {
               _switch_variant_to_type<params::one_float>(this->parameters);
            }
            break;
         case dovah::entry_point_value_type::leveled_item:
            _switch_variant_to_type<params::form>(this->parameters);
            {
               auto& data = std::get<params::form>(this->parameters);
               if (data.value && data.value->form_type != dovah::form_type::leveled_item)
                  data.value = nullptr;
            }
            break;
         case dovah::entry_point_value_type::spell:
            _switch_variant_to_type<params::form>(this->parameters);
            {
               auto& data = std::get<params::form>(this->parameters);
               if (data.value && data.value->form_type != dovah::form_type::spell)
                  data.value = nullptr;
            }
            break;
         case dovah::entry_point_value_type::localized_string:
            _switch_variant_to_type<params::localized_string>(this->parameters);
            break;
         case dovah::entry_point_value_type::raw_string:
            _switch_variant_to_type<params::raw_string>(this->parameters);
            break;
      }
   }

   void entry_point_entry::set_entry_point(dovah::perk_entry_point ep) {
      const auto prior = this->entry_point;
      this->entry_point = ep;
      if ((size_t)ep >= dovah::all_perk_entry_points.size()) {
         this->function   = dovah::entry_point_function::none;
         this->parameters = {};
         this->conditions_by_entity.clear();
         return;
      }
      const auto& info = dovah::all_perk_entry_points[(size_t)ep];
      this->_set_function_type(info.value_type);
      if (prior != ep) {
         this->conditions_by_entity.clear();

         size_t size = 0;
         for (auto& arg : info.args) {
            if (!arg.name || arg.name[0] == '\0')
               break;
            ++size;
         }
         this->conditions_by_entity.resize(size);
      }
   }
   void entry_point_entry::set_function(dovah::entry_point_function f) {
      if ((size_t)this->entry_point >= dovah::all_perk_entry_points.size()) {
         this->function   = dovah::entry_point_function::none;
         this->parameters = {};
         return;
      }
      auto type_prior = dovah::entry_point_value_type_of(this->function);
      auto type_after = dovah::entry_point_value_type_of(f);
      if (type_prior != type_after) {
         this->_set_function_type(type_after);
      }
      this->function = f;
   }
}