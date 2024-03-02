#pragma once
#include "./known_script.h"

namespace dovahkit::subsystems::papyrus {
   class known_script_ptr {
      private:
         using raw_pointer_type = known_script*;

      private:
         known_script* target = nullptr;

         void _inc_ref() {
            if (this->target)
               this->target->_on_reference_gained({});
         }
         void _dec_ref_of(raw_pointer_type v) {
            if (v)
               v->_on_reference_lost({});
         }

      public:
         constexpr known_script_ptr() {}
         ~known_script_ptr() {
            auto* prior = this->target;
            this->target = nullptr;
            this->_dec_ref_of(prior);
         }

         known_script_ptr(const known_script_ptr& o) {
            this->target = o.target;
            this->_inc_ref();
         }
         known_script_ptr(known_script_ptr&& o) noexcept {
            this->target = o.target;
            o.target     = nullptr;
         }
         known_script_ptr(raw_pointer_type t) {
            this->target = t;
            this->_inc_ref();
         }

         known_script_ptr& operator=(const known_script_ptr& o) noexcept {
            return this->operator=(o.target);
         }
         known_script_ptr& operator=(known_script_ptr&& o) noexcept {
            std::swap(this->target, o.target);
            return *this;
         }
         known_script_ptr& operator=(raw_pointer_type v) {
            auto* prior = this->target;
            this->target = v;
            this->_dec_ref_of(prior);
            this->_inc_ref();
            return *this;
         }

         constexpr operator bool() const noexcept { return this->target != nullptr; };
         constexpr operator const raw_pointer_type() const noexcept { return this->target; };
         constexpr const raw_pointer_type operator->() const noexcept { return this->target; };

         constexpr const raw_pointer_type get() const noexcept { return this->target; }

         constexpr bool operator==(const known_script_ptr& o) const noexcept {
            return this->target == o.target;
         }
   };
}