#include "./form_stub.h"
#include <cassert>
#include "lu/bit/edit_bit.h"
#include "./_passkeys/add_form_stub_source_file.h"
#include "./form_data/base_managed_form_data.h"
#include "./form_stub_addenda.h"
#include "./tes/file.h"
#include "./tes/reading/file_view.h"
#include "./tes/record_flags.h"
#include "./types/file_prefix.h"
#include "./use_info/entry_flag_to_mask.h"
#include "./use_info/entry_flags/base.h"
#include "./utils/form_stubs/can_ever_be_edited.h"
#include "./active_load_order.h"

namespace dovah {
   form_stub::form_stub() {
      this->file = {};
   }
   form_stub::~form_stub() {
      if (this->refcount) {
         #if _DEBUG
            __debugbreak(); // Something is still using this form_stub's loaded data. Why are you destroying it?
         #endif
         this->loaded = nullptr;
      } else if (auto* form = this->loaded) { // needed for edited forms, hardcoded forms, and other forms that aren't normally allowed to unload
         this->loaded = nullptr;
         delete form;
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

   const active_load_order& form_stub::get_load_order() const {
      auto* file = this->get_file_at_index(0);
      assert(file && "Cannot retrieve a form stub's owning load order if it has no files!");
      auto* alo  = file->get_active_load_order();
      assert(!!alo);
      return *alo;
   }
   active_load_order& form_stub::get_load_order() {
      return const_cast<active_load_order&>(std::as_const(*this).get_load_order());
   }

   bool form_stub::can_unload_form() const noexcept {
      if (this->is_edited())
         return false;
      return true;
   }
   bool form_stub::is_injected() const noexcept {
      auto* first_file = this->get_file_at_index(0);
      if (first_file) {
         //
         // Always double-check the form's first file, if there is one, because that file might be one 
         // of the "dummy" files used for things like none-stubs and hardcoded forms. We don't want 
         // those forms to test as "injected."
         //
         if (first_file->app_flags().has_any_of<tes::file_app_specific_flag::is_hardcoded_dummy, tes::file_app_specific_flag::is_none_stub_dummy>())
            return false;
      } else {
         return false;
      }
      //
      // Okay, we know that the form isn't contributed to by a "dummy" file. Let's grab the file that 
      // we would *expect* the form to be defined in based on its form ID, and see if that file actually 
      // contributes to the form. If not, then the form has been injected into that file.
      //
      auto& lo            = this->get_load_order();
      auto  prefix        = file_prefix::from_form_id(this->form_id, lo.get_current_game() == game::skyrim_classic);
      auto* expected_file = lo.get_file_by_prefix(prefix);
      return expected_file == first_file;
   }
   bool form_stub::is_none_stub() const noexcept {
      auto* info = this->get_source_file_info(-1); // get last file
      if (!info || !info->pointer)
         return false;
      return info->pointer->app_flags().has_all_of<tes::file_app_specific_flag::is_none_stub_dummy>();
   }
   void form_stub::set_edited(bool v) {
      if (this->is_edited() == v)
         return;
      if (v) {
         assert(utils::form_stubs::can_ever_be_edited(*this) && "Should not flag form stubs as \"edited\" if they cannot actually be edited!");
      }
      if (v)
         this->flags.set_each_of<flag::is_edited>();
      else
         this->flags.unset_each_of<flag::is_edited>();
      if (v) {
         this->get_load_order()._get_form_stub_collection({})._on_stub_flagged_as_edited({}, *this);
      } else {
         if (this->refcount == 0)
            this->_unload_form();
      }
   }

   #pragma region Record flag functions
      void form_stub::edit_record_flags(uint32_t mask, bool clear_or_set) noexcept {
         constexpr const uint32_t flag_mask_partial = (uint32_t)1 << (uint32_t)tes::record_flag::partial;

         if (!mask)
            return;
         auto* info = const_cast<file_data*>(this->get_source_file_info());
         if (!info)
            return;
         auto& lo     = this->get_load_order();
         auto* active = const_cast<tes::file*>(lo.get_active_file()); // HACK HACK HACK
         if (!active)
            return;
         if (info->pointer == active) {
            lu::edit_bit(info->record_flags, mask, clear_or_set);
            if (!(mask & flag_mask_partial) || !clear_or_set) { // if we're not explicitly setting the "partial" flag for some reason
               //
               // Modifying the flags in any respect will also flag the form as "edited," which 
               // means it won't save as partial... which means it effectively *is* no longer 
               // partial, and keeping the flag will only cause confusion for the frontend.
               //
               info->record_flags &= ~flag_mask_partial;
            }
            return;
         }
         auto flags = info->record_flags;
         lu::edit_bit(flags, mask, clear_or_set);
         this->set_edited(true);
         this->_add_source_file(*active, 0, flags);
      }
   #pragma endregion

   //
   // protected
   //

   tes::file_view form_stub::_open_record(const tes::file& file, uint32_t record_offset) const noexcept {
      return tes::file_view::open_view_to_record(file, record_offset);
   }
   void form_stub::_unload_form() {
      if (!this->can_unload_form())
         return;
      auto form = this->loaded;
      if (form) {
         delete form;
         this->loaded = nullptr;
      }
   }
      
   #pragma region Source file member functions (internal)
      void form_stub::_add_source_file(tes::file& f, uint32_t record_offset, uint32_t record_flags) {
         if (!this->has_multiple_source_files()) {
            if (!this->file) {
               this->file.pointer       = &f;
               this->file.record_offset = record_offset;
               this->file.record_flags  = record_flags;
               return;
            }
            auto prior = this->file;
            this->flags.set_each_of<flag::has_multiple_source_files>();
            this->files.entries = new file_data[2];
            this->files.count   = 2;
            this->files.entries[0] = prior;
            auto& next = this->files.entries[1];
            next.pointer = &f;
            next.record_offset = record_offset;
            next.record_flags  = record_flags;
            return;
         }
         assert(this->files.entries);
         auto resized = new file_data[size_t(this->files.count) + 1]; /// cast silences warning C26451 and is otherwise pointless
         memcpy(resized, this->files.entries, sizeof(file_data) * this->files.count);
         auto& last = resized[this->files.count];
         last.pointer       = &f;
         last.record_offset = record_offset;
         last.record_flags  = record_flags;
         delete[] this->files.entries;
         this->files.entries = resized;
         ++this->files.count;
      }
   #pragma endregion

   //
   // passkeyed
   //
      
   void form_stub::_add_source_file(_passkeys::add_form_stub_source_file, tes::file& f, uint32_t record_offset, uint32_t record_flags) {
      this->_add_source_file(f, record_offset, record_flags);
   }
   void form_stub::_add_one_way_outbound_use(_passkeys::build_use_info_during_load, form_stub& to_stub, use_info::entry_flag_underlying_type flags) {
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
      auto& list  = this->outbound;
      auto& entry = list[to_stub.form_id];
      if (!entry.other)
         entry.other = &to_stub;
      entry.refcount++;
      //
      if (flags)
         entry.flags |= flags;
   }
   void form_stub::_add_one_way_outbound_use(_passkeys::build_use_info_during_load, bare_form_id used_form_id, use_info::entry_flag_underlying_type flags) {
      //
      // Please refer to the documentation comments in this function's other overload.
      //
      if (used_form_id == 0)
         return;
      auto& list  = this->outbound;
      auto& entry = list[used_form_id];
      if (!entry.other) {
         entry.other = this->get_load_order().get_form_stub_collection().get_form_by_id(used_form_id, false);
      }
      entry.refcount++;
      //
      if (flags) {
         entry.flags |= flags;
      }
   }
   void form_stub::_set_parent_form_one_way(_passkeys::build_use_info_during_load, form_stub& parent) {
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
            if (entry.other == &parent)
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
      this->_add_one_way_outbound_use({}, parent, use_info::entry_flag_to_mask(use_info::entry_flags::base::parent));
   }
}