#pragma once

namespace ui::types::papyrus {
   enum class single_value_type {
      none,

      boolean,
      float32,
      integer,
      string,

      alias,
      form,
   };



   struct value_type {
      single_value_type base = single_value_type::none;
      bool is_array = false;
   };
   


   template<typename T>
   constexpr const value_type value_type_for = []() {
      if constexpr (cobb::is_std_vector<T>) {
         value_type v = value_type_for<typename T::value_type>;
         v.is_array = true;
         return v;
      } else {
         if constexpr (std::is_same_v<T, std::monostate>)
            return value_type{ single_value_type::none };
         else if constexpr (std::is_same_v<T, bool>)
            return value_type{ single_value_type::boolean };
         else if constexpr (std::is_same_v<T, float>)
            return value_type{ single_value_type::float32 };
         else if constexpr (std::is_same_v<T, int32_t>)
            return value_type{ single_value_type::integer };
         else if constexpr (std::is_same_v<T, QString>)
            return value_type{ single_value_type::string };
         else if constexpr (std::is_same_v<T, ui::types::quest_alias>)
            return value_type{ single_value_type::alias };
         else if constexpr (std::is_same_v<T, dovah::form_stub*> || std::is_same_v<T, const dovah::form_stub*>)
            return value_type{ single_value_type::form };
         else
            std::unreachable();
      }
   }();
}