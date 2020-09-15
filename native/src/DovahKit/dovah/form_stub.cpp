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
      if (this->get_refcount())
         assert(!this->form && "You should not be attempting to destroy a FormStub when something is still using its loaded form data!");
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
   void form_stub::set_edited(bool v) {
      cobb::modify_bit(this->flags, flag::is_edited, v);
      if (v)
         this->_get_load_order().stub_flagged_as_edited(this);
      else {
         if (this->refcount == 0)
            this->_unload_form();
      }
   }

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
      this->add_outbound_reference(this->groupInfo.parentFormID);
   }
   void form_stub::send_inbound_refs() noexcept {
      for (auto it = this->outbound.begin(); it != this->outbound.end(); ++it)
         if (it->second.other)
            it->second.other->receive_inbound_ref(this);
   }
   void form_stub::receive_inbound_ref(form_stub* inbound, form_stub::flags_t flags) noexcept {
      auto& list  = this->inbound;
      auto& entry = list[inbound->formID];
      entry.other = inbound;
      entry.refcount++;
      entry.flags = flags;
   }

   void form_stub::add_outbound_reference(uint32_t toFormID, form_stub::flags_t flags) {
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
         if (flags & use_info_entry::flag::i_am_child_of)
            entry.flags |= use_info_entry::flag::i_am_parent_of;
         else if (flags & use_info_entry::flag::i_am_parent_of)
            entry.flags |= use_info_entry::flag::i_am_child_of;
      }
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