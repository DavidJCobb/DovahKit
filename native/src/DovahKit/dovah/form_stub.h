#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#include "core.h"

namespace dovah {
   class  file_load_order;
   struct tes_file_record_header;
   namespace loaded_forms {
      class Form;
   }
   namespace tes_file_reading {
      class basic_reader;
      class file_loader;
      class threaded_load_order_use_info_builder;
   }
   namespace tes_file_writing {
      class file_writer;
   }

   class form_stub;

   template<typename loaded_form_t> class loaded_form_ptr {
      //
      // This is intended as a smart pointer not for the FormStub itself, but for the 
      // loaded form data, going *through* the FormStub.
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
      protected:
         form_stub* wrapped = nullptr;
         inline void _inc() {
            if (auto fs = this->wrapped) {
               assert(!fs->refcount_is_maxed_out() && "form_stub refcount is already at maximum!");
               ++fs->refcount;
            }
         }
         inline void _dec() {
            if (auto fs = this->wrapped) {
               assert(fs->get_refcount() != 0 && "form_stub refcount is already zero!");
               --fs->refcount;
               if (fs->refcount == 0)
                  fs->_unload_form();
            }
         }
      public:
         using wrapped_type = loaded_form_t;
         //
         loaded_form_ptr() {}
         loaded_form_ptr(form_stub* stub) : wrapped(stub) { this->_inc(); };
         template<typename other_form_t> loaded_form_ptr(loaded_form_ptr<other_form_t>&& other) {
            this->wrapped = other.wrapped;
            other.wrapped = nullptr;
         }
         ~loaded_form_ptr() {
            this->_dec();
            this->wrapped = nullptr;
         }

         operator bool() { return this->wrapped != nullptr && this->wrapped->form != nullptr; };
         operator loaded_form_t*() const noexcept { return (loaded_form_t*)this->wrapped->form; };
         loaded_form_t* operator->() const noexcept { return (loaded_form_t*)this->wrapped->form; };

         loaded_form_ptr<loaded_form_t>& operator=(form_stub* stub) noexcept {
            this->_dec();
            this->wrapped = stub;
            this->_inc();
            return *this;
         }
         loaded_form_ptr<loaded_form_t>& operator=(const loaded_form_ptr<loaded_form_t>& other) noexcept {
            this->_dec();
            this->wrapped = other.wrapped;
            this->_inc();
            return *this;
         }
         loaded_form_ptr<loaded_form_t>& operator=(loaded_form_ptr<loaded_form_t>&& other) noexcept {
            this->_dec();
            this->wrapped = other.wrapped;
            other.wrapped = nullptr;
            return *this;
         }

         template<typename other_loaded_form_t> loaded_form_ptr<other_loaded_form_t> ptr_cast() {
            return loaded_form_ptr<other_loaded_form_t>(this->wrapped);
         }
   };

   struct group_stub {
      bare_form_id_t parentFormID = 0; // 0 for interior cells
      int8_t  type  = -1; // init to tes_file_group_type::none
      // we have 3 padding bytes here
      int32_t gridX = 0; // exterior cells only; taken from CELL/XCLC. size of these fields is significant; CELL load/save code uses these fields directly.
      int32_t gridY = 0; // exterior cells only; taken from CELL/XCLC.
   };

   #pragma region Use Info
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
   // ---------------------------------------------------------------------------------------
   //
   // NOTES:
   //
   //  - Child forms have outbound references to their parent forms, so parent forms have 
   //    inbound references from their child forms.
   //
   struct use_info_entry {
      enum class outbound_type {
         i_am_reference_of,
      };
      struct flag {
         flag() = delete;
         enum type : uint8_t {
            i_am_child_of    = 0x01, // (this) is the child of (other)
            i_am_parent_of   = 0x02, // (other) is the child of (this)
            object_reference = 0x04, // one of the forms is the other's base form; check form types to know which is which
            dialogue         = 0x08, // DIAL/QNAM: topic owning quest; INFO/PNAM: info linked list
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      form_stub* other    = nullptr;
      uint32_t   refcount = 0;
      flags_t    flags    = 0;
      //
      static flags_t invert_flags(flags_t);
   };
   using use_info_list = std::map<bare_form_id_t, use_info_entry>;
   #pragma endregion

