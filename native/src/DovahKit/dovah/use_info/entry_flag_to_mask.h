#pragma once
#include "./entry_flag_underlying_type.h"
#include "./is_entry_flag_type.h"

namespace dovah::use_info {
   template<typename T> requires is_entry_flag_type_v<T>
   constexpr entry_flag_underlying_type entry_flag_to_mask(T v) {
      return (entry_flag_underlying_type)1 << (size_t)v;
   }
}