#pragma once
#include "./property_value.h"

namespace ui::types::papyrus {
   constexpr value_type value_type_of(const property_value& v) {
      value_type out;
      std::visit(
         [&out](const auto& data) {
            using value_type = std::decay_t<decltype(data)>;
            out = value_type_for<value_type>;
         },
         v
      );
      return out;
   }

   constexpr property_value property_value_from_type(value_type vt) {
      if (vt.is_array) {
         switch (vt.base) {
            case single_value_type::none:
               return {};

            case single_value_type::boolean:
               return std::vector<bool>{};
            case single_value_type::float32:
               return std::vector<float>{};
            case single_value_type::integer:
               return std::vector<int32_t>{};
            case single_value_type::string:
               return std::vector<QString>{};

            case single_value_type::alias:
               return std::vector<ui::types::quest_alias>{};
            case single_value_type::form:
               return std::vector<dovah::form_stub*>{};
         }
      } else {
         switch (vt.base) {
            case single_value_type::none:
               return {};

            case single_value_type::boolean:
               return bool{};
            case single_value_type::float32:
               return float{};
            case single_value_type::integer:
               return int32_t{};
            case single_value_type::string:
               return QString{};

            case single_value_type::alias:
               return ui::types::quest_alias{};
            case single_value_type::form:
               return (dovah::form_stub*)nullptr;
         }
      }

      return {};
   }
}