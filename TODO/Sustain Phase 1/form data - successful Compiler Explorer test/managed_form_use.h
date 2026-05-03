#pragma once
namespace dovah {
   struct base_managed_form_data;
   class  form_stub;
}

namespace dovah {
   // dummy definition for compiler explorer tests
   class managed_form_use {
      public:
         form_stub* value = nullptr;

      public:
         constexpr form_stub* get() const noexcept {
            return this->value;
         }
         void set(base_managed_form_data& owner, form_stub* v) {
            this->value = v;
         }

         constexpr form_stub* operator->() const noexcept {
            return this->value;
         }
         constexpr explicit operator bool() const noexcept {
            return this->value != nullptr;
         }
   };
}