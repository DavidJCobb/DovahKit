#pragma once
#include <cstdint>
#include "helpers/bitset.h"

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
   //
   // A custom allocator for FormStub which allocates in blocks (currently 100 at a time), 
   // to reduce memory fragmentation and overhead (i.e. every heap allocation has to track 
   // the size allocated and some other metadata; there's no point in doing that for each 
   // individual FormStub).
   //
   // NOTES:
   //
   //  - When creating stubs only for QUST forms from Skyrim.esm as a test, the load process 
   //    takes about 30ms with default allocation. When using this allocator with my custom 
   //    bitset class, it also takes 30ms. When using this allocator with std::bitset, it 
   //    takes 100ms.
   //
   //     - Further testing reveals that most of the slowdown comes from std::bitset not 
   //       having an equivalent to cobb::bitset::find_first_clear, forcing us to use 
   //       a for-loop to go over each individual bit. My find_first_clear function is a 
   //       bit more optimal, checking entire uint32_t chunks of the bitmask at a time. 
   //       When using cobb::bitset without using the find_first_clear method, the load 
   //       process takes 80ms on average.
   //
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
         cobb::bitset<ce_countPerBlock> presence;
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