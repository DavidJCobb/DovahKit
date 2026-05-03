#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "form_data_params.h"
namespace dovah {
   class form_stub;
   class managed_form_use;
}

namespace dovah {
   namespace form_data::alias_bags {
      struct managed {
         using form_use = managed_form_use;
      };
      struct unmanaged {
         using form_use = dovah::form_stub*;
      };
   }
   namespace form_data {
      template<form_data_params Params>
      using type_aliases = std::conditional_t<
         (Params::management_mode == use_info_management_mode::managed),
         alias_bags::managed,
         alias_bags::unmanaged
      >;
   }

   struct base_managed_form_data {
      public:
         // Commented out so we can create form data directly, for this 
         // Compiler Explorer test.
         //form_stub& stub;
   };
   struct base_unmanaged_form_data {
      public:
         struct {
            std::string editor_id;
            uint32_t    record_flags = 0;
         } stub;
   };
}
namespace dovah::form_data {
   template<form_data_params Params>
   using _base = std::conditional_t<
      (Params::management_mode == use_info_management_mode::managed),
      base_managed_form_data,
      base_unmanaged_form_data
   >;
}
namespace dovah {
   namespace impl {
      template<typename T>
      struct form_data_type : public std::false_type{};
      //
      template<template<form_data_params> typename T, form_data_params Params>
      struct form_data_type<T<Params>> : public std::bool_constant<
         std::is_base_of_v<form_data::_base<Params>, T<Params>>
      > {};
   }

   template<typename T>
   concept form_data_type = impl::form_data_type<T>::value;
}