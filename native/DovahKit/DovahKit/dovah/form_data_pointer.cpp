#include "./form_data_pointer.h"
#include <cassert>

namespace dovah {
   #pragma region untyped_form_data_pointer
      void untyped_form_data_pointer::_inc() {
         if (auto* s = this->_stub) {
            ++s->refcount;
         }
      }
      void untyped_form_data_pointer::_dec() {
         if (!this->_stub)
            return;
         auto* s = this->_stub;
         assert(s->refcount != 0 && "refcount is already zero!");
         --s->refcount;
         if (s->refcount == 0)
            s->_unload_form();
      }

      untyped_form_data_pointer::untyped_form_data_pointer(const untyped_form_data_pointer& other) {
         this->_stub = other._stub;
         this->_inc();
      }
      untyped_form_data_pointer::~untyped_form_data_pointer() {
         this->_dec();
         this->_stub = nullptr;
      }

      untyped_form_data_pointer& untyped_form_data_pointer::operator=(const untyped_form_data_pointer& other) {
         if (this == &other)
            return *this;
         this->_dec();
         this->_stub = other._stub;
         this->_inc();
         return *this;
      }
      untyped_form_data_pointer& untyped_form_data_pointer::operator=(untyped_form_data_pointer&& other) noexcept {
         if (this->_stub != other._stub) {
            this->_dec();
            this->_stub = other._stub;
         }
         other._stub = nullptr;
         return *this;
      }

      /*static*/ untyped_form_data_pointer untyped_form_data_pointer::_spawn(lu::passkey<form_stub>, form_stub& stub) {
         untyped_form_data_pointer v;
         v._stub = &stub;
         v._inc();
         return v;
      }
   #pragma endregion
}