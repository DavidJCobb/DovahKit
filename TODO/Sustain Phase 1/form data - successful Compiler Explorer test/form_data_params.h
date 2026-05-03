#pragma once
#include <type_traits>
#include "use_info_management_mode.h"

namespace dovah {
   // This is a struct in case we need to extend it later.
   struct default_form_data_params {
      default_form_data_params() = delete;
      static constexpr const use_info_management_mode management_mode = use_info_management_mode::managed;
   };

   struct managed_form_params : public default_form_data_params {
      static constexpr const use_info_management_mode management_mode = use_info_management_mode::managed;
   };
   struct unmanaged_form_params : public default_form_data_params {
      static constexpr const use_info_management_mode management_mode = use_info_management_mode::unmanaged;
   };
   
   template<typename T>
   concept form_data_params = std::is_base_of_v<default_form_data_params, T>;

   template<template<form_data_params> typename Struct>
   using managed_data = Struct<managed_form_params>;
   template<template<form_data_params> typename Struct>
   using unmanaged_data = Struct<unmanaged_form_params>;
   
   template<typename T>
   struct uses_form_data_params : std::false_type {};
   template<template<form_data_params> typename T, form_data_params Params>
   struct uses_form_data_params<T<Params>> : std::true_type {};
   //
   namespace impl {
      template<typename T>
      struct _uses_form_data_params : std::false_type {};
      template<template<form_data_params> typename T, form_data_params Params>
      struct _uses_form_data_params<T<Params>> : std::true_type {};
   }
   //
   template<typename T>
   concept uses_form_data_params = (
      // Check if the type is directly templated on params...
      impl::_uses_form_data_params<T>::value

      // ...or if the type has a `using` declaration with params. We need this for 
      // nested types, since the inner type will not, itself, be templated on the 
      // params, instead "inheriting" them from the outer type.
      || requires {
         typename T::form_data_params_type;
         std::is_base_of_v<default_form_data_params, typename T::form_data_params_type>;
      }
   );

   template<typename T, template<form_data_params> typename Type>
   concept is_form_data_parameterized_type = requires {
      typename T::form_data_params_type;
      requires std::is_same_v<T, Type<typename T::form_data_params_type>>;
   };
}