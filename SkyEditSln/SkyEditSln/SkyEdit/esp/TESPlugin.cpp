#include "TESPlugin.h"
#include <stdexcept>
#include <iostream> // for testing
#include "../output.h"

void _Debug(const char* msg) {
   std::cout << msg << std::endl;
}

TESPluginFile::TESPluginFile() {
   this->authorName[511]  = '\0';
   this->description[511] = '\0';
}
TESPluginFile::~TESPluginFile() {
   if (this->fileHandle) {
      fclose(this->fileHandle);
      this->fileHandle = nullptr;
   }
}
//
void TESPluginFile::setPos(uint32_t pos) {
   clearerr(this->fileHandle);
   fseek(this->fileHandle, pos, SEEK_SET);
}
uint32_t TESPluginFile::getPos() {
   return ftell(this->fileHandle);
}
void TESPluginFile::skipBytes(uint32_t count) {
   fseek(this->fileHandle, count, SEEK_CUR);
}
bool TESPluginFile::isEOF() {
   return feof(this->fileHandle);
}
bool TESPluginFile::is_good() {
   return !ferror(this->fileHandle) && !this->isEOF();
}
void TESPluginFile::readStringSubrecord(std::string& field) {
   field.clear();
   if (!this->subrecordSignature)
      return;
   field.resize(this->subrecordSize);
   fread(const_cast<char*>(field.data()), sizeof(char), this->subrecordSize, this->fileHandle);
}
//
bool TESPluginFile::loadRecordAt(uint32_t pos) {
   this->setPos(pos);
   this->recordHeadPos = 0;
   auto& r = this->record;
   this->read(r);
   r.signature = _byteswap_ulong(r.signature);
   this->recordBodyPos = this->getPos();
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextGroup() {
   auto& g = this->group;
   if (g.signature) {
      _DEBUGMSG("Skipped group spanning from %d to %d.", this->groupPos, this->getGroupEnd());
      this->setPos(this->getGroupEnd());
      g.signature = 0;
      //
      this->record.signature = 0;
      this->subrecordSignature = 0;
   }
   if (!this->is_good())
      return false;
   this->groupPos = this->getPos(); // group size includes the header, so use the start of the header as the offset
   this->read(g);
   g.signature = _byteswap_ulong(g.signature);
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextRecord() {
   auto& r = this->record;
   if (r.signature) {
      _DEBUGMSG("Skipped record body spanning from %d to %d.", this->recordBodyPos, this->getRecordEnd());
      this->setPos(this->getRecordEnd());
      r.signature = 0;
      //
      this->subrecordSignature = 0;
      //
      if (!this->is_good())
         return false;
   }
   this->recordHeadPos = this->getPos();
   if (this->recordHeadPos >= this->getGroupEnd())
      return false;
   this->read(r);
   r.signature = _byteswap_ulong(r.signature);
   this->recordBodyPos = this->getPos();
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextSubrecord() {
   if (this->subrecordSignature) {
      this->setPos(this->getSubrecordEnd());
      this->subrecordSignature = 0;
      //
      if (!this->is_good())
         return false;
   }
   if (this->getPos() >= this->getRecordEnd())
      return false;
   uint16_t size;
   this->read(this->subrecordSignature);
   this->read(size);
   this->subrecordSize = size;
   if (this->subrecordSignature == 'XXXX') {
      //
      // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
      // larger than what can be represented with the usual two-byte length.
      //
      if (this->subrecordSize != 4) {
         return false; // ERROR
      }
      this->read(this->subrecordSize); // the contents of the XXXX subrecord are the length
      //
      // Get the next subrecord.
      //
      this->read(this->subrecordSignature);
      this->skipBytes(2); // an XXXX-prefixed subrecord has no length of its own
   }
   this->subrecordSignature = _byteswap_ulong(this->subrecordSignature);
   this->subrecordPos = this->getPos();
   if (!this->is_good())
      return false;
   return true;
}
//
bool TESPluginFile::_loadHeader() {
   if (!this->loadRecordAt(0)) {
      _DEBUGMSG("Expected TES4 record; no record found.");
      return false;
   }
   auto& tes4 = this->record;
   if (tes4.signature != 'TES4') {
      _DEBUGMSG("Expected TES4 record; got something else.");
      return false;
   }
   this->flags = tes4.flags;
   //
   while (this->nextSubrecord()) {
      uint32_t signature = this->subrecordSignature;
      uint32_t size      = this->subrecordSize;
      switch (signature) {
         case 'HEDR':
            //
            // TODO
            //
            break;
         case 'CNAM': // author/creator
            this->read(this->authorName, size);
            break;
         case 'SNAM': // description
            this->read(this->description, size);
            break;
         case 'MAST':
            //
            // TODO
            //
            break;
         case 'DATA':
            //
            // TODO
            //
            break;
         case 'ONAM':
            //
            // TODO
            //
            break;
         case 'INTV':
            this->read(this->subINTV);
            break;
         case 'INCC':
            this->read(this->subINCC);
            break;
      }
   }
   return true;
}
bool TESPluginFile::load(const char* filepath) {
   this->fileHandle = _fsopen(filepath, "rb", _SH_DENYWR);
   if (!this->fileHandle) {
      _DEBUGMSG("Unable to open file for reading.");
      return false;
   }
   _DEBUGMSG("Opened file.");
   if (!this->_loadHeader()) {
      _DEBUGMSG("Unable to read header.");
      return false;
   }
   _DEBUGMSG("Read file header.");
   while (this->nextGroup()) {
      auto& group = this->group;
      if (group.type == kESPGroupType_FormsOfType) {
         switch (_byteswap_ulong(group.label)) {
            case 'DIAL':
            case 'QUST':
               break;
            default:
               //
               // Skip any form signatures not identified in the cases.
               //
               continue;
         }
      }
      while (this->nextRecord()) {
         auto& rh = this->record;
         //
         formtype_t formType = signatureToFormType(rh.signature);
         if (!formType)
            continue;
         //
         auto& list = this->formsByType[formType];
         auto  stub = new FormStub();
         stub->file   = this;
         stub->offset = this->getRecordHeadPos();
         stub->formID = rh.formID;
         list[rh.formID] = stub;
      }
   }
   return true;
}
//
FormStub* TESPluginFile::getForm(uint8_t formType, uint32_t formID) const {
   try {
      auto& list = this->formsByType.at(formType);
      return list.at(formID);
   } catch (std::out_of_range) {}
   return nullptr;
}
void TESPluginFile::forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)> functor) {
   try {
      auto& list = this->formsByType.at(formType);
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (functor(it->second))
            break;
      }
   } catch (std::out_of_range) {}
}