   #pragma region Interfaces for working with form stubs in specific contexts
   class form_stub_use_info_builder {
      friend class form_stub;
      protected:
         form_stub& _stub;
         bool       _is_final_file = false;
         //
         form_stub_use_info_builder(form_stub& s) : _stub(s) {}
         //
      public:
         void add_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags = 0);
         void add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags = 0);
         //
         inline const form_stub* stub() const noexcept { return &this->_stub; }
         inline bool is_final_file() const noexcept { return this->_is_final_file; }
         //
         // Generic state information, provided for form types that need it:
         form_id_t extra_form_ids[10];
   };
   #pragma endregion

   class form_stub {
      //
      // A class which represents a form, whether loaded or unloaded. Every FormStub contains 
      // information that can be used to load the form's data from a given ESP file on the fly. 
      // The owner of a FormStub is the TESPluginFile that produced it.
      //
      template<typename loaded_form_t> friend class loaded_form_ptr;
      friend file_load_order;
      friend tes_file_reading::file_or_file_part_loader;
      friend tes_file_reading::threaded_load_order_use_info_builder;
      friend tes_file_writing::file_writer;
      friend form_stub_use_info_builder;
      //
      public:
         form_stub();
         ~form_stub();
         //
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               //
               // (is_edited)
               // Indicates that changes have been made to this form_stub's loaded data, and so that data should not 
               // be allowed to unload. When changes are saved, the flag will be cleared, and if at that time the 
               // loaded form data is not in use, it will be unloaded.
               //
               is_edited = 0x01,
               //
               // (is_hardcoded)
               // Indicates that this form_stub represents a form that is hardcoded into Skyrim's game engine. These 
               // forms will always have form IDs below 0x00000800. If the form has not been overridden by one of 
               // the loaded files, then the stub's (file) pointer will be (file_load_order::hardcoded_forms_file).
               //
               is_hardcoded = 0x02,
               //
               // (missing_coordinates) // only for exterior CELLs
               // Indicates that an exterior CELL was missing its XCLC subrecord. These default to grid coordinates 
               // (0, 0), but if another cell exists at those coordinates, then the file will crash the Creation Kit 
               // while loading. Official tools shouldn't ever save an exterior cell without XCLC, and xEdit won't 
               // allow you to remove it (but also won't bother to add it if it's missing), so you'd have to hex-edit 
               // a file to even get this to happen.
               //
               missing_coordinates = 0x04,
               //
               // (flagged_as_deleted)
               // Indicates that the form is flagged as deleted.
               //
               flagged_as_deleted = 0x08,
               //
               // (has_multiple_source_files)
               // Indicates that the (file)/(files) union is (files).
               //
               has_multiple_source_files = 0x10,
            };
         };
         using flags_t      = std::underlying_type_t<flag::type>;
         using owner_file_t = tes_file_reading::file_loader;
         //
         struct file_data {
            owner_file_t* pointer = nullptr;
            uint32_t      offset  = 0;
            //
            operator bool() const noexcept { return this->pointer != nullptr; }
         };
         struct file_data_list {
            file_data* entries = nullptr;
            uint16_t   count   = 0;
         };
         //
      protected:
         union { // a stub must ALWAYS have at least one source file, unless it's literally in the middle of being loaded from that source file.
            file_data      file;
            file_data_list files;
         };
         std::atomic<uint32_t> refcount = 0;
         void build_outbound_refs(tes_file_reading::basic_reader&) noexcept;
         void send_inbound_refs() noexcept; // use my outbound ref data to add inbound refs to the forms I refer to
         void receive_inbound_ref(form_stub* inbound, use_info_entry::flags_t flags = 0) noexcept;
         //
         file_load_order& form_stub::_get_load_order() const noexcept;
         loaded_form_ptr<loaded_forms::Form> _load(bool force = false);
         void _unload_form();
         //
         void _add_file(owner_file_t&, uint32_t offset);
         void _set_active_file_data(owner_file_t&, uint32_t offset);
         void _get_source_file_list(file_data*& out_arr, uint16_t& out_count) const noexcept;
         void _adopt_source_file_list(const form_stub* other);
         void _set_source_file_list(const std::vector<file_data>&);
         //
         void _add_one_way_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags = 0);
         void _add_one_way_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags = 0);
         //
      public:
         group_stub    groupInfo;
         uint32_t      formID   = 0; // form ID (file-local)
         uint8_t       formType = 0;
         flags_t       flags    = 0; // when there are getters/setters for these, use those instead of editing the mask directly
         // there will be 2 bytes of padding here
         std::string   editorID;
         loaded_forms::Form* form = nullptr; // don't access directly; use FormStub::load() to get a refcounted pointer
         use_info_list outbound; // other forms that this one refers to. flags describe (this), the form that is referring.
         use_info_list inbound;  // other forms that refer to this one.  flags describe (this), the form that is referred to.
         //
         loaded_form_ptr<loaded_forms::Form> load() { return this->_load(); }
         loaded_form_ptr<loaded_forms::Form> get_content_if_loaded(); // returns a pointer to (this->form) only if it's already loaded
         //
         bool fetch_record_header(tes_file_record_header& out, uint32_t& out_record_decompressed_size, int16_t source_file_index = -1) const noexcept;
         //
         #pragma region Source file member functions
         const file_data* get_source_file_info(int16_t file_index = -1) const noexcept;
         bool file_list_includes(const owner_file_t*) const noexcept;
         bool has_source_files() const noexcept;
         inline bool has_multiple_source_files() const noexcept { return this->flags & flag::has_multiple_source_files; }
         uint16_t source_file_count() const noexcept;
         int16_t index_of_file(const owner_file_t*) const noexcept;
         owner_file_t* get_file_at_index(int16_t) const noexcept;
         //
         uint32_t get_file_offset(int16_t file_index = -1) const noexcept;
         #pragma endregion
         //
         bool can_unload_form() const noexcept;
         inline const char* get_editor_id() const noexcept { return this->editorID.c_str(); };
         inline uint32_t    get_refcount()  const noexcept { return this->refcount; };
         inline bool        refcount_is_maxed_out() const noexcept { return this->refcount == std::numeric_limits<uint32_t>::max(); }
         inline bool        is_deleted()   const noexcept { return (bool)(this->flags & flag::flagged_as_deleted); };
         inline bool        is_edited()    const noexcept { return (bool)(this->flags & flag::is_edited); };
         inline bool        is_hardcoded() const noexcept { return (bool)(this->flags & flag::is_hardcoded); };
         bool is_edited_or_in_active_file() const noexcept;
         bool is_injected() const noexcept;
         bool is_non_overridden_hardcoded_form() const noexcept;
         bool is_none_stub() const noexcept;
         void set_edited(bool v);

         form_stub* get_parent_form() const noexcept; // searches Use Info for a form with the same ID as the parent form
         bool has_child_forms() const noexcept;
         bool has_child_forms_of_group(uint8_t) const noexcept;
         bool is_any_descendant_form_edited() const noexcept;
         bool does_descendant_form_need_save() const noexcept;
         bool needs_save() const noexcept;
         //
         bool is_exterior_cell() const noexcept; // checks whether we have a parent form. can't check cell flags, since the form may not be loaded
         uint32_t get_cell_block() const noexcept;
         uint32_t get_cell_sub_block() const noexcept;
         //
         void revoke_outbound_reference(form_stub* target, use_info_entry::flags_t flags = 0);
         void revoke_all_outbound_references_to(form_stub* target);
         void replace_outbound_reference(bare_form_id_t old, form_stub* changeTo, use_info_entry::flags_t flags = 0);
         void replace_outbound_reference(bare_form_id_t old, bare_form_id_t change_to, use_info_entry::flags_t flags = 0);

         void sever_all_outbound_references(); // works bidirectionally; use when deleting a form
         
         static void* operator new(std::size_t sz);
         static void operator delete(void* ptr, std::size_t sz);
   };
}