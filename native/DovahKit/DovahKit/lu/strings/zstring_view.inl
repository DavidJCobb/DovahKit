#pragma once
#include "./zstring_view.h"

#pragma push_macro("TEMPLATE_PARAMS")
#pragma push_macro("CLASS_NAME")
#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#define TEMPLATE_PARAMS template<typename CharT>
#define CLASS_NAME basic_zstring_view<CharT>

namespace lu {
   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::starts_with(std_view_type s) const {
      return std_view_type(*this).starts_with(s);
   }
   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::starts_with(value_type c) const {
      return front() == c;
   }

   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::ends_with(std_view_type s) const {
      return std_view_type(*this).ends_with(s);
   }
   TEMPLATE_PARAMS
   constexpr bool CLASS_NAME::ends_with(value_type c) const {
      return back() == c;
   }
   
   #if __cpp_lib_string_contains >= 202011L
      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::contains(std_view_type s) const {
         return std_view_type(*this).contains(s);
      }
      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::contains(value_type c) const {
         return std_view_type(*this).contains(c);
      }
   #endif

      TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find(std_view_type needle, size_type pos) const noexcept {
      return std_view_type(*this).find(needle, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find(value_type needle, size_type pos) const noexcept {
      return std_view_type(*this).find(needle, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find(const value_type* needle, size_type pos, size_type needle_size) const noexcept {
      return std_view_type(*this).find(needle, pos, needle_size);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::rfind(std_view_type needle, size_type pos) const noexcept {
      return std_view_type(*this).rfind(needle, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::rfind(value_type needle, size_type pos) const noexcept {
      return std_view_type(*this).rfind(needle, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::rfind(const value_type* needle, size_type pos, size_type needle_size) const noexcept {
      return std_view_type(*this).rfind(needle, pos, needle_size);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_of(std_view_type set, size_type pos) const noexcept {
      return std_view_type(*this).find_first_of(set, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_of(value_type c, size_type pos) const noexcept {
      return std_view_type(*this).find_first_of(c, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std_view_type(*this).find_first_of(set, pos, set_size);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_of(std_view_type set, size_type pos) const noexcept {
      return std_view_type(*this).find_last_of(set, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_of(value_type c, size_type pos) const noexcept {
      return std_view_type(*this).find_last_of(c, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std_view_type(*this).find_last_of(set, pos, set_size);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_not_of(std_view_type set, size_type pos) const noexcept {
      return std_view_type(*this).find_first_not_of(set, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_not_of(value_type c, size_type pos) const noexcept {
      return std_view_type(*this).find_first_not_of(c, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_first_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std_view_type(*this).find_first_not_of(set, pos, set_size);
   }

   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_not_of(std_view_type set, size_type pos) const noexcept {
      return std_view_type(*this).find_last_not_of(set, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_not_of(value_type c, size_type pos) const noexcept {
      return std_view_type(*this).find_last_not_of(c, pos);
   }
   TEMPLATE_PARAMS
   constexpr CLASS_NAME::size_type CLASS_NAME::find_last_not_of(const_pointer set, size_type pos, size_type set_size) const noexcept {
      return std_view_type(*this).find_last_not_of(set, pos, set_size);
   }
}

#undef TEMPLATE_PARAMS
#undef CLASS_NAME
#pragma pop_macro("TEMPLATE_PARAMS")
#pragma pop_macro("CLASS_NAME")