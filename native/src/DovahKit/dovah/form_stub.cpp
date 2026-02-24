#include "form_stub.h"
#include <cassert>
#include <optional>
#include "../helpers/vectors/move_item_within.h"
#include "../helpers/bitwise.h"
#include "load_order_interfaces/form_load.h"
#include "files/tes_file_reading/file_loader.h"
#include "forms/factories/construct.h"
#include "forms/factories/hardcoded.h"
#include "forms/factories/use_info.h"
#include "forms/Form.h"
#include "use_info/entry_flags/base.h"
#include "use_info/entry_flag_to_mask.h"
#include "utils/file_prefix.h"
#include "./form_stub_addenda/passkeys/ordered_child_collection.h"
#include "form_stub_addenda.h"
#include "form_stub_heap.h"
#include "form_stub_use_info_builder.h"
#include "logging.h"

namespace {
   //
   // When DovahKit is finished, every form type should be implemented, which means we should 
   // be able to skim records and generate use info for every form stub. This, by definition, 
   // would entail looking at every subrecord in each form's record.
   // 
   // Until all form types are implemented, this variable controls whether we want to simulate 
   // some of the processing overhead involved: if a form type doesn't support generating use 
   // info, but this variable is true, then we'll open all of the record's subrecords anyway.
   //
   constexpr const bool dummy_parse_subrecords_when_use_info_isnt_possible =
      #if _DEBUG
         true
      #else
         false
      #endif
   ;
}

