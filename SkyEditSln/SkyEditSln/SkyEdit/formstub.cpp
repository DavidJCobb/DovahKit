#include "formstub.h"
#include "esp/TESPlugin.h"
#include "helpers/miscellaneous.h"
#include "output.h"
#include <cassert>
#include <cstddef>

#include "forms/Quest.h"

FormStub::~FormStub() {
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
loaded_form_ptr<TESForm> FormStub::load() {
   if (!this->form && this->file) {
      //_DEBUGMSG("stub is loading...");
      auto file = this->file;
      if (this->file->loadRecordAt(this->offset)) {
         auto& record   = this->file->getCurrentRecord();
         auto  formType = signatureToFormType(record.signature());
         switch (formType) {
            case kFormType_Quest:
               {
                  auto q = new TESQuest();
                  q->load(record);
                  this->form = q;
               }; break;
         }
      } else
         _DEBUGMSG("...stub failed.");
   }
   return loaded_form_ptr<TESForm>(this);
}
void FormStub::set_edited(bool v) {
   cobb::edit_bit(this->refcount, kRefcountFlag_Edited, v);
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