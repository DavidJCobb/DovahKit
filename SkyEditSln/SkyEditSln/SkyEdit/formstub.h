#pragma once
#include <cstdint>

class TESPlugin;
class TESForm;
class loaded_form_ptr;

struct FormStub {
   uint32_t   formID;
   TESForm*   form = nullptr;
   uint32_t   refcount = 0;
   bool       edited = false; // if true, then keep the wrapped form in memory even if its refcount hits zero, until we save changes
   TESPlugin* file   = nullptr;
   uint32_t   offset = 0; // offset of this form's record header within its owning file

   loaded_form_ptr load();
};

class loaded_form_ptr {
   //
   // This is intended as a smart pointer not for the FormStub itself, but for the 
   // loaded form data, going THROUGH the FormStub.
   //
   private:
      FormStub* wrapped = nullptr;
      //
      inline void _incRef() {
         if (this->wrapped)
            this->wrapped->refcount++;
      }
      inline void _decRef() {
         if (this->wrapped)
            this->wrapped->refcount--;
      }
   public:
      loaded_form_ptr(FormStub*);
      ~loaded_form_ptr();

      operator bool() { return this->wrapped != nullptr && this->wrapped->form != nullptr; };
      operator TESForm*() const noexcept { return this->wrapped->form; };
      TESForm* operator->() const noexcept { return this->wrapped->form; };

      loaded_form_ptr& operator=(FormStub* stub) noexcept;
};