#pragma once
#include <string>
#include "../concepts/instantiation_of/std_basic_string.h"
#include "../concepts/instantiation_of/std_basic_string_view.h"
namespace lu {
   template<typename CharT>
   class basic_zstring_view;
}

namespace lu::ascii {
   namespace impl {
      template<typename T>
      constexpr auto find_first_non_whitespace(const T& subject) {
         return subject.find_first_not_of(" \f\n\r\t\v");
      }

      template<typename T>
      constexpr auto find_last_non_whitespace(const T& subject) {
         return subject.find_last_not_of(" \f\n\r\t\v");
      }

      template<typename T>
      concept can_left_trim_whitespace = concepts::instantiation_of_std_basic_string<T> || concepts::instantiation_of_std_basic_string_view<T> || requires {
         typename T::value_type;
         requires std::is_same_v<T, basic_zstring_view<typename T::value_type>>;
      };

      template<typename T>
      concept can_right_trim_whitespace = concepts::instantiation_of_std_basic_string<T> || concepts::instantiation_of_std_basic_string_view<T>;

      template<typename T>
      concept can_trim_whitespace = can_left_trim_whitespace<T> && can_right_trim_whitespace<T>;
   }

   template<impl::can_left_trim_whitespace T>
   constexpr void ltrim_in_place(T& subject) {
      const auto i = impl::find_first_non_whitespace(subject);
      if (i == std::string::npos)
         return;
      if constexpr (requires (T & v, size_t n) {
         { v.remove_prefix(n) };
      }) {
         subject.remove_prefix(i);
      } else if constexpr (concepts::instantiation_of_std_basic_string<T>) {
         subject.erase(subject.begin(), subject.begin() + i);
      } else {
         subject = T(subject.begin() + i, subject.end());
      }
   }
   template<impl::can_left_trim_whitespace T>
   constexpr T ltrim(const T& subject) {
      auto v = subject;
      ltrim_in_place(v);
      return v;
   }

   template<impl::can_right_trim_whitespace T>
   constexpr void rtrim_in_place(T& subject) {
      const auto i = impl::find_last_non_whitespace(subject);
      if (i == std::string::npos || i == subject.size() - 1)
         return;
      if constexpr (requires (T & v, size_t n) {
         { v.remove_suffix(n) };
      }) {
         subject.remove_suffix(subject.size() - i - 1);
      } else if constexpr (concepts::instantiation_of_std_basic_string<T>) {
         subject.resize(i + 1);
      } else {
         subject = T(subject.begin(), subject.begin() + i + 1);
      }
   }
   template<impl::can_right_trim_whitespace T>
   constexpr T rtrim(const T& subject) {
      auto v = subject;
      rtrim_in_place(v);
      return v;
   }

   template<impl::can_trim_whitespace T>
   constexpr void trim_in_place(T& subject) {
      ltrim_in_place(subject);
      rtrim_in_place(subject);
   }
   template<impl::can_trim_whitespace T>
   constexpr T trim(const T& subject) {
      auto v = subject;
      trim_in_place(v);
      return v;
   }
}
