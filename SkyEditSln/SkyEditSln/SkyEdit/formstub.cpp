#include "formstub.h"
#include "esp/TESPlugin.h"
#include "forms/Quest.h"
#include "output.h"

loaded_form_ptr::loaded_form_ptr(FormStub* stub) {
   this->wrapped = stub;
   this->_incRef();
}
loaded_form_ptr::~loaded_form_ptr() {
   this->_decRef();
   this->wrapped = nullptr;
}
loaded_form_ptr& loaded_form_ptr::operator=(FormStub* stub) noexcept {
   this->_decRef();
   this->wrapped = stub;
   this->_incRef();
   return *this;
}

loaded_form_ptr FormStub::load() {
   if (!this->form && this->file) {
      //_DEBUGMSG("stub is loading...");
      auto file = this->file;
      if (this->file->loadRecordAt(this->offset)) {
         auto& header = this->file->getRecordHeader();
         //_DEBUGMSG("...header is %s...", FMT_SIGNATURE(header.signature));
         auto  formType = signatureToFormType(header.signature);
         //_DEBUGMSG("...form type is %d...", formType);
         switch (formType) {
            case 77:
               {
                  auto q = new TESQuest();
                  q->load(file);
                  this->form = q;
               }; break;
         }
      } else
         _DEBUGMSG("...stub failed.");
   }
   return loaded_form_ptr(this);
}