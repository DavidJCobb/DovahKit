#pragma once
#include "./form_data_pointer.h"

namespace dovah {
   namespace form_data_pointer_impl {
      constexpr const base_managed_form_data* base::_get_data() const noexcept {
         if (this->_stub)
            return this->_stub->loaded;
         return nullptr;
      }
      constexpr base_managed_form_data* base::_get_data() noexcept {
         if (this->_stub)
            return this->_stub->loaded;
         return nullptr;
      }
   }

   #pragma region untyped_form_data_pointer
      constexpr untyped_form_data_pointer::untyped_form_data_pointer(untyped_form_data_pointer&& other) noexcept {
         this->_stub = other._stub;
         other._stub = nullptr;
      }

      constexpr untyped_form_data_pointer::operator bool() const noexcept {
         return _get_data() != nullptr;
      };
      constexpr bool untyped_form_data_pointer::operator!() const noexcept {
         return _get_data() == nullptr;
      }
      constexpr untyped_form_data_pointer::operator const base_managed_form_data*() const noexcept {
         return _get_data();
      };
      constexpr untyped_form_data_pointer::operator base_managed_form_data*() noexcept {
         return _get_data();
      };
      constexpr const base_managed_form_data* untyped_form_data_pointer::operator->() const noexcept {
         return _get_data();
      };
      constexpr base_managed_form_data* untyped_form_data_pointer::operator->() noexcept {
         return _get_data();
      };
   #pragma endregion

   #pragma region form_data_pointer
      #pragma push_macro("TEMPLATE_PARAMS")
      #pragma push_macro("CLASS_NAME")
      #undef  TEMPLATE_PARAMS
      #define TEMPLATE_PARAMS template<template<typename> typename FormDataTemplate>
      #undef  CLASS_NAME
      #define CLASS_NAME form_data_pointer<FormDataTemplate>
      
      TEMPLATE_PARAMS
      /*static*/ form_stub* CLASS_NAME::_type_check(form_stub* s) {
         if (s) {
            if constexpr (form_type == form_type::reference) {
               if (!utils::form_types::is_ref(s->form_type))
                  s = nullptr;
            } else {
               if (s->form_type != form_type)
                  s = nullptr;
            }
         }
         return s;
      }

      TEMPLATE_PARAMS
      /*explicit*/ CLASS_NAME::form_data_pointer(const untyped_form_data_pointer& other) {
         this->_stub = _type_check(other._stub);
         this->_inc();
      }
      TEMPLATE_PARAMS
      CLASS_NAME::form_data_pointer(const CLASS_NAME& other) {
         this->_stub = other._stub;
         this->_inc();
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::form_data_pointer(CLASS_NAME&& other) noexcept {
         this->_stub = other._stub;
         other._stub = nullptr;
      }

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::operator bool() const noexcept {
         return _get_data() != nullptr;
      }
      TEMPLATE_PARAMS
      constexpr bool CLASS_NAME::operator!() const noexcept {
         return _get_data() == nullptr;
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::operator const form_data_type* () const noexcept {
         return (const form_data_type*)_get_data();
      }
      TEMPLATE_PARAMS
      constexpr CLASS_NAME::operator form_data_type* () noexcept {
         return (form_data_type*)_get_data();
      }
      TEMPLATE_PARAMS
      constexpr const typename CLASS_NAME::form_data_type* CLASS_NAME::operator->() const noexcept {
         return (const form_data_type*)_get_data();
      }
      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::form_data_type* CLASS_NAME::operator->() noexcept {
         return (form_data_type*)_get_data();
      }

      TEMPLATE_PARAMS
      CLASS_NAME& CLASS_NAME::operator=(const CLASS_NAME& other) {
         if (this == &other)
            return *this;
         this->_dec();
         this->_stub = other._stub;
         this->_inc();
         return *this;
      }
      TEMPLATE_PARAMS
      CLASS_NAME& CLASS_NAME::operator=(CLASS_NAME&& other) {
         if (this->_stub != other._stub) {
            this->_dec();
            this->_stub = other._stub;
         }
         other._stub = nullptr;
         return *this;
      }

      #undef  CLASS_NAME
      #undef  TEMPLATE_PARAMS
      #pragma pop_macro("CLASS_NAME")
      #pragma pop_macro("TEMPLATE_PARAMS")
   #pragma endregion
}