#pragma once
#include <cstdint>
#include "helpers/bitset.h"

struct FormStub;
class TESForm;
class TESPluginFile;

template<typename LoadedFormClass> class loaded_form_ptr {
   //
   // This is intended as a smart pointer not for the FormStub itself, but for the 
   // loaded form data, going THROUGH the FormStub.
   //
   // Suggested usage for when you have a FormStub, know the type of form it holds, 
   // and wish to load and use the form data:
   //
   //    auto form  = myFormStub.load();
   //    auto quest = form.ptr_cast<TESQuest>();
   //    //
   //    // ...and then you can use (quest) if it were a TESQuest*. Even without 
   //    // casting, you can use (form) as if it were a TESForm*.
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
      loaded_form_ptr(FormStub* stub) : wrapped(stub) { this->_incRef(); };
      ~loaded_form_ptr() {
         this->_decRef();
         this->wrapped = nullptr;
      }

      operator bool() { return this->wrapped != nullptr && this->wrapped->form != nullptr; };
      operator LoadedFormClass* () const noexcept { return (LoadedFormClass*)this->wrapped->form; };
      LoadedFormClass* operator->() const noexcept { return (LoadedFormClass*)this->wrapped->form; };

      loaded_form_ptr<LoadedFormClass>& operator=(FormStub* stub) noexcept {
         this->_decRef();
         this->wrapped = stub;
         this->_incRef();
         return *this;
      }
      loaded_form_ptr<LoadedFormClass>& operator=(const loaded_form_ptr<LoadedFormClass>& other) noexcept {
         this->_decRef();
         this->wrapped = other.wrapped;
         this->_incRef();
         return *this;
      }

      template<typename OtherFormClass> loaded_form_ptr<OtherFormClass> ptr_cast() {
         return loaded_form_ptr<OtherFormClass>(this->wrapped);
      }
};

struct FormStub {
   template<typename LoadedFormClass> friend class loaded_form_ptr;
   friend TESPluginFile;
   //
   public:
      enum RefcountFlags {
         kRefcountMask  = 0x7FFFFFFF,
         kRefcountFlags = 0x80000000,
         //
         kRefcountFlag_Edited = 0x80000000, // if true, then keep the wrapped form in memory even if its refcount hits zero, until we save changes
      };
      //
      ~FormStub();
      //
   private:
      TESPluginFile* file = nullptr;
      uint32_t offset   = 0; // offset of this form's record header within its owning file
      uint32_t refcount = 0;
      char*    editorID = nullptr;
      //
      // TODO: When we begin to add the ability to edit things, we'll have to keep editor IDs consistent 
      // between FormStubs and their loaded forms... or give every loaded form a reference to its stub, 
      // and have them skip loading their own editor IDs since the stubs already loaded those.
      //
   public:
      uint32_t       formID   = 0; // form ID (file-local)
      uint8_t        formType = 0;
      // there will be 3 bytes of padding here
      TESForm*       form     = nullptr;

      loaded_form_ptr<TESForm> load();

      inline const char* get_editor_id() { return this->editorID; };
      inline uint32_t get_refcount() {
         return this->refcount & kRefcountMask;
      };
      inline bool is_edited() { return (bool)(this->refcount & kRefcountFlag_Edited); };
      void set_edited(bool v);

      static void* operator new(std::size_t sz);
      static void operator delete(void* ptr, std::size_t sz);

   private:
      char* allocate_editor_id(size_t length);
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
      void forceFreeAll(); // for debugging purposes ONLY; this WILL leave dangling pointers everywhere
};