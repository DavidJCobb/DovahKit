#include "TESPlugin.h"
#include <stdexcept>
#include <iostream> // for testing

void _Debug(const char* msg) {
   std::cout << msg << std::endl;
}

TESPlugin::TESPlugin() : file(*this) {
   this->authorName[511]  = '\0';
   this->description[511] = '\0';
}
TESPlugin::~TESPlugin() {
   if (this->file.is_open())
      this->file.close();
}

bool esp_istream::isPastRecord() {
   if (!this->currentRecord.signature)
      return false;
   auto offset = this->tellg();
   return offset >= this->getRecordEndPos();
}
bool esp_istream::isPastSubrecord() {
   if (!this->subrecordSignature)
      return false;
   auto offset = this->tellg();
   return offset >= this->getSubrecordEndPos();
}
bool esp_istream::nextGroup() {
   auto& g = this->currentGroup;
   if (g.signature) {
//std::cout << "Skipped group spanning from " << this->currentGroupOffset << " to " << this->getGroupEndPos() << "." << std::endl; // DEBUG
      this->seekg(this->getGroupEndPos());
      g.signature = 0;
      //
      this->currentRecord.signature = 0;
      this->subrecordSignature = 0;
   }
   if (this->bad() || this->eof())
      return false;
   this->currentGroupOffset = this->tellg(); // group size includes the header, so use the start of the header as the offset
   this->read_value(g);
   g.signature = _byteswap_ulong(g.signature);
   if (this->bad() || this->eof())
      return false;
   return true;
}
bool esp_istream::nextRecord(bool notInGroup) {
   auto& rec = this->currentRecord;
   if (rec.signature) {
//std::cout << "Skipped record body spanning from " << this->currentRecordOffset << " to " << this->getRecordEndPos() << "." << std::endl; // DEBUG
      this->seekg(this->getRecordEndPos());
      rec.signature = 0;
      //
      this->subrecordSignature = 0;
      //
      if (this->bad() || this->eof())
         return false;
   }
   if (!notInGroup)
      if (this->tellg() >= this->getGroupEndPos())
         return false;
   this->recordHeaderOffset = this->tellg();
   this->read_value(rec);
   rec.signature = _byteswap_ulong(rec.signature);
   this->currentRecordOffset = this->tellg(); // record size does not include the header, so use the end of the header as the offset
   if (this->bad() || this->eof())
      return false;
   return true;
}
bool esp_istream::nextSubrecord() {
   if (this->subrecordSignature) {
      this->seekg(this->getSubrecordEndPos());
      this->subrecordSignature = 0;
      //
      if (this->bad() || this->eof())
         return false;
   }
   if (this->tellg() >= this->getRecordEndPos())
      return false;
   uint16_t size;
   this->read_value(this->subrecordSignature);
   this->read_value(size);
   this->subrecordSize = size;
   if (this->subrecordSignature == 'XXXX') {
      //
      // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
      // larger than what can be represented with the usual two-byte length.
      //
      if (this->subrecordSize != 4) {
         return false; // ERROR
      }
      this->read_value(this->subrecordSize); // the contents of the XXXX subrecord are the length
      //
      // Get the next subrecord.
      //
      this->read_value(this->subrecordSignature);
      this->ignore(2); // an XXXX-prefixed subrecord has no length of its own
   }
   this->subrecordSignature = _byteswap_ulong(this->subrecordSignature);
   this->subrecordOffset = this->tellg();
   if (this->bad() || this->eof())
      return false;
   return true;
}
      //
void esp_istream::clearParseState() {
   this->subrecordSignature = 0;
   this->currentRecord.signature = 0;
   this->currentGroup.signature = 0;
}

FormStub* TESPlugin::getForm(uint8_t formType, uint32_t formID) const {
   try {
      auto& list = this->formsByType.at(formType);
      return list.at(formID);
   } catch (std::out_of_range) {}
   return nullptr;
}
void TESPlugin::forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)> functor) {
   try {
      auto& list = this->formsByType.at(formType);
      for (auto it = list.begin(); it != list.end(); ++it) {
         if (functor(it->second))
            break;
      }
   }
   catch (std::out_of_range) {}
}

bool TESPluginRecordHeader::load(std::ifstream& file) {
   file.read((char*)this, sizeof(TESPluginRecordHeader));
   this->signature = _byteswap_ulong(this->signature);
   if (file.bad() || file.eof())
      return false;
   return true;
}
bool TESPlugin::loadHeader(esp_istream& file) {
   if (!file.nextRecord(true)) {
      _Debug("Expected TES4 record; no record found.");
      return false;
   }
   auto tes4 = file.getRecordHeader();
   if (tes4.signature != 'TES4') {
      _Debug("Expected TES4 record; got something else.");
      return false;
   }
   this->flags = tes4.flags;
   //
   std::string lastMaster;
   while (file.nextSubrecord()) {
      uint32_t signature = file.getSubrecordType();
      uint32_t size      = file.getSubrecordSize();
      switch (signature) {
         case 'HEDR':
            //
            // TODO
            //
            break;
         case 'CNAM': // author/creator
            file.read(this->authorName, size);
            break;
         case 'SNAM': // description
            file.read(this->description, size);
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
            file.read((char*)&this->subINTV, 4);
            break;
         case 'INCC':
            file.read((char*)&this->subINCC, 4);
            break;
      }
   }
}
void TESPlugin::load(const char* filepath) {
   file.open(filepath, std::ios_base::binary);
   if (!file) {
      _Debug("Unable to open file for reading.");
      return;
   }
   _Debug("Opened file.");
   if (!this->loadHeader(file)) {
      _Debug("Unable to read header.");
      return;
   }
   _Debug("Read file header.");
   while (file.nextGroup()) {
      auto group = file.getGroupHeader();
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
      while (file.nextRecord()) {
         auto& rh = file.getRecordHeader();
         //
         formtype_t formType = signatureToFormType(rh.signature);
         if (!formType)
            continue;
         //
         auto& list = this->formsByType[formType];
         auto  stub = new FormStub();
         stub->file   = this;
         stub->offset = file.getRecordHeaderOffset();
         stub->formID = rh.formID;
         list[rh.formID] = stub;
      }
   }
}