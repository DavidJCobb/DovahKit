#pragma once
#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <map>
#include <string>
#include <type_traits>
#include <vector>
#include "helpers/multiheap.h"
#include "./core.h"
#include "./form_types.h"
#include "./files/common.h"

#include "./form_stubs/passkeys/build_use_info_during_load.h"
#include "./form_stubs/passkeys/force_form_load.h"

namespace dovah {
   class  file_load_order;
   struct tes_file_record_header;
   namespace loaded_forms {
      class Form;
   }
   namespace load_order_interfaces {
      class form_load;
   }
   namespace load_order_processes {
      class file_save;
   }
   namespace tes_file_reading {
      class basic_reader;
      class file_or_file_part_loader;
      class file_loader;
      class record;
   }

   class form_stub;
   class form_stub_addenda;

   template<typename loaded_form_t> class loaded_form_ptr;

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
            parent_child    = 0x01, // the user-form is a child of the used-form
            //
            // The next flags are useful for unique and high-importance relationships between 
            // specific forms. These must be relationships that can only exist once; for example, 
            // a REFR can only have one base form. If the relevant (form_reference_t) is altered, 
            // the flag will be removed.
            //
            // If two relationships can be outbound from the same form but are mutually exclusive, 
            // that alone is not enough to distinguish them, because a form with malformed data 
            // could be loaded. For example, DIAL/BNAM and DIAL/QNAM are mutually exclusive by 
            // virtue of involving different form types, but a file with ill-formed data could 
            // contain a DIAL that points both subrecords at the same form, and so using the same 
            // flag for both subrecords could in that situation cause use info mismanagement should 
            // either subrecord be altered after load.
            //
            object_reference = 0x02, // REFR/NAME: the user-form is a reference and the used-form is its base form
            dialogue_branch  = 0x04, // DIAL/BNAM
            dialogue_quest   = 0x08, // DIAL/QNAM and DLBR/QNAM
            water_acti_type  = 0x10, // ACTI/WNAM: a water activator's water type
         };
      };
      using flags_t = std::underlying_type_t<flag::type>;
      //
      form_stub* other    = nullptr;
      uint32_t   refcount = 0;
      flags_t    flags    = 0;
   };
   using use_info_list = std::map<
      bare_form_id_t,
      use_info_entry,
      std::less<bare_form_id_t>,
      cobb::multiheap_allocator<std::pair<const bare_form_id_t, use_info_entry>, 64000>
   >;
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
      friend load_order_processes::file_save;
      //
      public:
         form_stub();
         ~form_stub();
         
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
               // (has_multiple_source_files)
               // Indicates that the (file)/(files) union is (files).
               //
               has_multiple_source_files = 0x04,
            };
         };
         using flags_t      = std::underlying_type_t<flag::type>;

         using owner_file_t = tes_file_reading::file_loader;
         
         struct file_data {
            owner_file_t* pointer = nullptr;
            uint32_t      offset  = 0;
            uint32_t      flags   = 0; // record flags
            //
            operator bool() const noexcept { return this->pointer != nullptr; }
         };
         struct file_data_list {
            file_data* entries = nullptr;
            int16_t    count   = 0; // values below 0 are illegal
         };

         using custom_parse_functor_type = std::function<void(form_stub&, tes_file_reading::record&, load_order_interfaces::form_load&)>;
         
      protected:
         union { // a stub must ALWAYS have at least one source file, unless it's literally in the middle of being loaded from that source file.
            file_data      file;
            file_data_list files;
         };
         std::atomic<uint32_t> refcount = 0;
         
         file_load_order& _get_load_order() const noexcept;

         // An internal-only version of (form_stub::load) which can be used by (file_load_order) during the file load process.
         loaded_form_ptr<loaded_forms::Form> _load(bool even_if_during_file_load = false);

         // An internal-only version of (form_stub::do_custom_parse) which can be used by (file_load_order) during the file load process.
         void _do_custom_parse_impl(tes_file_reading::basic_reader*, custom_parse_functor_type functor) noexcept;

         void _unload_form();
         
         void _add_file(owner_file_t&, uint32_t offset, uint32_t record_flags = 0);
         void _set_source_file_offset(owner_file_t&, uint32_t offset);
         void _modify_source_file_record_flags(owner_file_t&, uint32_t mask, bool clear_or_set);
         void _get_source_file_list(file_data*& out_arr, uint16_t& out_count) const noexcept;
         void _adopt_source_file_list(const form_stub* other); // prepends
         void _set_source_file_list(const std::vector<file_data>&);
         //
         file_data* _get_source_file_info(int16_t file_index = -1) const noexcept; // defined this way so code internal to form_stub can actually modify the info in question
         //
         
      public:
         #pragma region Passkeyed methods
         void build_outbound_refs(form_stub_passkeys::build_use_info_during_load, tes_file_reading::basic_reader&) noexcept;
         void send_inbound_refs(form_stub_passkeys::build_use_info_during_load) noexcept; // use my outbound ref data to add inbound refs to the forms I refer to
         void receive_inbound_ref(form_stub_passkeys::build_use_info_during_load, form_stub* inbound, uint32_t refcount, use_info_entry::flags_t flags = 0) noexcept;

         void _add_one_way_outbound_reference(form_stub_passkeys::build_use_info_during_load, form_stub* to_stub, use_info_entry::flags_t flags = 0);
         void _add_one_way_outbound_reference(form_stub_passkeys::build_use_info_during_load, uint32_t toFormID, use_info_entry::flags_t flags = 0);
         void _set_parent_form_one_way(form_stub_passkeys::build_use_info_during_load, form_stub* parent);

         void _insert_child_topic_info(form_stub_passkeys::build_use_info_during_load, form_stub& info, size_t at = std::string::npos); // inserts (info) into the addendum info list, without form type checks or managing parenthood
         void _remove_child_topic_info(form_stub_passkeys::build_use_info_during_load, form_stub& info, bool loading); // removes (info) from the addendum info list, without form type checks or managing parenthood

         loaded_form_ptr<loaded_forms::Form> load_even_if_unsafe(form_stub_passkeys::force_form_load);

         void do_custom_parse_during_serialization(
            form_stub_passkeys::force_form_load,
            tes_file_reading::basic_reader* reader,
            custom_parse_functor_type loader
         ) noexcept;
         #pragma endregion

      public:
         form_stub_addenda* addenda = nullptr;
         bare_form_id_t formID    = 0; // form ID (file-local during load; global once stored)
         enum form_type form_type = form_type::none;
         flags_t        flags     = 0; // when there are getters/setters for these, use those instead of editing the mask directly
         // there will be 2 bytes of padding here
         std::string    editorID;
         loaded_forms::Form* form         = nullptr; // don't access directly; use FormStub::load() to get a refcounted pointer
         loaded_forms::Form* working_copy = nullptr;
         use_info_list  outbound; // other forms that this one refers to. flags describe (this), the form that is referring.
         use_info_list  inbound;  // other forms that refer to this one.  flags describe (this), the form that is referred to.
         
         //
         // Loads a form, if possible. This accesses the form's original source files and is not thread-safe; 
         // loading multiple forms concurrently risks accessing the same file concurrently, which will cause 
         // corruption or crashes.
         //
         loaded_form_ptr<loaded_forms::Form> load();
         loaded_form_ptr<loaded_forms::Form> get_content_if_loaded(); // returns a pointer to (this->form) only if it's already loaded
         
         bool fetch_record_header(tes_file_record_header& out, uint32_t& out_record_decompressed_size, int16_t source_file_index = -1) const noexcept;

         //
         // A service provided to allow frontends to rapidly extract specific subrecords that are of interest, 
         // without having to fully load a form. This has the same thread-safety issues as (load) unless you 
         // pass your own reader object. If you do pass a reader, it must be blank, so that it can be "adopted" 
         // by the form's source files and then eventually severed from them.
         //
         void do_custom_parse(tes_file_reading::basic_reader* reader, custom_parse_functor_type loader) noexcept;
         
         #pragma region Source file member functions
         const file_data* get_source_file_info(int16_t file_index = -1) const noexcept;
         bool file_list_includes(const owner_file_t*) const noexcept;
         bool has_source_files() const noexcept;
         inline bool has_multiple_source_files() const noexcept { return this->flags & flag::has_multiple_source_files; }
         int16_t source_file_count() const noexcept;
         int16_t index_of_file(const owner_file_t*) const noexcept;
         owner_file_t* get_file_at_index(int16_t) const noexcept;
         //
         uint32_t get_file_offset(int16_t file_index = -1) const noexcept;
         #pragma endregion
         
         bool can_unload_form() const noexcept;
         file_load_order& get_owning_load_order() const noexcept;
         inline const char* get_editor_id() const noexcept { return this->editorID.c_str(); };
         inline uint32_t    get_refcount()  const noexcept { return this->refcount; };
         inline bool        refcount_is_maxed_out() const noexcept { return this->refcount == std::numeric_limits<uint32_t>::max(); }
         inline bool        is_deleted()   const noexcept { return this->test_record_flags(tes_file_record_header::flag::deleted); };
         inline bool        is_edited()    const noexcept { return (bool)(this->flags & flag::is_edited); };
         inline bool        is_hardcoded() const noexcept { return (bool)(this->flags & flag::is_hardcoded); };
         bool is_edited_or_in_active_file() const noexcept;
         bool is_injected() const noexcept;
         bool is_non_overridden_hardcoded_form() const noexcept;
         bool is_none_stub() const noexcept;
         void set_edited(bool v);

         uint32_t get_record_flags() const noexcept;
         bool test_record_flags(uint32_t mask) const noexcept;
         void edit_record_flags(uint32_t mask, bool clear_or_set) noexcept; // also sets the form as edited
         bool test_record_flags_for_file(uint32_t mask, int16_t file_index) const noexcept;
         bool test_record_flags_for_file(uint32_t mask, owner_file_t&) const noexcept;

         #pragma region Addenda helper functions
         form_stub_addenda& get_or_create_addenda() noexcept;
         //
         bool get_grid_coordinates(int32_t& x, int32_t& y) const noexcept;
         size_t child_info_count() const noexcept;
         size_t index_of_child_info(form_stub& info) const noexcept; // search a topic's list of infos; returns std::string::npos if no match
         void insert_child_topic_info(form_stub& info, size_t at = std::string::npos); // also forces (info)'s parent to (this) if that isn't already the case
         void remove_child_topic_info(form_stub& info); // also orphans (info)
         //
         void sever_addenda_references_to(form_stub& other);
         #pragma endregion

         #pragma region Parenthood functions
         form_stub* get_parent_form() const noexcept; // searches Use Info

         bool has_child_forms() const noexcept;

         bool is_parent_form_of(form_stub&) const noexcept;

         //
         // Orphan the form from its parent form. If the form is a topic info, it will also be removed from 
         // the parent form's addendum info list.
         //
         // Note that cells can only tell whether they're exteriors or interiors based on the presence or 
         // absence of a parent form, so, uh,... don't run this on cells.
         //
         void orphan();

         //
         // Calls (orphan) and then changes the parent form. If this form is a topic info and the new parent 
         // is a topic, then this form will be added to the new parent's addendum info list.
         //
         // If this form is a cell, then the same caveats apply as with (orphan).
         //
         void set_parent_form(form_stub* p) noexcept;
         #pragma endregion

         [[nodiscard]] bool is_any_descendant_form_edited() const noexcept;
         [[nodiscard]] bool does_descendant_form_need_save() const noexcept;
         [[nodiscard]] bool needs_save() const noexcept;
         //
         [[nodiscard]] bool is_exterior_cell() const noexcept; // checks whether we have a parent form. can't check cell flags, since the form may not be loaded
         [[nodiscard]] uint32_t get_cell_block() const noexcept;
         [[nodiscard]] uint32_t get_cell_sub_block() const noexcept;
         //
         [[nodiscard]] form_stub* get_outbound_use_with_flag(use_info_entry::flags_t) const noexcept;
         
         #pragma region Functions for modifying use info
         void revoke_outbound_reference(form_stub* target, use_info_entry::flags_t flags = 0);
         void revoke_all_outbound_references_to(form_stub* target);
         void replace_outbound_reference(bare_form_id_t old, form_stub* changeTo, use_info_entry::flags_t flags = 0);
         void replace_outbound_reference(bare_form_id_t old, bare_form_id_t change_to, use_info_entry::flags_t flags = 0);

         void sever_all_outbound_references(); // works bidirectionally; use when deleting a form
         #pragma endregion

         #pragma region Functions for working copies
         //
         // "Working copies" are a helper functionality provided to make certain tasks 
         // easier for frontends. A working copy of a form is a duplicate of its loaded 
         // data, intended for use in temporary editing operations. A working copy can 
         // be "committed," overwriting the original form's data, or deleted.
         //
         // Essentially, if you want to have a dialog box for editing a form, with an 
         // "OK" button and a "Cancel" button, a working copy gives you a place to 
         // store changes that have been made within that dialog box, to be committed 
         // when the user clicks "OK" or deleted when the user clicks "Cancel."
         //
         // A form stub can only have one working copy. DovahKit makes absolutely no 
         // attempt to manage ownership or lifetimes for working copies OTHER THAN 
         // deleting the working copy in the form_stub destructor; the frontend is 
         // responsible for deciding when to create, commit, and delete working copies.
         //
         template<class C = loaded_forms::Form> C* get_working_copy() const noexcept {
            if constexpr (!std::is_same_v<C, loaded_forms::Form>) {
               if (this->form_type != C::form_type)
                  return nullptr;
            }
            return (C*)this->working_copy;
         }
         loaded_forms::Form* create_working_copy(); // returns nullptr if one already exists
         void commit_working_copy(); // commit the working copy, and then delete it
         void delete_working_copy(); // delete the working copy without committing it
         
         //
         // Helper function. In some functions, you will want to work with a form stub's 
         // working copy if there is one, or the normal loaded-form data otherwise. In 
         // these cases, you need to have a loaded_form_ptr on hand because if you do end 
         // up using the normal loaded-form data, the smart pointer is needed to keep that 
         // data alive until you're done with it.
         //
         template<class C = loaded_forms::Form> C* get_working_or_stable_copy(loaded_form_ptr<C>& p) noexcept;
         #pragma endregion
         
         static void* operator new(std::size_t sz);
         static void operator delete(void* ptr, std::size_t sz);
   };

   template<typename loaded_form_t> class loaded_form_ptr {
      //
      // This is intended as a smart pointer not for the FormStub itself, but for the 
      // loaded form data, going *through* the FormStub.
      //
      // Suggested usage for when you have a FormStub, know the type of form it holds, 
      // and wish to load and use the form data:
      //
      //    auto form  = myFormStub.load();
      //    auto quest = form.ptr_cast<loaded_forms::Quest>();
      //    //
      //    // ...and then you can use (quest) if it were a loaded_forms::Quest*. Even 
      //    // without casting, you can use (form) as if it were a loaded_forms::Form*.
      //
      // When these smart pointers are templated on a class other than the base Form 
      // class, they will type-check any incoming stubs and refuse to store any that 
      // have a mismatched type; loaded_form_ptr<loaded_forms::Quest>, for example, 
      // will not store a stub whose formType is not form_type::quest.
      //
      protected:
         form_stub* wrapped = nullptr;
         inline void _inc() {
            if (auto* fs = this->wrapped) {
               assert(!fs->refcount_is_maxed_out() && "form_stub refcount is already at maximum!");
               ++fs->refcount;
            }
         }
         inline void _dec() {
            if (auto* fs = this->wrapped) {
               assert(fs->get_refcount() != 0 && "form_stub refcount is already zero!");
               --fs->refcount;
               if (fs->refcount == 0)
                  fs->_unload_form();
            }
         }
         inline static form_stub* _type_check(form_stub* s) noexcept {
            if constexpr (std::is_same_v<loaded_form_t, loaded_forms::Form>)
               return s;
            if (s && s->form_type == loaded_form_t::form_type)
               return s;
            return nullptr;
         }
      public:
         using wrapped_type = loaded_form_t;
         //
         loaded_form_ptr() {}
         loaded_form_ptr(form_stub* stub) : wrapped(_type_check(stub)) {
            this->_inc();
         };
         loaded_form_ptr(const loaded_form_ptr& other) {
            this->wrapped = other.wrapped;
            this->_inc();
         }
         loaded_form_ptr(loaded_form_ptr&& other) noexcept {
            this->wrapped = other.wrapped;
            other.wrapped = nullptr;
         }

         template<typename other_form_t> requires (!std::is_same_v<other_form_t, wrapped_type>)
         loaded_form_ptr(const loaded_form_ptr<other_form_t>& other) {
            this->wrapped = _type_check(other.wrapped);
            this->_inc();
         }
         template<typename other_form_t> requires (!std::is_same_v<other_form_t, wrapped_type>)
         loaded_form_ptr(loaded_form_ptr<other_form_t>&& other) {
            this->wrapped = _type_check(other.wrapped);
            other.wrapped = nullptr;
         }

         ~loaded_form_ptr() {
            this->_dec();
            this->wrapped = nullptr;
         }

         operator bool() { return this->wrapped != nullptr && this->wrapped->form != nullptr; };
         operator loaded_form_t*() const noexcept { return (loaded_form_t*)this->wrapped->form; };
         loaded_form_t* operator->() const noexcept { return (loaded_form_t*)this->wrapped->form; };

         inline loaded_form_t* unwrap() const noexcept {
            if (!this->wrapped)
               return nullptr;
            return (loaded_form_t*)this->wrapped->form;
         }

         loaded_form_ptr<loaded_form_t>& operator=(form_stub* stub) {
            this->_dec();
            this->wrapped = _type_check(stub);
            this->_inc();
            return *this;
         }
         loaded_form_ptr<loaded_form_t>& operator=(const loaded_form_ptr<loaded_form_t>& other) {
            this->_dec();
            this->wrapped = other.wrapped;
            this->_inc();
            return *this;
         }
         loaded_form_ptr<loaded_form_t>& operator=(loaded_form_ptr<loaded_form_t>&& other) {
            this->_dec();
            this->wrapped = other.wrapped;
            other.wrapped = nullptr;
            return *this;
         }

         template<typename other_loaded_form_t> loaded_form_ptr<other_loaded_form_t> ptr_cast() {
            return loaded_form_ptr<other_loaded_form_t>(this->wrapped);
         }
   };

   #pragma region Detached form_stub members
   //
   // These member functions refer to members on the loaded_form_ptr class, and so must go 
   // after the class definition.
   //
   template<class C> C* form_stub::get_working_or_stable_copy(loaded_form_ptr<C>& p) noexcept {
      C* wc = this->get_working_copy<C>();
      if (wc) {
         p = nullptr;
         return wc;
      }
      p = this->load().ptr_cast<C>();
      return p.unwrap();
   }
   #pragma endregion
}