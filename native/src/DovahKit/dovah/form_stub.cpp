#include "form_stub.h"
#include <cassert>
#include "../helpers/bitwise.h"
#include "files/tes_file_reading/basic_reader.h"
#include "files/tes_file_reading/file.h"
#include "forms/factories/construct.h"
#include "forms/factories/hardcoded.h"
#include "forms/factories/use_info.h"
#include "forms/Form.h"
#include "form_stub_heap.h"
#include "logging.h"

namespace dovah {
   /*static*/ use_info_entry::flags_t use_info_entry::invert_flags(use_info_entry::flags_t f) {
      flags_t flags = 0;
      //
      if (f & use_info_entry::flag::i_am_child_of)
         flags |= use_info_entry::flag::i_am_parent_of;
      else if (f & use_info_entry::flag::i_am_parent_of)
         flags |= use_info_entry::flag::i_am_child_of;
      //
      if (f & use_info_entry::flag::i_am_base_form_of)
         flags |= use_info_entry::flag::i_am_reference_of;
      else if (f & use_info_entry::flag::i_am_reference_of)
         flags |= use_info_entry::flag::i_am_base_form_of;
      //
      return flags;
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
   void form_stub::get_source_filename(std::string& out) const noexcept {
      out.clear();
      if (this->file)
         out = this->file->get_filename();
   }
   file_load_order& form_stub::_get_load_order() const noexcept {
      return this->file->load_order;
   }
   loaded_form_ptr<loaded_forms::Form> form_stub::_load(bool force) {
      if (!this->form && this->file) {
         auto file = this->file;
         //
         if (!force) {
            //
            // Don't try to load the form if the file's load order is in the middle of 
            // a save operation, or if loading is otherwise unsafe.
            //
            auto& lo = this->_get_load_order();
            if (lo.is_form_loading_blocked(this))
               return loaded_form_ptr<loaded_forms::Form>(this);
         }
         //
         if (this->file->load_record_at(this->offset)) {
            auto& record   = this->file->get_current_record();
            auto  formType = form_type_info::signature_to_form_type(record.signature());
            auto  factory  = get_loaded_form_factory_by_type(formType);
            if (factory) {
               this->form = factory(record);
               this->form->stub = this;
            }
         } else
            dovah::logging::print_line("...stub failed.");
      }
      return loaded_form_ptr<loaded_forms::Form>(this);
   }
   loaded_form_ptr<loaded_forms::Form> form_stub::get_content_if_loaded() {
      return loaded_form_ptr<loaded_forms::Form>(this);
   }
   bool form_stub::fetch_record_header(tes_file_record_header& out, uint32_t& out_record_decompressed_size) const noexcept {
      if (!this->file || this->is_non_overridden_hardcoded_form())
         return false;
      return this->file->fetch_record_header(this->offset, out, out_record_decompressed_size);
   }
   bool form_stub::can_unload_form() const noexcept {
      if (this->is_edited())
         return false;
      if (this->is_non_overridden_hardcoded_form())
         return false;
      return true;
   }
   bool form_stub::is_non_overridden_hardcoded_form() const noexcept {
      if (this->is_hardcoded()) {
         if (!this->file)
            return true;
         if (this->file->header.details & owner_file_t::detail_flag::is_hardcoded_dummy) // allow overrides of hardcoded forms to unload
            return true;
      }
      return false;
   }
   void form_stub::set_edited(bool v) {
      cobb::modify_bit(this->flags, flag::is_edited, v);
      if (v)
         this->_get_load_order().stub_flagged_as_edited(this);
      else {
         if (this->refcount == 0)
            this->_unload_form();
      }
   }

   #pragma region form_stub use info functions
   void form_stub::build_outbound_refs(tes_file_reading::basic_reader* reader) noexcept {
      if (this->is_non_overridden_hardcoded_form()) { // hardcoded forms only have hardcoded outbound refs
         build_hardcoded_form_outbound_refs(*this);
         return;
      }
      assert(this->file && "FormStub cannot build outbound refs without a file. How did this happen?");
      if (this->file->load_record_at(this->offset, reader)) {
         auto& record = reader->get_current_record();
         auto  builder = get_outbound_uses_builder_by_type(this->formType);
         if (builder)
            builder(record, this);
      }
      this->add_outbound_reference(this->groupInfo.parentFormID, use_info_entry::flag::i_am_child_of);
   }
   void form_stub::send_inbound_refs() noexcept {
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

   void form_stub::add_outbound_reference(form_stub* to_stub, use_info_entry::flags_t flags) {
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
   void form_stub::add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags) {
      if (toFormID == 0)
         return;
      auto& list  = this->outbound;
      auto& entry = list[toFormID];
      auto& lo    = this->_get_load_order();
      if (!entry.other)
         entry.other = lo.get_form(toFormID);
      #ifdef _DEBUG
         //
         // When compiled in Debug, warn on any use of a missing form.
         //
         if (!entry.other
            && toFormID != 0x02006718 // Dawnguard.esm
            && toFormID != 0x00106633 // Skyrim.esm: CELL/XCLR has references to a non-existent REGN
         ) {
            if (lo.has_form(toFormID))
               __debugbreak();
            else
               __debugbreak();
         }
      #endif
      entry.refcount++;
      //
      if (flags) {
         entry.flags |= flags;
      }
   }

   bool form_stub::has_child_forms() const noexcept {
      for (auto& pair : this->outbound) {
         auto& entry = pair.second;
         if (entry.flags & use_info_entry::flag::i_am_parent_of)
            return true;
      }
      return false;
   }
   bool form_stub::has_child_forms_of_group(uint8_t gt) const noexcept {
      for (auto& pair : this->outbound) {
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
      for (auto& pair : this->outbound) {
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
      for (auto& pair : this->outbound) {
         auto& entry = pair.second;
         if (!(entry.flags & use_info_entry::flag::i_am_parent_of))
            continue;
         auto stub = entry.other;
         if (owner.is_defined_or_overridden_in_active_file(stub))
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
      if (owner.is_defined_or_overridden_in_active_file(this))
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
   void form_stub::replace_outbound_reference(bare_form_id_t old, form_stub* new_stub, use_info_entry::flags_t flags) {
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
      //
      this->add_outbound_reference(new_stub, flags);
      new_stub->receive_inbound_ref(this, use_info_entry::invert_flags(flags));
   }
   void form_stub::replace_outbound_reference(bare_form_id_t old, bare_form_id_t change_to, use_info_entry::flags_t flags) {
      auto& lo       = this->_get_load_order();
      auto* new_stub = lo.get_form(change_to);
      if (!new_stub)
         return;
      this->replace_outbound_reference(old, new_stub, flags);
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