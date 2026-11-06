#pragma once
#include "lu/passkey.h"
#include "./form_data/params.h"
#include "./form_stub.h"
#include "./form_data/form_type_for_form_data_class.h"
#include "./utils/form_types/is_ref.h"
namespace dovah {
   class base_managed_form_data;
}

namespace dovah {
   namespace form_data_pointer_impl {
      class base {
         protected:
            form_stub* _stub = nullptr;

         protected:
            void _inc();
            void _dec();

            constexpr const base_managed_form_data* _get_data() const noexcept;
            constexpr base_managed_form_data* _get_data() noexcept;
      };
   }

   template<template<typename> typename>
   class form_data_pointer;

   class untyped_form_data_pointer : public form_data_pointer_impl::base {
      template<template<typename> typename> friend class form_data_pointer;
      public:
         constexpr untyped_form_data_pointer() {}
         untyped_form_data_pointer(const untyped_form_data_pointer& other);
         constexpr untyped_form_data_pointer(untyped_form_data_pointer&& other) noexcept;
         ~untyped_form_data_pointer();

         constexpr operator bool() const noexcept;
         constexpr bool operator!() const noexcept;
         constexpr operator const base_managed_form_data*() const noexcept;
         constexpr operator base_managed_form_data*() noexcept;
         constexpr const base_managed_form_data* operator->() const noexcept;
         constexpr base_managed_form_data* operator->() noexcept;

         untyped_form_data_pointer& operator=(const untyped_form_data_pointer& other);
         untyped_form_data_pointer& operator=(untyped_form_data_pointer&& other) noexcept;

      public: // passkeyed
         static untyped_form_data_pointer _spawn(lu::passkey<form_stub>, form_stub& stub);
   };

   template<template<typename> typename FormDataTemplate>
   class form_data_pointer : public form_data_pointer_impl::base {
      public:
         using form_data_type = FormDataTemplate<managed_form_params>;
         static constexpr const form_type form_type = form_type_for_form_data_class_v<form_data_type>;

         static_assert(
            form_type != form_type::none,
            "You must include the header for the given form type template (and that header must properly associate the `form_type` constant with the template)."
         );

      protected:
         static form_stub* _type_check(form_stub* s);

      public:
         constexpr form_data_pointer() {}
         explicit form_data_pointer(const untyped_form_data_pointer& other);
         form_data_pointer(const form_data_pointer& other);
         constexpr form_data_pointer(form_data_pointer&& other) noexcept;

         // same-template assignment operators don't need the type-checking
         form_data_pointer& operator=(const form_data_pointer& other);
         form_data_pointer& operator=(form_data_pointer&& other);

         constexpr operator bool() const noexcept;
         constexpr bool operator!() const noexcept;
         constexpr operator const form_data_type*() const noexcept;
         constexpr operator form_data_type*() noexcept;
         constexpr const form_data_type* operator->() const noexcept;
         constexpr form_data_type* operator->() noexcept;
   };
}

#include "./form_data_pointer.inl"