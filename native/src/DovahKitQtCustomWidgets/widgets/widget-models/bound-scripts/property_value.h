#pragma once
#include <cstdint>
#include <variant>
#include <vector>
#include <QString>
#include "dovah/forms/components/papyrus/property_type.h"
#include "dovah/forms/components/papyrus/property_value.h"
#include "ui/types/quest_alias.h"

#include "./_forward_declarations.h"

namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}

namespace ui::bound_script_models {
   using property_value = std::variant<
      std::monostate, // only for clearing an inherited property value REFR-side
      //
      dovah::form_stub*,
      ui::types::quest_alias,
      QString,
      int32_t,
      float,
      bool,
      //
      std::vector<dovah::form_stub*>,
      std::vector<ui::types::quest_alias>,
      std::vector<QString>,
      std::vector<int32_t>,
      std::vector<float>,
      std::vector<bool>
   >;

   void property_value_to_vmad(const property_value& src, vmad::property_value& dst, dovah::loaded_forms::Form& dst_form);

   QString stringify(const property_value&);

   constexpr vmad::property_type property_type_for(const property_value& src) {
      if (std::holds_alternative<std::monostate>(src))
         return vmad::property_type::none;

      if (std::holds_alternative<dovah::form_stub*>(src))
         return vmad::property_type::object;
      if (std::holds_alternative<ui::types::quest_alias>(src))
         return vmad::property_type::object;
      if (std::holds_alternative<QString>(src))
         return vmad::property_type::string;
      if (std::holds_alternative<int32_t>(src))
         return vmad::property_type::integer;
      if (std::holds_alternative<float>(src))
         return vmad::property_type::float32;
      if (std::holds_alternative<bool>(src))
         return vmad::property_type::boolean;

      if (std::holds_alternative<std::vector<dovah::form_stub*>>(src))
         return vmad::property_type::array_of_object;
      if (std::holds_alternative<std::vector<ui::types::quest_alias>>(src))
         return vmad::property_type::array_of_object;
      if (std::holds_alternative<std::vector<QString>>(src))
         return vmad::property_type::array_of_string;
      if (std::holds_alternative<std::vector<int32_t>>(src))
         return vmad::property_type::array_of_integer;
      if (std::holds_alternative<std::vector<float>>(src))
         return vmad::property_type::array_of_float32;
      if (std::holds_alternative<std::vector<bool>>(src))
         return vmad::property_type::array_of_boolean;

      return vmad::property_type::none;
   }
}