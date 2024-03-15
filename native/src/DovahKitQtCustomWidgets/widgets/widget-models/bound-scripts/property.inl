#pragma once
#include "./property.h"

namespace ui::bound_script_models {
   constexpr std::optional<vmad::property_status> property::get_computed_status() const {
      if (this->values.local.has_value()) {
         auto& value = this->values.local.value();
         if (std::holds_alternative<std::monostate>(value))
            return vmad::property_status::inherited_and_removed;
         return vmad::property_status::defined_locally;
      }
      if (this->values.inherited.has_value()) {
         return vmad::property_status::defined_only_on_base;
      }
      return {};
   }

   constexpr bool property::is_inherited() const {
      auto& opt = this->values.inherited;
      if (!opt.has_value())
         return false;
      if (std::holds_alternative<std::monostate>(opt.value())) // is the base form clearing a PEX-side default value?
         return false; // or should this be true?
      return true;
   }
   
   // ---

   constexpr bool property::is_form_or_array_thereof() const {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::object:
         case vmad::property_type::array_of_object:
            break;
         default:
            return false;
      }

      auto& object_info = this->typeinfo.object;

      dovah::form_type type;
      //
      if (object_info.native_type.has_value())
         type = object_info.native_type.value();
      else if (object_info.guessed_type.has_value())
         type = object_info.guessed_type.value();
      else
         return false;
      //
      switch (type) {
         case dovah::form_type::alias:
         case dovah::form_type::location_alias:
         case dovah::form_type::reference_alias:
            return false;
      }

      return true;
   }

   constexpr bool property::is_object_or_array_thereof() const {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::object:
         case vmad::property_type::array_of_object:
            return true;
      }
      return false;
   }

   constexpr bool property::is_quest_alias_or_array_thereof() const {
      switch (this->typeinfo.raw_type) {
         case vmad::property_type::object:
         case vmad::property_type::array_of_object:
            break;
         default:
            return false;
      }

      auto& object_info = this->typeinfo.object;

      dovah::form_type type;
      //
      if (object_info.native_type.has_value())
         type = object_info.native_type.value();
      else if (object_info.guessed_type.has_value())
         type = object_info.guessed_type.value();
      else
         return false;
      //
      switch (type) {
         case dovah::form_type::alias:
         case dovah::form_type::location_alias:
         case dovah::form_type::reference_alias:
            return true;
      }

      return false;
   }
}
