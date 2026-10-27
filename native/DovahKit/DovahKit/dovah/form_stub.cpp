#include "./form_stub.h"
#include <cassert>
#include <limits>
#include "lu/bit/edit_bit.h"
#include "./_passkeys/add_form_stub_source_file.h"
#include "./active_load_order.h"
#include "./form_data/base_managed_form_data.h"
#include "./form_data_pointer.h"
#include "./form_data/helpers/load_hardcoded_form_data.h"
#include "./form_data_function_tables/create_blank_instance.h"
#include "./form_data_function_tables/load.h"
#include "./form_stub_addenda.h"
#include "./load_order_interfaces/form_load.h"
#include "./tes/file.h"
#include "./tes/reading/file_view.h"
#include "./tes/record_flags.h"
#include "./types/file_prefix.h"
#include "./use_info/entry_flag_to_mask.h"
#include "./use_info/entry_flags/base.h"
#include "./utils/form_stubs/can_ever_be_edited.h"
#include "./utils/form_stubs/can_load_form_data.h"

namespace {
   using form_stub_heap = lu::memory::threaded_slab_heap<dovah::form_stub, 16000>;
}

namespace dovah {
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

   form_stub_addenda& form_stub::get_or_create_addenda() {
      if (this->addenda)
         return *this->addenda;
      this->addenda = new form_stub_addenda;
      return *this->addenda;
   }

