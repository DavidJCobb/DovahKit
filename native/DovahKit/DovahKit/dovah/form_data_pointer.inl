#pragma once
#include "./form_data_pointer.h"

namespace dovah {
   #pragma region untyped_form_data_pointer
      constexpr untyped_form_data_pointer::untyped_form_data_pointer(untyped_form_data_pointer&& other) noexcept {
         this->_stub = other._stub;
         other._stub = nullptr;
      }

      constexpr untyped_form_data_pointer::operator bool() const noexcept {
         return this->_stub != nullptr && this->_stub->loaded != nullptr;
      };
      constexpr bool untyped_form_data_pointer::operator!() const noexcept {
         return this->_stub == nullptr || this->_stub->loaded == nullptr;
      }
      constexpr untyped_form_data_pointer::operator base_managed_form_data*() const noexcept {
         return this->_stub ? this->_stub->loaded : nullptr;
      };
      constexpr base_managed_form_data* untyped_form_data_pointer::operator->() const noexcept {
         return this->_stub ? this->_stub->loaded : nullptr;
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
      static form_stub* CLASS_NAME::_type_check(form_stub* s) {
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

      TEMPLATE_PARAMS
      constexpr CLASS_NAME::operator form_data_type* () const noexcept {
         return this->_stub ? (form_data_type*)this->_stub->loaded : nullptr;
      };
      TEMPLATE_PARAMS
      constexpr typename CLASS_NAME::form_data_type* CLASS_NAME::operator->() const noexcept {
         return this->_stub ? (form_data_type*)this->_stub->loaded : nullptr;
      };

      #undef  CLASS_NAME
      #undef  TEMPLATE_PARAMS
      #pragma pop_macro("CLASS_NAME")
      #pragma pop_macro("TEMPLATE_PARAMS")
   #pragma endregion
}