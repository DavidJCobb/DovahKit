#pragma once
#include <atomic>
#include <cstdint>
#include <cstring>
#include <map>
#include "esp/base.h" // ESPGroupType
#include "helpers/bitset.h"
#include "helpers/multiheap.h"
#include "helpers/scoped_enum.h"

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

//
// Turns out, using a block allocator for Use Info is slower than using the 
// default allocation. Still, I'm retaining this in case it comes in handy 
// in the future.
//
#define COBB_ESP_BLOCK_ALLOCATE_USE_INFO 0

template<typename LoadedFormClass> class loaded_form_ptr {
   //
   // This is intended as a smart pointer not for the FormStub itself, but for the 
   // loaded form data, going THROUGH the FormStub.
   //
   // Suggested usage for when you have a FormStub, know the type of form it holds, 
   // and wish to load and use the form data:
   //
   //    auto form  = myFormStub.load();
   //    auto quest = form.ptr_cast<LoadedForms::Quest>();
   //    //
   //    // ...and then you can use (quest) if it were a LoadedForms::Quest*. Even 
   //    // without casting, you can use (form) as if it were a LoadedForms::Form*.
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
            if (fs->refcount == 0 && fs->can_unload_form()) {
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

//
// USE INFO
//
// Use Info serves two purposes: it is a useful thing to be able to display in the UI; and 
// it is needed in order to safely delete a form. If Form A refers to Form B, and we delete 
// Form B, then we must clear B's form ID from Form A; if we don't, and if we create a new 
// form using the former Form B's form ID, then Form A will now have a reference to a form 
// that it doesn't expect. As such, Form B must be aware of all inbound connections, incl-
// uding from Form A.
//
// Yet Form A must also have a list of all outbound connections, including to Form B. If 
// we delete Form A, then we must be able to tell Form B that Form A no longer refers to 
// it; otherwise, if we then delete Form B, it will try to update the non-existent Form A.
//
// So we must keep track of all connections between forms, bidirectionally.
//
SCOPE_ENUM(UseInfoFlags, enum UseInfoFlags : uint8_t {
   i_am_child_of  = 1,
   i_am_parent_of = 2,
});
struct UseInfoEntry {
   FormStub* other    = nullptr; // this can be nullptr, as in the case of dangling references between forms in a hand-edited user file
   uint32_t  refcount = 0;
   uint8_t   flags    = 0; // UseInfoFlags
   //
   UseInfoEntry() {}
   UseInfoEntry(FormStub* s) : other(s) {}
};
#if COBB_ESP_BLOCK_ALLOCATE_USE_INFO == 1
   typedef std::pair<const uint32_t, UseInfoEntry> UseInfoEntryPair;
   typedef cobb::multiheap_allocator<UseInfoEntryPair, 3200> UseInfoEntryAllocator;
   typedef std::map<uint32_t, UseInfoEntry, std::less<uint32_t>, UseInfoEntryAllocator> UseInfoList; // <formID, entry>
#else
   typedef std::map<uint32_t, UseInfoEntry> UseInfoList; // <formID, entry>
#endif

SCOPE_ENUM(FormStubFlags, enum FormStubFlags : uint8_t {
   is_edited    = 0x00000001,
   is_hardcoded = 0x00000002,
});
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
   using flags_type = std::underlying_type_t<FormStubFlags>;
   //
   public:
      ~FormStub();
      //
   protected:
      TESPluginFile* file   = nullptr;
      uint32_t       offset = 0; // offset of this form's record header within its owning file
      std::atomic<uint32_t> refcount = 0;
      void build_outbound_refs(TESPluginFileView*) noexcept;
      void send_inbound_refs() noexcept; // use my outbound ref data to add inbound refs to the forms I refer to
      void receive_inbound_ref(FormStub* inbound, flags_type flags = 0) noexcept;
      //
   public:
      GroupMetadata groupInfo;
      uint32_t      formID   = 0; // form ID (file-local)
      uint8_t       formType = 0;
      flags_type    flags = 0;
      // there will be 2 bytes of padding here
      std::string   editorID;
      LoadedForms::Form* form = nullptr; // don't access directly; use FormStub::load() to get a refcounted pointer
      UseInfoList   outbound; // other forms that this one refers to
      UseInfoList   inbound;  // other forms that refer to this one
      //
      loaded_form_ptr<LoadedForms::Form> load();
      //
      inline bool can_unload_form() const noexcept {
         if (this->is_edited())
            return false;
         if (this->is_hardcoded() && !this->file) // form is hardcoded and this FormStub is not an override
            return false;
         return true;
      }
      inline const char* get_editor_id() const noexcept { return this->editorID.c_str(); };
      inline uint32_t    get_refcount()  const noexcept { return this->refcount; };
      inline bool        refcount_is_maxed_out() const noexcept { return this->refcount == std::numeric_limits<uint32_t>::max(); }
      inline bool        is_edited()    const noexcept { return (bool)(this->flags & FormStubFlags::is_edited); };
      inline bool        is_hardcoded() const noexcept { return (bool)(this->flags & FormStubFlags::is_hardcoded); };
      void set_edited(bool v);
      //
      void get_source_filename(std::string& out) const noexcept;
      //
      void add_outbound_reference(uint32_t toFormID, flags_type flags = 0);
      //
      static void* operator new(std::size_t sz);
      static void operator delete(void* ptr, std::size_t sz);
};

typedef cobb::multiheap<FormStub, 16000> FormStubHeap;