#include "formstub.h"
#include "esp/TESPlugin.h"
#include "forms/Quest.h"

#include <iostream> // DEBUG

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
std::cout << "...stub is loading..." << std::endl; // DEBUG
      auto& stream = this->file->file;
      stream.clearParseState();
      stream.clear(); // clear EOF state (seekg doesn't do this)
      stream.seekg(this->offset);
std::cout << "...moved to offset " << this->offset << "..." << std::endl; // DEBUG
      if (stream.nextRecord(true)) {
         auto& header   = stream.getRecordHeader();
char sig[5]; // DEBUG
sig[0] = header.signature >> 0x18;
sig[1] = header.signature >> 0x10 & 0xFF;
sig[2] = header.signature >> 0x08 & 0xFF;
sig[3] = header.signature & 0xFF;
sig[4] = 0;
std::cout << "...header is " << sig << "..." << std::endl; // DEBUG
         auto  formType = signatureToFormType(header.signature);
std::cout << "...form type is " << (int)formType << "..." << std::endl; // DEBUG
         switch (formType) {
            case 77: {
                  auto q = new TESQuest();
                  q->load(stream);
                  this->form = q;
               }; break;
         }
      }
else std::cout << "...stub failed." << std::endl; // DEBUG
      stream.clearParseState();
   }
   return loaded_form_ptr(this);
}