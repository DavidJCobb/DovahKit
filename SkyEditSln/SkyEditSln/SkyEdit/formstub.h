#pragma once
#include <cstdint>
#include <cstring>
#include "helpers/bitset.h"
#include "helpers/memory.h"

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

struct FormStubHeapPrinter : public cobb::block_allocator_debug_printer {
   virtual void forBlock(uint32_t index) {};
   virtual void forElement(void* element) {
      FormStub* stub = (FormStub*)element;
      auto id = stub->get_editor_id();
      if (id)
         this->editorIDBytes += strlen(id) + 1;
   }
   virtual void printExtraStats() {
      printf(" - %d bytes' worth of editor ID text held elsewhere\n", this->editorIDBytes);
   }

   uint32_t editorIDBytes = 0;
};
class FormStubHeap : public cobb::block_allocator<FormStub, 16000> {
   /*
   PERF STATS - OCTOBER 19 2019
    - Skyrim.esm
    - Loading ALL forms in ALL GRUPs including nested groups
    - FormStubs only
    - No group-related metadata retained; just the stub

   84 seconds with a block size of 400
   63 seconds with a block size of 800
	     110376 bytes overhead
	   22892800 bytes allocated
	   22881068 bytes in use
	     918342 editor ID bytes
   56 seconds with a block size of 1600
	     817181 slots used out of 817600
	     106288 bytes overhead
	   22892800 bytes allocated
	   22881068 bytes in use
	     918342 editor ID bytes
   48 seconds with a block size of 16000
	     817181 slots used out of 832000
	     104416 bytes overhead
	   23296000 bytes allocated
	   22881068 bytes in use
	     918342 editor ID bytes
   
   Bear in mind: the block size applies to ALL FormStub instances 
   across ALL files. Increasing it will have diminishing returns; 
   the way to get a *real* performance improvement is to use 
   multi-threaded file loading for all top-level GRUPs with form 
   types that can't have nested GRUPs (though we need to be 
   careful about redundant GRUPs e.g. two ACTI groups).
   */
   public:
      inline static FormStubHeap& get() {
         static FormStubHeap instance;
         return instance;
      }
};