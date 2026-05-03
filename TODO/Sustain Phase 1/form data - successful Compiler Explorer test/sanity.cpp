#pragma once
#include <type_traits>
#include <utility> // std::declval
#include "lu_std_array.h"
#include "form_data_params.h"
#include "form_data.h"
#include "managed_form_use.h"
namespace dovah {
   class form_stub;
}

/*
   Template function to sanity-check form-data classes, i.e. ensure 
   that no members of a managed type are unmanaged, nor vice versa.

   Not sure I'd recommend actually using this as a line of defense, 
   given that MSVC seems to get nauseous when it processes this, but 
   it's here as an academic exercise and a point of reference.
*/

namespace dovah::sanity {
   template<template<form_data_params> typename T>
   constexpr bool form_data_is_valid() {
      auto walk = [](this auto&& self, const auto& test, const auto& subject) consteval -> void {
         using subject_type = std::decay_t<decltype(subject)>;
         test.operator()<subject_type>();

         auto recurse = [&self, &test](const auto& f) consteval -> void {
            self(test, f);
         };

         if constexpr (uses_form_data_params<subject_type>) {
            subject_type::visit_fields(subject, recurse);
         } else if constexpr (lu::std_array<subject_type>) {
            using value_type = typename subject_type::value_type;
            if constexpr (uses_form_data_params<value_type>) {
               // Actually accessing a sub-object on `subject` causes MSVC to 
               // choke, but since `subject` is default-constructible, its 
               // sub-objects ought to be as well, I guess.
               value_type::visit_fields(value_type{}, recurse);
            }
         }
      };

      // Don't allow managed form data to contain bare `form_stub` pointers.
      walk([]<typename subject_type>() {
         static_assert(!(std::is_pointer_v<subject_type> && std::is_base_of_v<form_stub, std::remove_pointer_t<subject_type>>));
      }, T<managed_form_params>{});

      // Don't allow unmanaged form data to contain managed uses.
      walk([]<typename subject_type>() {
         static_assert(!std::is_base_of_v<managed_form_use, subject_type>);
      }, T<unmanaged_form_params>{});

      return true;
   }
}

// test:
//#include "shout.h"
//static_assert(dovah::sanity::form_data_is_valid<dovah::form_data::shout>());