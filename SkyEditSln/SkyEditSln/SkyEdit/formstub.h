#pragma once
#include <bitset>
#include <cstdint>

class TESPluginFile;
class TESForm;
class loaded_form_ptr;

struct FormStub {
   uint32_t   formID;
   TESForm*   form = nullptr;
   uint32_t   refcount = 0;
   bool       edited = false; // if true, then keep the wrapped form in memory even if its refcount hits zero, until we save changes
   TESPluginFile* file   = nullptr;
   uint32_t   offset = 0; // offset of this form's record header within its owning file

   loaded_form_ptr load();

   static void* operator new(std::size_t sz);
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

class FormStubHeap {
   public:
      typedef FormStub element_type;
      //
      inline static FormStubHeap& get() {
         static FormStubHeap instance;
         return instance;
      }
      //
   protected:
      static constexpr uint16_t ce_countPerBlock = 100;
      //
      struct Block;
      struct BlockInfo {
         Block*   prev = nullptr;
         Block*   next = nullptr;
         std::bitset<ce_countPerBlock> presence;
         uint16_t firstFree = 0;
         //
         BlockInfo() {
            memset(&this->presence, 0, sizeof(this->presence));
         };
      };
      struct Block {
         BlockInfo info;
         uint8_t   buffer[FormStubHeap::ce_countPerBlock * sizeof(FormStub)];
         //
         void* allocate();
      };
   public:
      void* allocate();
      void  free(void*);
      //
      Block* firstBlock = nullptr;
      //
      void dump();
};