namespace dovah {
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
      if (this->form_type == form_type::setting || this->form_type == form_type::none)
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
      if (force) {
         intfc.is_during_file_save_cleanup = true;
      }
      auto* loader = get_form_loader_function(this->form_type);
      if (!loader)
         return loaded_form_ptr<loaded_forms::Form>(this); // load failed
      bool can_be_parent = form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children;
      if (auto* file = arr[0].pointer) {
         if (file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) {
            //
            // This file is the hardcoded dummy file. We'll need to instantiate the loaded-form 
            // class a bit differently, since hardcoded forms don't come from a "real" file.
            //
            this->form = instantiate_hardcoded_form(*this);
            assert(this->form != nullptr && "If this assertion fails, then we have an unimplemented hardcoded form!");
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
               this->form = create_blank_loaded_form_by_type(this->form_type, fcp);
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
   void form_stub::_do_custom_parse_impl(tes_file_reading::basic_reader* reader, custom_parse_functor_type loader) noexcept {
      if (!this->has_source_files())
         return; // no files to load from
      //
      auto& lo = this->_get_load_order();
      //
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      if (!size)
         return; // no source files (this should never occur; it is only possible while the stub is being built)
      //
      auto     intfc             = load_order_interfaces::form_load(lo, *this);
      bool     can_be_parent     = form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children;
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

   loaded_form_ptr<loaded_forms::Form> form_stub::load_even_if_unsafe(form_stub_passkeys::force_form_load) {
      return this->_load(true);
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
   void form_stub::do_custom_parse(tes_file_reading::basic_reader* reader, custom_parse_functor_type loader) noexcept {
      if (this->_get_load_order().is_form_loading_blocked(this)) // don't allow a load if the file's load order is in the middle of a save operation or some other unsafe circumstance
         return;
      this->_do_custom_parse_impl(reader, loader);
   }

   void form_stub::do_custom_parse_during_load(
      form_stub_passkeys::do_custom_parse_during_load,
      tes_file_reading::basic_reader* reader,
      custom_parse_functor_type loader
   ) noexcept {
      this->_do_custom_parse_impl(reader, loader);
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
      assert(this->files.entries);
      auto resized = new file_data[size_t(this->files.count) + 1]; /// cast silences warning C26451 and is otherwise pointless
      memcpy(resized, this->files.entries, sizeof(file_data) * this->files.count);
      auto& last = resized[this->files.count];
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
      this->files = {
         .entries = resized,
         .count   = (int16_t)(count_a + count_b)
      };
      this->flags |= flag::has_multiple_source_files;
   }
   void form_stub::_set_source_file_list(const std::vector<file_data>& list) {
      if (this->has_multiple_source_files())
         delete[] this->files.entries;
      auto size = list.size();
      if (size < 2) {
         this->flags &= ~flag::has_multiple_source_files;
         this->file = {
            .pointer = nullptr,
            .offset  = 0,
            .flags   = 0,
         };
         if (size) {
            this->file.offset  = list[0].offset;
            this->file.pointer = list[0].pointer;
            this->file.flags   = list[0].flags;
         }
         return;
      }
      this->flags |= flag::has_multiple_source_files;
      this->files = {
         .entries = new file_data[size],
         .count   = (int16_t)size,
      };
      for (uint16_t i = 0; i < size; ++i) {
         auto& f = this->files.entries[i];
         auto& o = list[i];
         f.offset  = o.offset;
         f.pointer = o.pointer;
         f.flags   = o.flags;
      }
   }
   #pragma endregion

   bool form_stub::can_unload_form() const noexcept {
      if (this->is_edited())
         return false;
      return true;
   }
   file_load_order& form_stub::get_owning_load_order() const noexcept {
      return this->_get_load_order();
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
   void form_stub::edit_record_flags(uint32_t mask, bool clear_or_set) noexcept {
      if (!mask)
         return;
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
      auto flags = info->flags;
      cobb::edit_bit(flags, mask, clear_or_set);
      this->set_edited(true);
      this->_add_file(*active, 0, flags);
   }
   #pragma endregion

   #pragma region form_stub use info functions
   void form_stub::build_outbound_refs(form_stub_passkeys::build_use_info_during_load, tes_file_reading::basic_reader& reader) noexcept {
      file_data* arr;
      uint16_t   size;
      this->_get_source_file_list(arr, size);
      //
      form_stub_use_info_builder use_interface(*this);
      bool can_be_parent = form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children;
      auto& lo = this->_get_load_order();
      //
      for (uint16_t i = 0; i < size; ++i) {
         if (i == size - 1)
            use_interface._is_final_file = true;
         auto* file = arr[i].pointer;
         use_interface._is_active_file = lo.file_is_active(*file);
         use_interface._is_base_record = i == 0;
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
               auto  builder = get_outbound_uses_builder_by_type(this->form_type);
               if (builder) {
                  builder(record, use_interface);
                  use_interface.commit();

                  bool didnt_read_anything = record.current_offset() == 0 && !record.get_current_subrecord();
                  bool read_everything     = !record.is_in_bounds();

                  assert((didnt_read_anything || read_everything) && "A form type's `generate_use_info` function should either read nothing (not even opening any subrecords), or read the whole record! (Reading nothing is fine if the form type in question can't refer to any other forms.)");
               } else {
                  if constexpr (dummy_parse_subrecords_when_use_info_isnt_possible) {
                     while (auto& subrecord = record.next_subrecord()) {
                        ;
                     }
                  }
               }
            }
         }
      }
      reader.file_data = nullptr;
      reader.file_size = 0;
      reader.loader    = nullptr;
   }
   void form_stub::send_inbound_refs(form_stub_passkeys::build_use_info_during_load) noexcept {
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
         if (it->second.other)
            it->second.other->receive_inbound_ref({}, this, it->second.refcount, it->second.flags);
      }
   }
   void form_stub::receive_inbound_ref(form_stub_passkeys::build_use_info_during_load, form_stub* inbound, uint32_t refcount, use_info::entry_flag_underlying_type flags) noexcept {
      auto& list  = this->inbound;
      auto& entry = list[inbound->formID];
      entry.other     = inbound;
      entry.refcount += refcount;
      entry.flags    |= flags;
   }

   void form_stub::_add_one_way_outbound_reference(form_stub_passkeys::build_use_info_during_load, form_stub* to_stub, use_info::entry_flag_underlying_type flags) {
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
   void form_stub::_add_one_way_outbound_reference(form_stub_passkeys::build_use_info_during_load, uint32_t toFormID, use_info::entry_flag_underlying_type flags) {
      //
      // Please refer to the documentation comments in this function's other overload.
      //
      if (toFormID == 0)
         return;
      auto& list  = this->outbound;
      auto& entry = list[toFormID];
      if (!entry.other)
         entry.other = this->_get_load_order().get_form(toFormID, false);
      entry.refcount++;
      //
      if (flags) {
         entry.flags |= flags;
      }
   }
   void form_stub::_set_parent_form_one_way(form_stub_passkeys::build_use_info_during_load, form_stub* parent) {
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
         if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)) {
            if (entry.other == parent)
               return;
            entry.flags &= ~use_info::entry_flag_to_mask(use_info::entry_flags::base::parent);
            if (--entry.refcount == 0)
               list.erase(it);
            break;
         }
      }
      //
      // Set the new parent form.
      //
      this->_add_one_way_outbound_reference({}, parent, use_info::entry_flag_to_mask(use_info::entry_flags::base::parent));
   }
   #pragma endregion

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
      if (!this->addenda || !this->addenda->grid_position.has_value())
         return false;
      auto& gc = this->addenda->grid_position.value();
      x = gc.x;
      y = gc.y;
      return true;
   }
   size_t form_stub::child_info_count() const noexcept {
      if (this->form_type != form_type::topic)
         return 0;
      if (!this->addenda)
         return 0;
      return this->addenda->ordered_children.get_active_list().size();
   }
   size_t form_stub::index_of_child_info(form_stub& info) const noexcept {
      if (this->form_type != form_type::topic || info.form_type != form_type::topic_info)
         return std::string::npos;
      if (!this->addenda)
         return std::string::npos;
      auto&  list = this->addenda->ordered_children.get_active_list();
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == &info)
            return i;
      return std::string::npos;
   }
   #pragma endregion

   #pragma region Form stub parenthood functions
   form_stub* form_stub::get_parent_form() const noexcept {
      for (auto& pair : this->outbound) {
         auto& entry = pair.second;
         if (!entry.other)
            continue;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
            continue;
         return entry.other;
      }
      return nullptr;
   }
   bool form_stub::has_child_forms() const noexcept {
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent))
            return true;
      }
      return false;
   }
   bool form_stub::is_parent_form_of(form_stub& child) const noexcept {
      auto& map = this->inbound;
      for (auto it = map.begin(); it != map.end(); ++it) {
         auto& pair = *it;
         if (pair.first != child.formID)
            continue;
         if (pair.second.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent))
            return true;
      }
      return false;
   }

   void form_stub::orphan() {
      auto* parent = this->get_parent_form();
      if (!parent)
         return;
      this->revoke_outbound_reference(parent, use_info::entry_flag_to_mask(use_info::entry_flags::base::parent));
      if (this->form_type == form_type::topic_info && parent->form_type == form_type::topic) {
         if (auto* addenda = parent->addenda)
            addenda->ordered_children._remove_child_after_load({}, *this);
      }
   }
   void form_stub::set_parent_form(form_stub* target) noexcept {
      //
      // This function is only used after files have been loaded. During file load, we 
      // use the `_set_parent_form_one_way` function instead.
      //
      auto* parent = this->get_parent_form();
      if (target == parent)
         return;
      this->orphan();
      if (!target)
         return;
      this->replace_outbound_reference(0, target, use_info::entry_flag_to_mask(use_info::entry_flags::base::parent));
      if (target->form_type == form_type::topic && this->form_type == form_type::topic_info) {
         target->get_or_create_addenda().ordered_children._insert_child_after_load({}, *this, std::numeric_limits<size_t>::max());
      }
   }
   #pragma endregion

   [[nodiscard]] bool form_stub::is_any_descendant_form_edited() const noexcept {
      if (!(form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children)) {
         return false;
      }
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
            continue;
         auto stub = entry.other;
         if (stub->is_edited() || stub->is_any_descendant_form_edited())
            return true;
      }
      return false;
   }
   [[nodiscard]] bool form_stub::does_descendant_form_need_save() const noexcept {
      if (!(form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children)) {
         return false;
      }
      auto& owner = this->_get_load_order();
      for (auto& pair : this->inbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info::entry_flag_to_mask(use_info::entry_flags::base::parent)))
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
      if (this->does_descendant_form_need_save())
         return true;
      if (this->form_type == form_type::topic) {
         if (auto* addenda = this->addenda) {
            auto& list_d = addenda->ordered_children.get_master_list();
            auto& list_a = addenda->ordered_children.get_active_list();
            if (list_d != list_a) {
               //
               // No child forms were added nor reparented into these lists (else they 
               // would've tripped the `does_descendant_form_need_save` check for the 
               // "edited" flag), but child forms may have been reordered without ever 
               // actually being modified.
               //
               return true;
            }
         }
      }
      return false;
   }

   [[nodiscard]] bool form_stub::is_exterior_cell() const noexcept {
      if (this->form_type != form_type::cell)
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
      if (this->form_type != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         if (!this->addenda || !this->addenda->grid_position.has_value())
            return 0;
         return this->addenda->grid_position.value().to_cell_block();
      }
      uint32_t masked = this->formID & 0x00FFFFFF;
      if (auto* file = this->get_file_at_index(0))
         if (file->is_light())
            masked &= 0x00000FFF;
      return masked % 10;
   }
   [[nodiscard]] uint32_t form_stub::get_cell_sub_block() const noexcept {
      if (this->form_type != form_type::cell)
         return 0;
      if (this->is_exterior_cell()) {
         if (!this->addenda || !this->addenda->grid_position.has_value())
            return 0;
         return this->addenda->grid_position.value().to_cell_sub_block();
      }
      uint32_t masked = this->formID & 0x00FFFFFF;
      if (auto* file = this->get_file_at_index(0))
         if (file->is_light())
            masked &= 0x00000FFF;
      return (masked % 100) / 10;
   }

   #pragma region Functions for modifying use info
   void form_stub::revoke_outbound_reference(form_stub* target, use_info::entry_flag_underlying_type flags) {
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
            entry.flags &= ~flags;
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
   void form_stub::replace_outbound_reference(bare_form_id_t old, form_stub* new_stub, use_info::entry_flag_underlying_type flags) {
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
      this->_add_one_way_outbound_reference({}, new_stub, flags);
      new_stub->receive_inbound_ref({}, this, 1, flags);
   }
   void form_stub::replace_outbound_reference(bare_form_id_t old, bare_form_id_t change_to, use_info::entry_flag_underlying_type flags) {
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
      auto* instance = create_blank_loaded_form_by_type(this->form_type, fcp);
      if (instance) {
         auto source = this->load();
         if (!source) {
            delete instance;
            instance = nullptr;
         } else {
            source->_clone_impl(instance);
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