   untyped_form_data_pointer form_stub::load() {
      if (this->loaded)
         return untyped_form_data_pointer::_spawn({}, *this);
      if (!utils::form_stubs::can_load_form_data(*this))
         return untyped_form_data_pointer::_spawn({}, *this);
      
      auto& alo = this->get_load_order();

      file_data* files;
      uint16_t   file_count;
      if (this->has_multiple_source_files()) {
         files      = this->files.entries;
         file_count = this->files.count;
         assert(file_count > 0);
      } else {
         files      = &this->file;
         file_count = 1;
      }
      
      auto intfc   = load_order_interfaces::form_load(*this);
      auto loader  = form_data_function_tables::load::lookup(this->form_type);
      assert(!!loader);  // should've been checked by `can_load_form_data`
      auto factory = form_data_function_tables::create_blank_instance::lookup(this->form_type);
      assert(!!factory); // should've been checked by `can_load_form_data`

      this->loaded = &((factory)(*this));
      assert(!!this->loaded);

      uint16_t first_file_to_load_from = 0;
      if (auto* file = files[0].pointer) {
         if (file->app_flags().has_all_of<tes::file_app_specific_flag::is_hardcoded_dummy>()) {
            form_data_helpers::load_hardcoded_form_data(*this->loaded);
            ++first_file_to_load_from;
         }
      }

      uint16_t winning_record_index = file_count - 1;
      if (winning_record_index > 0 && files[winning_record_index].record_offset == 0) {
         //
         // This can happen if the form's record flags have been altered (such that the active 
         // file was added to the stub's file list), but the form has otherwise not been edited.
         //
         --winning_record_index;
      }

      const bool can_be_parent = form_type_info::lookup(this->form_type).flags & form_type_info::flag::can_have_children;
      
      uint32_t last_record_flags = files[0].record_flags;
      for (uint16_t i = first_file_to_load_from; i < file_count; ++i) {
         if (files[i].record_offset == 0) // file has no actual data (unsaved new active file, etc.). skip it
            continue;
         auto* file = files[i].pointer;
         
         intfc.is_winning_record = i == winning_record_index;
         intfc.current_file      = file;
         intfc.is_partial_record = can_be_parent && i > first_file_to_load_from && (files[i].record_flags & ((uint32_t)1 << (size_t)tes::record_flag::partial));
         intfc.last_record_flags = last_record_flags;

         #pragma warning(suppress:28182) // suppress spurious IntelliSense warning: `file` should never be nullptr
         auto file_view = _open_record(*file, files[i].record_offset);
         (loader)(*this->loaded, file_view.subviews.record, intfc);

         last_record_flags = files[i].record_flags;
      }

      return untyped_form_data_pointer::_spawn({}, *this);
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
      
   #pragma region Use info (internal)
      void form_stub::_sever_outbound_use(form_stub& used, use_info::entry_flag_underlying_type flags_mask) {
         auto user_it = this->outbound.find(used.form_id);
         if (user_it == this->outbound.end()) {
            #if _DEBUG
            {
               auto used_it = used.inbound.find(this->form_id);
               assert(used_it == used.inbound.end() && "If we don't track an outbound use to a form, they shouldn't track an inbound use from us!");
            }
            #endif
            return;
         }
         auto used_it = used.inbound.find(this->form_id);
         #if _DEBUG
            assert(used_it != used.inbound.end() && "If we track an outbound use to a form, they should track an inbound use from us!");
         #endif

         auto& user_entry = user_it->second;
         auto& used_entry = used_it->second;
         user_entry.flags &= ~flags_mask;
         used_entry.flags &= ~flags_mask;
         --user_entry.refcount;
         --used_entry.refcount;
         if (user_entry.refcount == 0) {
            assert(used_entry.refcount == 0 && "Refcounts for a bidirectionally-tracked use should be equal on both sides!");
            this->outbound.erase(user_it);
            used.inbound.erase(used_it);
         }
      }
      void form_stub::_build_outbound_use(form_stub& used, use_info::entry_flag_underlying_type flags_mask) {
         auto& user_entry = this->outbound[used.form_id];
         auto& used_entry = used.outbound[this->form_id];
         #if _DEBUG
            assert(user_entry.refcount == used_entry.refcount && "Refcounts for a bidirectionally-tracked use should be equal on both sides!");
            assert(user_entry.flags    == used_entry.flags && "Flags for a bidirectionally-tracked use should be equal on both sides!");
         #endif
         if (++user_entry.refcount == 1) {
            user_entry.other = &used;
            used_entry.other = this;
         }
         ++used_entry.refcount;
         user_entry.flags |= flags_mask;
         used_entry.flags |= flags_mask;
      }
   #pragma endregion

   //
   // passkeyed
   //

   #pragma region During initial load
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
      void form_stub::_amend_source_file_flag(_passkeys::add_form_stub_source_file, const tes::file& file, tes::record_flag flag, bool clear_or_set) {
         const uint32_t mask = (uint32_t)1 << (size_t)flag;
         if (this->has_multiple_source_files()) {
            auto& info = this->file;
            if (info.pointer != &file) [[unlikely]]
               return;
            if (clear_or_set)
               info.record_flags |= mask;
            else
               info.record_flags &= ~mask;
         } else {
            for (uint16_t i = 0; i < this->files.count; ++i) {
               auto& info = this->files.entries[i];
               if (info.pointer == &file) {
                  if (clear_or_set)
                     info.record_flags |= mask;
                  else
                     info.record_flags &= ~mask;
                  break;
               }
            }
         }
      }
      void form_stub::_replace_source_file_list(_passkeys::add_form_stub_source_file, std::vector<file_data>&& list) {
         if (this->has_multiple_source_files())
            delete[] this->files.entries;
         auto size = list.size();
         if (size < 2) {
            this->flags.unset_each_of<flag::has_multiple_source_files>();
            this->file = {
               .pointer       = nullptr,
               .record_offset = 0,
               .record_flags  = 0,
            };
            if (size) {
               this->file = list[0];
            }
            return;
         }
         assert(size <= std::numeric_limits<uint16_t>::max());
         this->flags.set_each_of<flag::has_multiple_source_files>();
         this->files = {
            .entries = new file_data[size],
            .count   = (uint16_t)size,
         };
         for (uint16_t i = 0; i < size; ++i) {
            auto& f = this->files.entries[i];
            auto& o = list[i];
            f = o;
         }
      }
      void form_stub::_set_parent_form_one_way(_passkeys::build_use_info_during_load, form_stub* parent) {
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
         if (parent)
            this->_add_one_way_outbound_use({}, *parent, use_info::entry_flag_to_mask(use_info::entry_flags::base::parent));
      }
   #pragma endregion

   void form_stub::_replace_outbound_use(lu::passkey<managed_form_use>, form_stub* old_used, form_stub* now_used, use_info::entry_flag_underlying_type flags_mask) {
      if (old_used)
         this->_sever_outbound_use(*old_used, flags_mask);
      if (now_used)
         this->_build_outbound_use(*now_used, flags_mask);
   }
}