#include "form_stub.h"
#include <cassert>
#include "../helpers/bitwise.h"
#include "files/tes_file_reading/basic_reader.h"
#include "files/tes_file_reading/file.h"
#include "forms/factories/construct.h"
#include "forms/factories/use_info.h"
#include "forms/Form.h"
#include "form_stub_heap.h"
#include "logging.h"

namespace dovah {
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
   loaded_form_ptr<loaded_forms::Form> form_stub::load() {
      if (!this->form && this->file) {
         auto file = this->file;
         //
         // TODO: if the file's load_order is in the middle of a save operation, then 
         // do not try to load the form.
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
      if (this->is_hardcoded()) { // hardcoded forms only have hardcoded outbound refs
         //
         // TODO: [ACHR:00000014]PlayerRef is a hardcoded form and should be given a 
         // ref to its ActorBase, no?
         //
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
         use_info_entry::flags_t flags = 0;
         if (it->second.flags & use_info_entry::flag::i_am_child_of)
            flags |= use_info_entry::flag::i_am_parent_of;
         else if (it->second.flags & use_info_entry::flag::i_am_parent_of)
            flags |= use_info_entry::flag::i_am_child_of;
         if (it->second.flags & use_info_entry::flag::i_am_base_form_of)
            flags |= use_info_entry::flag::i_am_reference_of;
         else if (it->second.flags & use_info_entry::flag::i_am_reference_of)
            flags |= use_info_entry::flag::i_am_base_form_of;
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

   void form_stub::add_outbound_reference(uint32_t toFormID, use_info_entry::flags_t flags) {
      if (toFormID == 0)
         return;
      auto& list  = this->outbound;
      auto& entry = list[toFormID];
      auto& lo    = this->_get_load_order();
      if (!entry.other)
         entry.other = lo.get_form(toFormID);
      #ifdef _DEBUG
         if (!entry.other && /*toFormID >= 0x800 &&*/ toFormID != 0x02006718) { // exclude known dangling ref in Dawnguard.esm
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