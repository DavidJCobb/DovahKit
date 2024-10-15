#pragma once
#include <array>
#include "../form_types.h"

namespace dovah {
   constexpr const auto all_base_form_types = []() {
      constexpr const size_t count = []() {
         size_t out = 0;
         for (auto& info : form_types)
            if (form_type_is_base_form(info.form_type))
               ++out;
         return out;
      }();

      std::array<form_type, count> list = {};
      size_t i = 0;
      for (auto& info : form_types)
         if (form_type_is_base_form(info.form_type))
            list[i++] = info.form_type;
      return list;
   }();
}