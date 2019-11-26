#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <forward_list>
#include <map>
#include <vector>
#include "esp/base.h" // ESPGroupType
#include "helpers/bitset.h"
#include "helpers/memory.h"
#include "helpers/wavl_tree.h"

class FormStub;
class LoadOrder;
class ThreadedUseInfoOutboundBuilder;
class TESPluginFile;
class TESPluginBaseReader;
class TESPluginFileView;
class TESPluginThreadedSimpleReader;
class TESPluginThreadedInteriorCellReader;
class TESPluginThreadedWorldspaceSubBlockReader;
namespace LoadedForms {
   class Form;
}

#define COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS 0
#define COBB_ESP_BLOCK_ALLOCATE_USE_INFO 0

#if COBB_ESP_BLOCK_ALLOCATE_MAP_PAIRS == 1
   typedef cobb::wavl_node<uint32_t, FormStub*> FormMapPair;
   class FormMapHeap : public cobb::multithreaded_block_allocator<FormMapPair, 16000, 8> {
      public:
         inline static FormMapHeap& get() {
            static FormMapHeap instance;
            return instance;
         }
   };
   class FormMapAllocator : public std::allocator<FormMapPair> {
      public:
         FormMapPair* allocate(size_type n, const FormMapPair* hint = nullptr) {
            if (n > 1)
               throw std::invalid_argument("Cannot allocate more than 1.");
            return (FormMapPair*)FormMapHeap::get().allocate();
         }
         void deallocate(FormMapPair* p, size_type n) {
            FormMapHeap::get().free((void*)p);
         }
         //
         bool operator==(const FormMapAllocator& right) { return this == &right; }
         bool operator!=(const FormMapAllocator& right) { return this != &right; }
   };

   typedef cobb::wavl_tree<uint32_t, FormStub*, FormMapAllocator> map_of_forms;
#else
   typedef std::map<uint32_t, FormStub*> map_of_forms;
#endif

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
         auto fs = this->wrapped;
         if (fs) {
            assert(!fs->refcount_is_maxed_out() && "FormStub refcount is already at maximum!");
            fs->refcount++;
         }
      }
      inline void _decRef() {
         auto fs = this->wrapped;
         if (fs) {
            assert(fs->get_refcount() != 0 && "FormStub refcount is already zero!");
            fs->refcount--;
            if ((fs->refcount & FormStub::kRefcountMask) == 0) {
               auto form = fs->form;
               if (form) {
                  delete form;
                  fs->form = nullptr;
               }
            }
         }
      }
   public:
      typedef LoadedFormClass wrapped_type;
      //
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

enum class GroupParentRelationship : uint8_t {
   Default,
   WorldDirectChild,
   WorldIndirectChild,
};
struct GroupMetadata { // sizeof == 0xC
   uint32_t     parentFormID = 0; // 0 for interior cells
   ESPGroupType groupType;
   union {
      uint32_t interior = 0;
      struct {
         int16_t x; // TODO: is it XXXXYYYY or YYYYXXXX? how does endianness affect it?
         int16_t y;
      } exterior;
   } cellBlock; // 0 for non-cells
   union {
      uint32_t interior = 0;
      struct {
         int16_t x; // TODO: is it XXXXYYYY or YYYYXXXX? how does endianness affect it?
         int16_t y;
      } exterior;
   } cellSubBlock; // 0 for non-cells
};

struct UseInfoEntry {
   FormStub* other    = nullptr; // this can be nullptr, as in the case of dangling references between forms in a hand-edited user file
   uint32_t  refcount = 0;
   //
   UseInfoEntry() {}
   UseInfoEntry(FormStub* s) : other(s) {}
};
#if COBB_ESP_BLOCK_ALLOCATE_USE_INFO == 1
   typedef cobb::wavl_node<uint32_t, UseInfoEntry> UseInfoEntryPair;
   class UseInfoEntryHeap : public cobb::multithreaded_block_allocator<UseInfoEntryPair, 3200, ESP_LOAD_TOTAL_THREADS> {
      public:
      inline static UseInfoEntryHeap& get() {
         static UseInfoEntryHeap instance;
         return instance;
      }
   };
   class UseInfoEntryAllocator : public std::allocator<UseInfoEntryPair> {
      public:
      UseInfoEntryPair* allocate(size_type n, const UseInfoEntryPair* hint = nullptr) {
         if (n > 1)
            throw std::invalid_argument("Cannot allocate more than 1.");
         return (UseInfoEntryPair*)UseInfoEntryHeap::get().allocate();
      }
      void deallocate(UseInfoEntryPair* p, size_type n) {
         UseInfoEntryHeap::get().free((void*)p);
      }
      //
      bool operator==(const UseInfoEntryAllocator& right) { return this == &right; }
      bool operator!=(const UseInfoEntryAllocator& right) { return this != &right; }
   };
   typedef cobb::wavl_tree<uint32_t, UseInfoEntry, UseInfoEntryAllocator> UseInfoList; // <formID, entry>
#else
   typedef std::map<uint32_t, UseInfoEntry> UseInfoList; // <formID, entry>
#endif

class FormStub {
   //
   // A class which represents a form, whether loaded or unloaded. Every FormStub contains 
   // information that can be used to load the form's data from a given ESP file on the fly. 
   // The owner of a FormStub is the TESPluginFile that produced it.
   //
   template<typename LoadedFormClass> friend class loaded_form_ptr;
   friend TESPluginFile;
   friend TESPluginBaseReader;
   friend LoadOrder;
   friend ThreadedUseInfoOutboundBuilder;
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
   protected:
      TESPluginFile* file   = nullptr;
      uint32_t       offset = 0; // offset of this form's record header within its owning file
      std::atomic<uint32_t> refcount = 0;
      char*          editorID = nullptr;
      void build_outbound_refs(TESPluginFileView*) noexcept;
      void send_inbound_refs() noexcept; // use my outbound ref data to add inbound refs to the forms I refer to
      void receive_inbound_ref(FormStub* inbound) noexcept;
      //
   public:
      GroupMetadata groupInfo;
      uint32_t      formID   = 0; // form ID (file-local)
      uint8_t       formType = 0;
      // there will be 3 bytes of padding here
      LoadedForms::Form* form = nullptr;
      UseInfoList   outbound; // other forms that this one refers to
      UseInfoList   inbound;  // other forms that refer to this one
      //
      loaded_form_ptr<LoadedForms::Form> load();
      //
      inline const char* get_editor_id() const noexcept { return this->editorID; };
      inline uint32_t    get_refcount()  const noexcept { return this->refcount & kRefcountMask; };
      inline bool        refcount_is_maxed_out() const noexcept { return this->refcount == kRefcountMask; }
      inline bool        is_edited() const noexcept { return (bool)(this->refcount & kRefcountFlag_Edited); };
      void set_edited(bool v);
      //
      void get_source_filename(std::string& out) const noexcept;
      //
      void add_outbound_reference(uint32_t toFormID);
      //
      static void* operator new(std::size_t sz);
      static void operator delete(void* ptr, std::size_t sz);
      //
   private:
      char* allocate_editor_id(size_t length);
};

class FormStubHeap : public cobb::multithreaded_block_allocator<FormStub, 16000, ESP_LOAD_TOTAL_THREADS> {
   //
   // NOTE: Keep the number of threads (third template argument) in synch with the 
   // number of threads used by TESPluginFile to load a file (or, if we decide to 
   // multi-thread the loading of multiple files, the total number of threads 
   // across all files being loaded concurrently).
   //
   public:
      inline static FormStubHeap& get() {
         static FormStubHeap instance;
         return instance;
      }
};