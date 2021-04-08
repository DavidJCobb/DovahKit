#include "form_stub.h"
#include <cassert>
#include "../helpers/bitwise.h"
#include "files/tes_file_reading/file_loader.h"
#include "forms/factories/construct.h"
#include "forms/factories/hardcoded.h"
#include "forms/factories/use_info.h"
#include "forms/Form.h"
#include "form_stub_addenda.h"
#include "form_stub_heap.h"
#include "form_stub_use_info_builder.h"
#include "logging.h"

namespace dovah {
   /*static*/ use_info_entry::flags_t use_info_entry::invert_flags(use_info_entry::flags_t f) {
      using flag = use_info_entry::flag;
      //
      flags_t flags = f;
      //
      flags &= ~(flag::i_am_child_of | flag::i_am_parent_of);
      if (f & use_info_entry::flag::i_am_child_of)
         flags |= use_info_entry::flag::i_am_parent_of;
      else if (f & use_info_entry::flag::i_am_parent_of)
         flags |= use_info_entry::flag::i_am_child_of;
      //
      return flags;
   }

   form_stub::form_stub() {
      this->file = file_data();
   }
   form_stub::~form_stub() {
      //
      // NOTE: It is not safe to try and sever connections between forms here, because 
      // the destructor for (file_load_order) deletes stubs directly without removing 
      // them from the form maps (because that could be slow...). 
      //
      if (this->get_refcount()) {
         #if _DEBUG
            __debugbreak(); // Something is still using this form_stub's loaded data. Why are you destroying it?
         #endif
         this->form = nullptr;
      }
      if (auto* form = this->form) { // needed for edited forms, hardcoded forms, and other forms that aren't normally allowed to unload
         this->form = nullptr;
         delete form;
      }
      if (auto* copy = this->working_copy) {
         this->working_copy = nullptr;
         delete copy;
      }
      if (this->has_multiple_source_files()) {
         if (this->files.entries)
            delete[] this->files.entries;
      }
      if (auto* a = this->addenda) {
         this->addenda = nullptr;
         delete a;
      }
   }
   void form_stub::_unload_form() {
      if (!this->can_unload_form())
         return;
      auto form = this->form;
      if (form) {
         delete form;
         this->form = nullptr;
      }
   }
   file_load_order& form_stub::_get_load_order() const noexcept {
      auto* file = this->get_file_at_index(0);
      assert(file && "Cannot retrieve a form stub's owning load order if it has no files!");
      return file->get_load_order();
   }
   loaded_form_ptr<loaded_forms::Form> form_stub::_load(bool force) {
      if (this->form)
         return loaded_form_ptr<loaded_forms::Form>(this); // already loaded
      if (!this->has_source_files())
         return loaded_form_ptr<loaded_forms::Form>(this); // no files to load from
      if (this->formType == form_type::setting || this->formType == form_type::none)
         return loaded_form_ptr<loaded_forms::Form>(this); // skip GMSTs (because they aren't actually forms) and none-stubs
      //
      auto& lo = this->_get_load_order();
      if (!force) {
         //
         // Don't try to load the form if the file's load order is in the middle of 
         // a save operation, or if loading is otherwise unsafe.
         //
         if (lo.is_form_loading_blocked(this))
            return loaded_form_ptr<loaded_forms::Form>(this);
      }
      //
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      if (!size)
         return loaded_form_ptr<loaded_forms::Form>(this); // no source files (this should never occur; it is only possible while the stub is being built)
      //
      auto  intfc  = load_order_interfaces::form_load(lo, *this);
      auto* loader = get_form_loader_function(this->formType);
      if (!loader)
         return loaded_form_ptr<loaded_forms::Form>(this); // load failed
      bool can_be_parent = form_type_info::lookup(this->formType).flags & form_type_info::flag::can_have_children;
      if (auto* file = arr[0].pointer) {
         if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) {
            //
            // This file is the hardcoded dummy file. We'll need to instantiate the loaded-form 
            // class a bit differently, since hardcoded forms don't come from a "real" file.
            //
            this->form = instantiate_hardcoded_form(*this);
         } else {
            if (arr[0].offset == 0)
               return loaded_form_ptr<loaded_forms::Form>(this); // file has no actual data (unsaved new active file, etc.). skip it
            //
            // This file is a real file. Load from it.
            //
            loaded_forms::Form::constructor_params fcp;
            fcp.stub = this;
            //
            intfc.current_file      = file;
            intfc.is_winning_record = (1 == size);
            intfc.is_partial_record = false;
            if (file->load_record_at(arr[0].offset)) {
               auto& record = file->get_current_record();
               this->form = create_blank_loaded_form_by_type(this->formType, fcp);
               if (this->form)
                  (loader)(this->form, record, intfc);
            }
         }
      }
      if (!this->form)
         return loaded_form_ptr<loaded_forms::Form>(this); // load failed
      //
      uint32_t last_record_flags = arr[0].flags;
      for (uint16_t i = 1; i < size; ++i) {
         auto  offset = arr[i].offset;
         if (offset == 0)
            continue;
         auto* file   = arr[i].pointer;
         //
         intfc.is_winning_record = (i + 1 == size);
         intfc.current_file      = file;
         intfc.is_partial_record = can_be_parent && (arr[i].flags & tes_file_record_header::flag::partial);
         intfc.last_record_flags = last_record_flags;
         if (file->load_record_at(offset)) {
            auto& record = file->get_current_record();
            (loader)(this->form, record, intfc);
         }
         last_record_flags = arr[i].flags;
      }
      //
      return loaded_form_ptr<loaded_forms::Form>(this);
   }

   loaded_form_ptr<loaded_forms::Form> form_stub::load() {
      return this->_load();
   }
   loaded_form_ptr<loaded_forms::Form> form_stub::get_content_if_loaded() {
      return loaded_form_ptr<loaded_forms::Form>(this);
   }
   bool form_stub::fetch_record_header(tes_file_record_header& out, uint32_t& out_record_decompressed_size, int16_t source_file_index) const noexcept {
      auto* data = this->get_source_file_info(source_file_index);
      if (!data)
         return false;
      return data->pointer->fetch_record_header(data->offset, out, out_record_decompressed_size);
   }
   void form_stub::do_custom_parse(void(*loader)(const form_stub&, tes_file_reading::record&, load_order_interfaces::form_load&), tes_file_reading::basic_reader* reader) const noexcept {
      if (!loader)
         return; // no loader supplied
      if (!this->has_source_files())
         return; // no files to load from
      //
      auto& lo = this->_get_load_order();
      if (lo.is_form_loading_blocked(this)) // don't allow a load if the file's load order is in the middle of a save operation or some other unsafe circumstance
         return;
      //
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      if (!size)
         return; // no source files (this should never occur; it is only possible while the stub is being built)
      //
      auto     intfc             = load_order_interfaces::form_load(lo, *this);
      bool     can_be_parent     = form_type_info::lookup(this->formType).flags & form_type_info::flag::can_have_children;
      uint32_t last_record_flags = 0;
      for (uint16_t i = 0; i < size; ++i) {
         auto* file   = arr[i].pointer;
         auto  offset = arr[i].offset;
         if (!file || !offset) // no file, or file has no actual data (e.g. unsaved new file); skip it.
            continue;
         if (i == 0) {
            if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy)
               continue;
         }
         intfc.is_winning_record = (i + 1 == size);
         intfc.current_file      = file;
         intfc.is_partial_record = can_be_parent && (arr[i].flags & tes_file_record_header::flag::partial);
         intfc.last_record_flags = last_record_flags;
         //
         bool result;
         if (reader) {
            file->adopt(*reader);
            result = reader->load_record_at(offset);
         } else {
            result = file->load_record_at(offset);
         }
         //
         if (result) {
            if (reader)
               (loader)(*this, reader->get_current_record(), intfc);
            else
               (loader)(*this, file->get_current_record(), intfc);
         }
         last_record_flags = arr[i].flags;
      }
      //
      if (reader) {
         reader->file_data = nullptr;
         reader->file_size = 0;
         reader->loader    = nullptr;
      }
   }

   #pragma region form_stub file list
   void form_stub::_add_file(owner_file_t& f, uint32_t offset, uint32_t record_flags) {
      if (!this->has_multiple_source_files()) {
         if (!this->file) {
            this->file.pointer = &f;
            this->file.offset  = offset;
            this->file.flags   = record_flags;
            return;
         }
         auto prior = this->file;
         this->flags |= flag::has_multiple_source_files;
         this->files.entries = new file_data[2];
         this->files.count   = 2;
         this->files.entries[0] = prior;
         auto& next = this->files.entries[1];
         next.pointer = &f;
         next.offset  = offset;
         next.flags   = record_flags;
         return;
      }
      auto resized = new file_data[size_t(this->files.count) + 1]; /// cast silences warning C26451 and is otherwise pointless
      uint16_t i = 0;
      for (; i < this->files.count; ++i)
         resized[i] = this->files.entries[i];
      auto& last = resized[i];
      last.pointer = &f;
      last.offset  = offset;
      last.flags   = record_flags;
      delete[] this->files.entries;
      this->files.entries = resized;
      ++this->files.count;
   }
   void form_stub::_set_source_file_offset(owner_file_t& f, uint32_t offset) {
      auto i = this->index_of_file(&f);
      if (i < 0) {
         this->_add_file(f, offset);
         return;
      }
      if (i == 0 && !this->has_multiple_source_files()) {
         this->file.offset = offset;
         return;
      }
      this->files.entries[i].offset = offset;
   }
   void form_stub::_modify_source_file_record_flags(owner_file_t& f, uint32_t mask, bool clear_or_set) {
      auto i = this->index_of_file(&f);
      if (i < 0)
         return;
      if (i == 0 && !this->has_multiple_source_files()) {
         cobb::edit_bit(this->file.flags, mask, clear_or_set);
         return;
      }
      cobb::edit_bit(this->files.entries[i].flags, mask, clear_or_set);
   }
   void form_stub::_get_source_file_list(file_data*& out_arr, uint16_t& out_count) const noexcept {
      out_arr   = nullptr;
      out_count = 0;
      if (!this->has_multiple_source_files()) {
         if (this->file) {
            out_arr   = const_cast<file_data*>(&this->file);
            out_count = 1;
         }
         return;
      }
      out_arr   = this->files.entries;
      out_count = this->files.count;
   }
   void form_stub::_adopt_source_file_list(const form_stub* other) {
      if (!other)
         return;
      uint16_t   count_a;
      uint16_t   count_b;
      file_data* array_a;
      file_data* array_b;
      other->_get_source_file_list(array_a, count_a);
      this->_get_source_file_list(array_b, count_b);
      //
      auto* resized = new file_data[size_t(count_a) + count_b]; /// cast silences warning C26451 and is otherwise pointless
      for (uint16_t i = 0; i < count_a; ++i)
         resized[i] = array_a[i];
      for (uint16_t i = 0; i < count_b; ++i)
         resized[i + count_a] = array_b[i];
      //
      if (this->has_multiple_source_files())
         delete[] this->files.entries;
      this->files.entries = resized;
      this->files.count   = count_a + count_b;
      this->flags |= flag::has_multiple_source_files;
   }
   void form_stub::_set_source_file_list(const std::vector<file_data>& list) {
      if (this->has_multiple_source_files())
         delete[] this->files.entries;
      auto size = list.size();
      if (size < 2) {
         this->flags &= ~flag::has_multiple_source_files;
         this->file.offset  = 0;
         this->file.pointer = nullptr;
         this->file.flags   = 0;
         if (size) {
            this->file.offset  = list[0].offset;
            this->file.pointer = list[0].pointer;
            this->file.flags   = list[0].flags;
         }
         return;
      }
      this->flags |= flag::has_multiple_source_files;
      this->files.entries = new file_data[size];
      this->files.count   = size;
      for (uint16_t i = 0; i < size; ++i) {
         auto& f = this->files.entries[i];
         auto& o = list[i];
         f.offset  = o.offset;
         f.pointer = o.pointer;
         f.flags   = o.flags;
      }
   }

   const form_stub::file_data* form_stub::get_source_file_info(int16_t i) const noexcept {
      return this->_get_source_file_info(i);
   }

   bool form_stub::has_source_files() const noexcept {
      if (!this->has_multiple_source_files())
         return this->file;
      return this->files.entries != nullptr;
   }
   int16_t form_stub::source_file_count() const noexcept {
      if (!this->has_multiple_source_files()) {
         if (!this->file)
            return 0;
         return 1;
      }
      return this->files.count;
   }
   int16_t form_stub::index_of_file(const owner_file_t* f) const noexcept {
      if (!this->has_multiple_source_files()) {
         if (this->file.pointer == f)
            return 0;
         return -1;
      }
      auto size = this->files.count;
      for (uint16_t i = 0; i < size; ++i)
         if (this->files.entries[i].pointer == f)
            return i;
      return -1;
   }
   bool form_stub::file_list_includes(const owner_file_t* f) const noexcept {
      if (!this->has_multiple_source_files())
         return this->file.pointer == f;
      auto size = this->files.count;
      for (uint16_t i = 0; i < size; ++i)
         if (this->files.entries[i].pointer == f)
            return true;
      return false;
   }
   form_stub::owner_file_t* form_stub::get_file_at_index(int16_t i) const noexcept {
      auto* data = this->get_source_file_info(i);
      if (data)
         return data->pointer;
      return nullptr;
   }
   uint32_t form_stub::get_file_offset(int16_t file_index) const noexcept {
      auto* data = this->get_source_file_info(file_index);
      if (data)
         return data->offset;
      return 0;
   }
   #pragma endregion

   bool form_stub::can_unload_form() const noexcept {
      if (this->is_edited())
         return false;
      return true;
   }
   bool form_stub::is_edited_or_in_active_file() const noexcept {
      if (this->is_edited())
         return true;
      auto& lo = this->_get_load_order();
      if (lo.is_defined_in_active_file(*this))
         return true;
      return false;
   }
   bool form_stub::is_injected() const noexcept {
      if (auto* first_file = this->get_file_at_index(0)) {
         using df = owner_file_t::detail_flag;
         //
         // Always double-check the form's first file, if there is one, because that file might be one 
         // of the "dummy" files used for things like none-stubs and hardcoded forms. We don't want 
         // those forms to test as "injected."
         //
         if (first_file->header.details & (df::is_none_stub_dummy | df::is_hardcoded_dummy))
            return false;
      }
      //
      // Okay, we know that the form isn't contributed to by a "dummy" file. Let's grab the file that 
      // we would *expect* the form to be defined in based on its form ID, and see if that file actually 
      // contributes to the form. If not, then the form has been injected into that file.
      //
      auto& lo     = this->_get_load_order();
      auto  prefix = file_prefix::from_form_id(this->formID, lo.get_current_game() == game::skyrim_classic);
      auto* file   = lo.get_file_by_prefix(prefix);
      if (!file)
         return false;
      if (this->file_list_includes(file))
         return false;
      return true;
   }
   bool form_stub::is_non_overridden_hardcoded_form() const noexcept {
      if (this->is_hardcoded()) {
         auto* info = this->get_source_file_info(-1); // get last file
         if (!info || !info->pointer)
            return false;
         if (info->pointer->header.details & owner_file_t::detail_flag::is_hardcoded_dummy)
            return true;
      }
      return false;
   }
   bool form_stub::is_none_stub() const noexcept {
      auto* info = this->get_source_file_info(-1); // get last file
      if (!info || !info->pointer)
         return false;
      if (info->pointer->header.details & owner_file_t::detail_flag::is_none_stub_dummy)
         return true;
      return false;
   }
   void form_stub::set_edited(bool v) {
      if (this->is_edited() == v)
         return;
      cobb::modify_bit(this->flags, flag::is_edited, v);
      if (v)
         this->_get_load_order().stub_flagged_as_edited(this);
      else {
         if (this->refcount == 0)
            this->_unload_form();
      }
   }

   #pragma region form stub record flags
   uint32_t form_stub::get_record_flags() const noexcept {
      auto* info = this->get_source_file_info();
      if (!info)
         return 0;
      return info->flags;
   }
   bool form_stub::test_record_flags(uint32_t mask) const noexcept {
      return this->test_record_flags_for_file(mask, -1);
   }
   void form_stub::edit_record_flags(uint32_t mask, bool clear_or_set) noexcept {
      auto* info = this->_get_source_file_info();
      if (!info)
         return;
      auto& lo     = this->_get_load_order();
      auto* active = const_cast<owner_file_t*>(lo.get_active_file()); // HACK HACK HACK
      if (!active)
         return;
      if (info->pointer == active) {
         cobb::edit_bit(info->flags, mask, clear_or_set);
         if (!(mask & tes_file_record_header::flag::partial) || !clear_or_set) { // if we're not explicitly setting the "partial" flag for some reason
            //
            // Modifying the flags in any respect will also flag the form as "edited," which 
            // means it won't save as partial... which means it effectively *is* no longer 
            // partial, and keeping the flag will only cause confusion for the frontend.
            //
            info->flags &= ~tes_file_record_header::flag::partial;
         }
         return;
      }
      if (!clear_or_set)
         return;
      this->set_edited(true);
      this->_add_file(*active, 0, mask);
   }
   bool form_stub::test_record_flags_for_file(uint32_t mask, int16_t file_index) const noexcept {
      if (file_index < 0)
         return false;
      auto* info = this->get_source_file_info(file_index);
      if (!info)
         return false;
      return (info->flags & mask) == mask;
   }
   bool form_stub::test_record_flags_for_file(uint32_t mask, owner_file_t& f) const noexcept {
      return this->test_record_flags_for_file(mask, this->index_of_file(&f));
   }
   #pragma endregion

   #pragma region form_stub use info functions
   void form_stub::build_outbound_refs(tes_file_reading::basic_reader& reader) noexcept {
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      //
      form_stub_use_info_builder use_interface(*this);
      bool can_be_parent = form_type_info::lookup(this->formType).flags & form_type_info::flag::can_have_children;
      auto& lo = this->_get_load_order();
      //
      for (uint16_t i = 0; i < size; ++i) {
         if (i == size - 1)
            use_interface._is_final_file = true;
         auto* file = arr[i].pointer;
         use_interface._is_active_file = lo.file_is_active(*file);
         if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) {
            build_hardcoded_form_outbound_refs(use_interface);
            use_interface.commit();
         } else {
            if (arr[i].offset == 0)
               continue;
            file->adopt(reader);
            if (reader.load_record_at(arr[i].offset)) {
               use_interface.is_partial_record = (i > 0) && can_be_parent && (arr[i].flags & tes_file_record_header::flag::partial);
               //
               auto& record  = reader.get_current_record();
               auto  builder = get_outbound_uses_builder_by_type(this->formType);
               if (builder) {
                  builder(record, use_interface);
                  use_interface.commit();
               }
            }
         }
      }
      reader.file_data = nullptr;
      reader.file_size = 0;
      reader.loader    = nullptr;
   }
   void form_stub::send_inbound_refs() noexcept {
      //
      // This function takes all outbound connections and creates, for the connected forms, inbound 
      // connections from this form. It is intended only for use at the tail end of the (file_load_order) 
      // load process, for building use info for all loaded forms: (_add_one_way_outbound_reference) is 
      // used to create single-direction connections, and this function subsequently makes all such 
      // connections bidirectional.
      //
      // If you were to call this function later on, you would end up with redundant inbound connections, 
      // as already-sent outbound connections would be sent again.
      //
      for (auto it = this->outbound.begin(); it != this->outbound.end(); ++it) {
         use_info_entry::flags_t flags = use_info_entry::invert_flags(it->second.flags);
         //
         if (it->second.other)
            it->second.other->receive_inbound_ref(this, it->second.refcount, flags);
      }
   }
   void form_stub::receive_inbound_ref(form_stub* inbound, uint32_t refcount, use_info_entry::flags_t flags) noexcept {
      auto& list  = this->inbound;
      auto& entry = list[inbound->formID];
      entry.other = inbound;
      entry.refcount += refcount;
      entry.flags = flags;
   }

   void form_stub::_add_one_way_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags) {
      //
      // This function creates a single-direction connection from (this) to (to_stub), with the understanding 
      // that a later call to (this->send_inbound_refs()) will make all such connections bidirectional. As 
      // such, this function should only be used at the tail end of the (file_load_order) load process, for 
      // building use info for all loaded forms.
      //
      // If you need to programmatically create a connection from this form to another, after the initial file 
      // load, then call (replace_outbound_reference) on this form, passing (bare_form_id_t(0)) as the "old" 
      // form ID that we want to "replace." This is not strictly intuitive, but then, code outside of the 
      // backend should not be manipulating form stubs and their use info directly, but rather should be 
      // working with loaded-form classes. If you're thinking in terms of manipulating form stubs rather than 
      // in terms of manipulating forms, then you're either thinking in the wrong terms, or you need to make 
      // sure that you fully understand how form stubs are supposed to work.
      //
      if (!to_stub)
         return;
      auto& list  = this->outbound;
      auto& entry = list[to_stub->formID];
      if (!entry.other)
         entry.other = to_stub;
      entry.refcount++;
      //
      if (flags)
         entry.flags |= flags;
   }
   void form_stub::_add_one_way_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags) {
      //
      // Please refer to the documentation comments in this function's other overload.
      //
      if (toFormID == 0)
         return;
      auto& list  = this->outbound;
      auto& entry = list[toFormID];
      auto& lo    = this->_get_load_order();
      if (!entry.other)
         entry.other = lo.get_form(toFormID, false);
      entry.refcount++;
      //
      if (flags) {
         entry.flags |= flags;
      }
   }
   void form_stub::_set_parent_form_one_way(form_stub* parent) {
      //
      // When setting a parent form during the load process (i.e. before the use info build step), 
      // you should use this function instead of (set_parent_form). This is because at the end of 
      // the use info build step, all outbound references are made bidirectional. If you use the 
      // (set_parent_form) function, you'll create bidirectional references, which will be made 
      // bidirectional "again" i.e. the parent form will believe it has two inbound references 
      // when in reality it only has one.
      //
      auto& list = this->outbound;
      for (auto it = list.begin(); it != list.end(); ++it) {
         auto& pair  = *it;
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_child_of) {
            if (entry.other == parent)
               return;
            entry.flags &= ~use_info_entry::flag::i_am_child_of;
            if (--entry.refcount == 0)
               list.erase(it);
            break;
         }
      }
      //
      // Set the new parent form.
      //
      this->_add_one_way_outbound_reference(parent, use_info_entry::flag::i_am_child_of);
   }
   #pragma endregion

   form_stub::file_data* form_stub::_get_source_file_info(int16_t i) const noexcept {
      if (!this->has_multiple_source_files()) {
         if (i != 0 && i != -1)
            return nullptr;
         return const_cast<file_data*>(&this->file);
      }
      if (i < 0)
         i += this->files.count;
      if (i >= this->files.count)
         return nullptr;
      return &this->files.entries[i];
   }

   void form_stub::_insert_child_topic_info(form_stub& info, size_t at) {
      assert(info.get_parent_form() == this);
      //
      if (!this->addenda)
         this->addenda = new form_stub_addenda;
      auto& list = this->addenda->ordered_children;
      auto  it   = std::find(list.begin(), list.end(), &info);
      //
      if (at == 0xFFFFFFFF) {
         //
         // Sentinel value 0xFFFFFFFF will avoid moving an info at all if it's already 
         // in any topic's info list, or place it at the end of the parent topic's info 
         // list otherwise.
         //
         if (it != list.end()) // already in our list
            return;
         at = std::numeric_limits<size_t>::max();
      }
      if (at >= list.size()) {
         list.push_back(&info);
         return;
      }
      //
      if (it != list.end()) {
         //
         // This info is already in this topic's info list. Remove it now, so that 
         // the later insertion ends up just moving it.
         //
         size_t i = std::distance(list.begin(), it);
         if (i < at)
            --at; // Removing this info will shift the insertion position up.
         //
         list.erase(it);
      }
      list.insert(list.cbegin() + at, &info);
   }
   void form_stub::_remove_child_topic_info(form_stub& info, bool loading) {
      if (loading && info.test_record_flags(tes_file_record_header::flag::partial))
         return;
      if (!this->addenda)
         return;
      auto& list = this->addenda->ordered_children;
      list.erase(std::remove(list.begin(), list.end(), &info), list.end());
   }

   #pragma region Addenda helper functions
   form_stub_addenda& form_stub::get_or_create_addenda() noexcept {
      if (!this->addenda)
         this->addenda = new form_stub_addenda;
      return *this->addenda;
   }
   //
   bool form_stub::get_grid_coordinates(int32_t& x, int32_t& y) const noexcept {
      x = 0;
      y = 0;
      if (!this->addenda)
         return false;
      if (!(this->addenda->flags & form_stub_addenda::flag::has_grid_coordinates))
         return false;
      auto& gc = this->addenda->grid_coords;
      x = gc.x;
      y = gc.y;
      return true;
   }
   size_t form_stub::child_info_count() const noexcept {
      if (this->formType != form_type::topic)
         return 0;
      if (!this->addenda)
         return 0;
      return this->addenda->ordered_children.size();
   }
   size_t form_stub::index_of_child_info(form_stub& info) const noexcept {
      if (this->formType != form_type::topic || info.formType != form_type::topic_info)
         return std::string::npos;
      if (!this->addenda)
         return std::string::npos;
      auto&  list = this->addenda->ordered_children;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == &info)
            return i;
      return std::string::npos;
   }
   void form_stub::insert_child_topic_info(form_stub& info, size_t at) {
      if (this->formType != form_type::topic)
         return;
      if (info.formType != form_type::topic_info)
         return;
      if (info.get_parent_form() != this) {
         info.set_parent_form(this);
         if (at == std::string::npos)
            return;
      }
      //
      // Reposition (info) within the list:
      //
      assert(this->addenda);
      auto& list = this->addenda->ordered_children;
      auto  size = list.size();
      if (at >= size)
         at = size - 1;
      auto it = std::find(list.begin(), list.end(), &info);
      assert(it != list.end());
      if (it != list.begin() + at)
         std::move(it, it + 1, list.begin() + at);
   }
   void form_stub::remove_child_topic_info(form_stub& info) {
      if (this->formType != form_type::topic)
         return;
      if (info.formType != form_type::topic_info)
         return;
      if (info.get_parent_form() != this)
         return;
      info.orphan();
   }
   //
   void form_stub::sever_addenda_references_to(form_stub& other) {
      if (this->addenda)
         this->addenda->sever_references_to(other);
   }
   #pragma endregion

   #pragma region Form stub parenthood functions
   bool form_stub::has_child_forms() const noexcept {
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_parent_of)
            return true;
      }
      return false;
   }
   form_stub* form_stub::get_parent_form() const noexcept {
      for (auto& pair : this->outbound) {
         auto& entry = pair.second;
         if (!entry.other)
            continue;
         if (!(entry.flags & use_info_entry::flag::i_am_child_of))
            continue;
         return entry.other;
      }
      return nullptr;
   }
   bool form_stub::is_parent_form_of(form_stub& child) const noexcept {
      auto& map = this->inbound;
      for (auto it = map.begin(); it != map.end(); ++it) {
         auto& pair = *it;
         if (pair.first != child.formID)
            continue;
         if (pair.second.flags & use_info_entry::flag::i_am_parent_of)
            return true;
      }
      return false;
   }
   void form_stub::orphan() {
      auto* parent = this->get_parent_form();
      if (!parent)
         return;
      this->revoke_outbound_reference(parent, use_info_entry::flag::i_am_child_of);
      if (this->formType == form_type::topic_info && parent->formType == form_type::topic)
         parent->_remove_child_topic_info(*this, false);
   }
   void form_stub::set_parent_form(form_stub* target) noexcept {
      auto* parent = this->get_parent_form();
      if (target == parent)
         return;
      this->orphan();
      if (!target)
         return;
      this->replace_outbound_reference(0, target, use_info_entry::flag::i_am_child_of);
      if (target->formType == form_type::topic && this->formType == form_type::topic_info)
         target->_insert_child_topic_info(*this);
   }
   #pragma endregion

   [[nodiscard]] bool form_stub::is_any_descendant_form_edited() const noexcept {
      if (!(form_type_info::lookup(this->formType).flags & form_type_info::flag::can_have_children)) {
         return false;
      }
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::i_am_parent_of))
            continue;
         auto stub = entry.other;
         if (stub->is_edited() || stub->is_any_descendant_form_edited())
            return true;
      }
      return false;
   }
   [[nodiscard]] bool form_stub::does_descendant_form_need_save() const noexcept {
      if (!(form_type_info::lookup(this->formType).flags & form_type_info::flag::can_have_children)) {
         return false;
      }
      auto& owner = this->_get_load_order();
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::i_am_parent_of))
            continue;
         auto stub = entry.other;
         if (owner.is_defined_or_overridden_in_active_file(*stub))
            return true;
         if (stub->is_edited() || stub->does_descendant_form_need_save())
            return true;
      }
      return false;
   }
   [[nodiscard]] bool form_stub::needs_save() const noexcept {
      if (this->is_edited())
         return true;
      auto& owner = this->_get_load_order();
      if (owner.is_defined_or_overridden_in_active_file(*this))
         return true;
      return this->does_descendant_form_need_save();
   }

   [[nodiscard]] bool form_stub::is_exterior_cell() const noexcept {
      if (this->formType != form_type::cell)
         return false;
      return this->get_parent_form() != nullptr;
   }
   namespace {
      union _cell_grid_dword {
         struct {
            int16_t y;
            int16_t x;
         };
         uint32_t merged; // 0xXXXXYYYY, but since it's in little-endian, the words are reversed above
      };
   }
   [[nodiscard]] uint32_t form_stub::get_cell_block() const noexcept {
      if (this->formType != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         if (!this->addenda)
            return 0;
         auto& gc = this->addenda->grid_coords;
         _cell_grid_dword value;
         value.x = gc.x / 8 / 4;
         value.y = gc.y / 8 / 4;
         return value.merged;
      }
      return (this->formID % 10);
   }
   [[nodiscard]] uint32_t form_stub::get_cell_sub_block() const noexcept {
      if (this->formType != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         if (!this->addenda)
            return 0;
         auto& gc = this->addenda->grid_coords;
         _cell_grid_dword value;
         value.x = gc.x / 8;
         value.y = gc.y / 8;
         return value.merged;
      }
      return (this->formID % 100) / 10;
   }
   [[nodiscard]] form_stub* form_stub::get_outbound_use_with_flag(use_info_entry::flags_t f) const noexcept {
      assert(f && "This function is meaningless without a flag specified.");
      for (auto& pair : this->outbound)
         if (pair.second.flags & f)
            return pair.second.other;
      return nullptr;
   }

   #pragma region Functions for modifying use info
   void form_stub::revoke_outbound_reference(form_stub* target, use_info_entry::flags_t flags) {
      //
      // Bidirectionally sever a connection from this form to another: this form's outbound 
      // connection will be severed, and the other form's inbound connection will be severed. 
      // Note that this is not the same thing as wholly deleting a (use_info_entry), as each 
      // entry represents all connections from one form to another.
      //
      auto& target_list = target->inbound;
      for (auto it = target_list.begin(); it != target_list.end(); ++it) {
         auto& pair  = *it;
         auto& entry = pair.second;
         if (pair.first == this->formID) {
            entry.flags &= ~use_info_entry::invert_flags(flags);
            if (--entry.refcount == 0)
               target_list.erase(it);
            break;
         }
      }
      auto& subject_list = this->outbound;
      for (auto it = subject_list.begin(); it != subject_list.end(); ++it) {
         auto& pair  = *it;
         auto& entry = pair.second;
         if (pair.first == target->formID) {
            entry.flags &= ~flags;
            if (--entry.refcount == 0)
               subject_list.erase(it);
            break;
         }
      }
   }
   void form_stub::revoke_all_outbound_references_to(form_stub* target) {
      //
      // Bidirectionally sever a connection from this form to another: this form's outbound 
      // connection will be severed, and the other form's inbound connection will be severed.
      //
      auto& target_list = target->inbound;
      for (auto it = target_list.begin(); it != target_list.end(); ++it) {
         auto& pair  = *it;
         auto& entry = pair.second;
         if (pair.first == this->formID) {
            target_list.erase(it);
            break;
         }
      }
      auto& subject_list = this->outbound;
      for (auto it = subject_list.begin(); it != subject_list.end(); ++it) {
         auto& pair  = *it;
         auto& entry = pair.second;
         if (pair.first == target->formID) {
            subject_list.erase(it);
            break;
         }
      }
   }
   void form_stub::replace_outbound_reference(bare_form_id_t old, form_stub* new_stub, use_info_entry::flags_t flags) {
      //
      // This function replaces an outbound reference from this form to some other form, while also 
      // making appropriate changes to that other form's inbound connections.
      //
      // This function can also be used to programmatically create a connection from this form to 
      // the other form. Code outside of the (file_load_order) load process should call this function 
      // rather than calling (add_outbound_reference); refer to its documentation for details.
      //
      if (old != 0) {
         form_stub* old_stub = nullptr;
         for (auto& pair : this->outbound)
            if (pair.first == old)
               old_stub = pair.second.other;
         //
         if (old_stub)
            this->revoke_outbound_reference(old_stub, flags);
      }
      if (!new_stub)
         return;
      //
      this->_add_one_way_outbound_reference(new_stub, flags);
      new_stub->receive_inbound_ref(this, 1, use_info_entry::invert_flags(flags));
   }
   void form_stub::replace_outbound_reference(bare_form_id_t old, bare_form_id_t change_to, use_info_entry::flags_t flags) {
      auto& lo       = this->_get_load_order();
      auto* new_stub = lo.get_form(change_to, false);
      this->replace_outbound_reference(old, new_stub, flags);
   }

   void form_stub::sever_all_outbound_references() {
      for (auto& pair : this->outbound) {
         auto& entry  = pair.second;
         auto& target = *entry.other;
         //
         target.inbound.erase(this->formID);
      }
      this->outbound.clear();
   }
   #pragma endregion

   #pragma region Functions for working copies
   loaded_forms::Form* form_stub::create_working_copy() {
      if (this->working_copy)
         return nullptr;
      //
      loaded_forms::Form::constructor_params fcp;
      fcp.stub = this;
      fcp.is_working_copy = true;
      //
      auto* instance = create_blank_loaded_form_by_type(this->formType, fcp);
      if (instance) {
         auto source = this->load();
         if (!source || !source->_clone_impl(instance)) {
            delete instance;
            instance = nullptr;
         }
      }
      this->working_copy = instance;
      return instance;
   }
   void form_stub::commit_working_copy() {
      auto* working = this->working_copy;
      assert(working);
      //
      auto loaded = this->load();
      loaded->_clear_impl();
      working->_clone_impl(loaded);
      //
      this->working_copy = nullptr;
      delete working;
   }
   void form_stub::delete_working_copy() {
      if (!this->working_copy)
         return;
      auto* working = this->working_copy;
      this->working_copy = nullptr;
      delete working;
   }
   #pragma endregion

   /*static*/ void* form_stub::operator new(std::size_t sz) {
      if (sz != sizeof(form_stub))
         return ::operator new(sz);
      return form_stub_heap::allocate();
   }
   /*static*/ void form_stub::operator delete(void* ptr, std::size_t sz) {
      if (sz != sizeof(form_stub))
         return ::operator delete(ptr, sz);
      return form_stub_heap::free(ptr);
   }
}