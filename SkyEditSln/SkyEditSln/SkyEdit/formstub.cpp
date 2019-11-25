#include "formstub.h"
#include "esp/LoadOrder.h"
#include "esp/TESPlugin.h"
#include "helpers/miscellaneous.h"
#include "output.h"
#include <algorithm>
#include <cassert>
#include <cstddef>

#include "forms/factories/use_info.h"
#include "forms/loaded/Quest.h"

FormStub::~FormStub() {
   if (this->get_refcount())
      assert(!this->form && "You should not be attempting to destroy a FormStub when something is still using its loaded form data!");
   if (this->editorID) {
      free(this->editorID);
      this->editorID = nullptr;
   }
}
char* FormStub::allocate_editor_id(size_t length) {
   if (this->editorID)
      free(this->editorID);
   this->editorID = (char*)malloc(length);
   return this->editorID;
}
void FormStub::get_source_filename(std::string& out) const noexcept {
   out.clear();
   if (this->file)
      out = this->file->getFilename();
}
loaded_form_ptr<LoadedForms::Form> FormStub::load() {
   if (!this->form && this->file) {
      //_DEBUGMSG("stub is loading...");
      auto file = this->file;
      if (this->file->loadRecordAt(this->offset)) {
         auto& record   = this->file->getCurrentRecord();
         auto  formType = signatureToFormType(record.signature());
         switch (formType) {
            case FormType::Quest:
               {
                  auto q = new LoadedForms::Quest();
                  q->load(record);
                  this->form = q;
               }; break;
         }
         if (this->form)
            this->form->stub = this;
      } else
         _DEBUGMSG("...stub failed.");
   }
   return loaded_form_ptr<LoadedForms::Form>(this);
}
void FormStub::set_edited(bool v) {
   cobb::edit_bit(this->refcount, kRefcountFlag_Edited, v);
}

void FormStub::build_outbound_refs() noexcept {
   assert(this->file && "FormStub cannot build outbound refs without a file. How did this happen?");
   if (this->file->loadRecordAt(this->offset)) {
      auto& record = this->file->getCurrentRecord();
      auto  builder = getOutboundUsesBuilderForFormType(this->formType);
      if (builder)
         builder(record, this);
   }
}
void FormStub::send_inbound_refs() noexcept {
   for (auto it = this->outbound.begin(); it != this->outbound.end(); ++it)
      it->second.other->receive_inbound_ref(this);
}
void FormStub::receive_inbound_ref(FormStub* inbound) noexcept {
   auto& list  = this->inbound;
   auto& entry = list[inbound->formID];
   entry.other = inbound;
   entry.refcount++;
}
void FormStub::add_outbound_reference(uint32_t toFormID) {
   auto& list  = this->outbound;
   auto& entry = list[toFormID];
   if (!entry.other)
      entry.other = LoadOrder::get().getForm(toFormID);
   entry.refcount++;
}
/*static*/ void* FormStub::operator new(std::size_t sz) {
   if (sz != sizeof(FormStub))
      return ::operator new(sz);
   return FormStubHeap::get().allocate();
}
/*static*/ void FormStub::operator delete(void* ptr, std::size_t sz) {
   if (sz != sizeof(FormStub))
      return ::operator delete(ptr, sz);
   return FormStubHeap::get().free(ptr);
}