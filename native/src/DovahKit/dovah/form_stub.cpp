#include "form_stub.h"
#include <cassert>
#include "../helpers/bitwise.h"
#include "files/tes_file_reading/file_loader.h"
#include "forms/factories/construct.h"
#include "forms/factories/hardcoded.h"
#include "forms/factories/use_info.h"
#include "forms/Form.h"
#include "form_stub_heap.h"
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

   #pragma region form_stub_use_info_builder
   void form_stub_use_info_builder::add_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags) {
      this->_stub._add_one_way_outbound_reference(to_stub, flags);
   }
   void form_stub_use_info_builder::add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags) {
      this->_stub._add_one_way_outbound_reference(toFormID, flags);
   }
   #pragma endregion

   form_stub::form_stub() {
      this->file = file_data();
   }
   form_stub::~form_stub() {
      if (this->get_refcount()) {
         #if _DEBUG
            __debugbreak(); // Something is still using this form_stub's loaded data. Why are you destroying it?
         #endif
         this->form = nullptr;
      }
      if (auto form = this->form) { // needed for edited forms, hardcoded forms, and other forms that aren't normally allowed to unload
         delete form;
         this->form = nullptr;
      }
      if (this->has_multiple_source_files()) {
         if (this->files.entries)
            delete[] this->files.entries;
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
      assert(file);
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
      if (auto* file = arr[0].pointer) {
         if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) {
            //
            // This file is the hardcoded dummy file. We'll need to instantiate the loaded-form 
            // class a bit differently, since hardcoded forms don't come from a "real" file.
            //
            this->form = instantiate_hardcoded_form(*this);
            if (this->form)
               this->form->stub = this;
         } else {
            if (arr[0].offset == 0)
               return loaded_form_ptr<loaded_forms::Form>(this); // file has no actual data (unsaved new active file, etc.). skip it
            //
            // This file is a real file. Load from it.
            //
            intfc.current_file      = file;
            intfc.is_winning_record = (1 == size);
            if (file->load_record_at(arr[0].offset)) {
               auto& record = file->get_current_record();
               this->form = create_blank_loaded_form_by_type(this->formType);
               if (this->form) {
                  this->form->stub = this;
                  (loader)(this->form, record, intfc);
               }
            }
         }
      }
      if (!this->form)
         return loaded_form_ptr<loaded_forms::Form>(this); // load failed
      //
      for (uint16_t i = 1; i < size; ++i) {
         auto  offset = arr[i].offset;
         if (offset == 0)
            continue;
         auto* file   = arr[i].pointer;
         //
         intfc.is_winning_record = (i + 1 == size);
         intfc.current_file      = file;
         if (file->load_record_at(offset)) {
            auto& record = file->get_current_record();
            (loader)(this->form, record, intfc);
         }
      }
      //
      return loaded_form_ptr<loaded_forms::Form>(this);
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
      auto resized = new file_data[size_t(this->files.count) + 1]; // cast silences warning C26451 and is otherwise pointless
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
   void form_stub::_set_active_file_data(owner_file_t& f, uint32_t offset) {
      auto i = this->index_of_file(&f);
      if (i < 0) {
         this->_add_file(f, offset);
         return;
      }
      if (i == 0 && !this->has_multiple_source_files()) {
         this->file.pointer = &f;
         this->file.offset  = offset;
         return;
      }
      this->files.entries[i].pointer = &f;
      this->files.entries[i].offset  = offset;
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
      auto* resized = new file_data[size_t(count_a) + count_b]; // cast silences warning C26451 and is otherwise pointless
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
      if (!this->has_multiple_source_files()) {
         if (i != 0 && i != -1)
            return nullptr;
         return &this->file;
      }
      if (i < 0)
         i += this->files.count;
      if (i >= this->files.count)
         return nullptr;
      return &this->files.entries[i];
   }

   bool form_stub::has_source_files() const noexcept {
      if (!this->has_multiple_source_files())
         return this->file;
      return this->files.entries != nullptr;
   }
   uint16_t form_stub::source_file_count() const noexcept {
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

   uint32_t form_stub::get_record_flags() const noexcept {
      auto* info = this->get_source_file_info();
      if (!info)
         return 0;
      return info->flags;
   }
   bool form_stub::test_record_flags(uint32_t mask) const noexcept {
      auto* info = this->get_source_file_info();
      if (!info)
         return false;
      return (info->flags & mask) == mask;
   }
   void form_stub::edit_record_flags(uint32_t mask, bool clear_or_set) noexcept {
      auto* info = this->get_source_file_info();
      if (!info)
         return;
      auto& lo     = this->_get_load_order();
      auto* active = const_cast<owner_file_t*>(lo.get_active_file()); // HACK HACK HACK
      if (!active)
         return;
      if (info->pointer == active) {
         cobb::edit_bit(info->flags, mask, clear_or_set);
         return;
      }
      if (!clear_or_set)
         return;
      this->set_edited(true);
      this->_add_file(*active, 0, mask);
   }

   #pragma region form_stub use info functions
   void form_stub::build_outbound_refs(tes_file_reading::basic_reader& reader) noexcept {
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      //
      form_stub_use_info_builder use_interface(*this);
      //
      for (uint16_t i = 0; i < size; ++i) {
         if (i == size - 1)
            use_interface._is_final_file = true;
         auto* file = arr[i].pointer;
         if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) {
            build_hardcoded_form_outbound_refs(use_interface);
         } else {
            file->adopt(reader);
            if (reader.load_record_at(arr[i].offset)) {
               auto& record  = reader.get_current_record();
               auto  builder = get_outbound_uses_builder_by_type(this->formType);
               if (builder)
                  builder(record, use_interface);
            }
         }
      }
      reader.file_data = nullptr;
      reader.file_size = 0;
      reader.loader    = nullptr;
      this->_add_one_way_outbound_reference(this->groupInfo.parentFormID, use_info_entry::flag::i_am_child_of);
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
            it->second.other->receive_inbound_ref(this, flags);
      }
   }
   void form_stub::receive_inbound_ref(form_stub* inbound, use_info_entry::flags_t flags) noexcept {
      auto& list  = this->inbound;
      auto& entry = list[inbound->formID];
      entry.other = inbound;
      entry.refcount++;
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

   bool form_stub::has_child_forms() const noexcept {
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_parent_of)
            return true;
      }
      return false;
   }
   bool form_stub::has_child_forms_of_group(uint8_t gt) const noexcept {
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (!entry.other)
            continue;
         if (!(entry.flags & use_info_entry::flag::i_am_parent_of))
            continue;
         if (entry.other->groupInfo.type == gt)
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
         if (entry.other->formID == this->groupInfo.parentFormID)
            return entry.other;
      }
      return nullptr;
   }
   bool form_stub::is_any_descendant_form_edited() const noexcept {
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
   bool form_stub::does_descendant_form_need_save() const noexcept {
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
   bool form_stub::needs_save() const noexcept {
      if (this->is_edited())
         return true;
      auto& owner = this->_get_load_order();
      if (owner.is_defined_or_overridden_in_active_file(*this))
         return true;
      return this->does_descendant_form_need_save();
   }
   #pragma endregion

   bool form_stub::is_exterior_cell() const noexcept {
      if (this->formType != form_type::cell)
         return false;
      return this->groupInfo.parentFormID != 0;
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
   uint32_t form_stub::get_cell_block() const noexcept {
      if (this->formType != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         _cell_grid_dword value;
         value.x = this->groupInfo.gridX / 8 / 4;
         value.y = this->groupInfo.gridY / 8 / 4;
         return value.merged;
      }
      return (this->formID % 10);
   }
   uint32_t form_stub::get_cell_sub_block() const noexcept {
      if (this->formType != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         _cell_grid_dword value;
         value.x = this->groupInfo.gridX / 8;
         value.y = this->groupInfo.gridY / 8;
         return value.merged;
      }
      return (this->formID % 100) / 10;
   }

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
      using outbound_type = use_info_entry::outbound_type;
      using use_flag      = use_info_entry::flag;
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
      new_stub->receive_inbound_ref(this, use_info_entry::invert_flags(flags));
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