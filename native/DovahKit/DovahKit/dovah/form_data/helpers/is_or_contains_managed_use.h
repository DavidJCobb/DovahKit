#pragma once
#include <array>
#include <tuple>
#include <type_traits>
#include <vector>
#include "./is_visitable.h"
namespace dovah {
   namespace form_data::pieces {
      template<typename T>
      class managed_vector;
   }
   class _base_managed_localized_string_use;
   class managed_form_use;
}

namespace dovah::form_data_helpers {
   namespace is_or_contains_managed_use_impl {
      template<typename T>
      concept is_managed_use = (
         std::is_base_of_v<managed_form_use, T>
      || std::is_base_of_v<_base_managed_localized_string_use, T>
      );

      template<typename T>
      struct tuple_contains_managed_use;

      template<typename T>
      struct is_or_contains_managed_use : public std::false_type {};
      //
      template<is_managed_use T>
      struct is_or_contains_managed_use<T> : public std::true_type {};
      //
      template<is_visitable T>
      struct is_or_contains_managed_use<T> : public tuple_contains_managed_use<typename T::member_types> {};

      // Containers:
      template<typename T, size_t S>
      struct is_or_contains_managed_use<std::array<T, S>> : public is_or_contains_managed_use<T> {};

      template<typename T>
      struct is_or_contains_managed_use<std::vector<T>> : public is_or_contains_managed_use<T> {};

      template<typename T>
      struct is_or_contains_managed_use<form_data::pieces::managed_vector<T>> : public is_or_contains_managed_use<T> {};

      // Specialization of a helper template for the "visitable" types:
      template<typename... Types>
      struct tuple_contains_managed_use<std::tuple<Types...>> {
         static constexpr const bool value = (is_or_contains_managed_use<Types>::value || ...);
      };
   }

   template<typename T>
   concept is_or_contains_managed_use = is_or_contains_managed_use_impl::is_or_contains_managed_use<T>::value;
}