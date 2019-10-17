#include "TESPlugin.h"
#include <stdexcept>
#include <iostream> // for testing
#include "../output.h"
#include "../forms/components.h"

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
void TESPluginFile::readStringSubrecord(LStringRef& field) {
   field.value.clear();
   if (this->flags & kFlag_LocalizedStringTable) {
      this->read(field.index);
      //
      // TODO: implement reading from the string table
      //
      field.value  = "<THE LOADING OF LSTRINGS IS NOT YET IMPLEMENTED>";
      //
      field.exists = true;
   } else {
      this->readStringSubrecord(field.value);
   }
}
void TESPluginFile::readWString(std::string& field) {
   field.clear();
   uint16_t length;
   this->read(length);
   field.resize(length);
   fread(const_cast<char*>(field.data()), sizeof(char), length, this->fileHandle);
}
//
bool TESPluginFile::loadRecordAt(uint32_t pos) {
   this->group.signature = 0;
   this->record.signature = 0;
   this->subrecordSignature = 0;
   //
   this->setPos(pos);
   this->recordHeadPos = 0;
   auto& r = this->record;
   this->read(r);
   r.signature = _byteswap_ulong(r.signature);
   this->recordBodyPos = this->getPos();
   this->recordEnd = this->recordBodyPos + r.size;
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextGroup() {
   auto& g = this->group;
   if (g.signature) {
      //_DEBUGMSG("Skipped group spanning from %d to %d.", this->groupPos, this->getGroupEnd());
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
   this->groupEnd = this->groupPos + g.size;
   g.signature = _byteswap_ulong(g.signature);
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextRecord() {
   auto& r = this->record;
   if (r.signature) {
      //_DEBUGMSG("Skipped record body spanning from %d to %d.", this->recordBodyPos, this->getRecordEnd());
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
   this->recordEnd = this->recordBodyPos + r.size;
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
   this->subrecordEnd = this->subrecordPos + size;
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
         case 'HEDR': // required subrecord; TODO: fail if this isn't present
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
         case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
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
   //
   // TODO: need to define hardcoded forms so that references to them don't break, OR 
   // special-case them in whatever code we write to handle references between forms
   //
   while (this->nextGroup()) {
      auto& group = this->group;
      if (group.type == kESPGroupType_FormsOfType) {
         switch (_byteswap_ulong(group.label)) {
            case 'ASTP':
            case 'DIAL':
            case 'DLBR':
            case 'FACT':
            case 'GLOB':
            case 'LCTN':
            case 'NPC_':
            case 'QUST':
            case 'RELA':
            case 'VTYP':
               break;
            default:
               //
               // Skip any form signatures not identified in the cases.
               //
               continue;
         }
      }
      uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
      formtype_t lastFormType  = 0;
      while (this->nextRecord()) {
         auto& rh = this->record;
         //
         if (rh.signature != lastSignature) {
            lastSignature = rh.signature;
            lastFormType  = signatureToFormType(lastSignature);
         }
         formtype_t formType = lastFormType;
         if (!formType)
            continue;
         //
         auto& list = this->formsByType[formType];
         auto  stub = new FormStub();
         stub->file   = this;
         stub->offset = this->getRecordHeadPos();
         stub->formID = rh.formID;
         stub->formType = formType;
         list[rh.formID] = stub;
         //
         while (this->nextSubrecord()) { // TODO: If the CK or game require that EDID be the first subrecord, then make this (if) rather than (while)
            //
            // TODO: This breaks for NPC_ in the vanilla ESMs, since those records are compressed 
            // (i.e. rh.is_compressed() == true).
            //
            // UESP doesn't have documentation on compressed records for Skyrim, but they do have 
            // documentation for Oblivion. In Oblivion, the body of a compressed record consists 
            // of: the size of the decompressed data (as a uint32_t); followed by the compressed 
            // data (which extends to the end of the record) in ZLIB level 6 format. ZLIB is free 
            // to use in any project provided the copyright notice and so on are included and the 
            // ZLIB code is clearly delineated from my own: <https://github.com/madler/zlib>
            //
            // Implementing support for this will be somewhat tricky:
            //
            //  - nextRecord() and loadRecordAt() will need to check if the loaded record is 
            //    compressed. If so, we'll need to load the compressed data into memory and 
            //    decompress it.
            //
            //  - Whenever there is decompressed data loaded, read() will need to pull from 
            //    that data until such time as we reach/pass its end. (This is a good opportunity 
            //    to also alter read() so that it can't blow past the end of a subrecord, record, 
            //    group, etc..)
            //
            //     - skipBytes() will also need to be altered.
            //
            //     - Either read() needs to know what we're inside of (record, subrecord, etc.), 
            //       or we need to offer different methods for reading data from each place, 
            //       OR we should have structs representing records and subrecords (rather than 
            //       just header structs) and give them a "read" member function.
            //
            //        - Kinda digging that last idea because then, the (load) member functions 
            //          for loaded form data can just take a TESRecord& or whatever, instead of 
            //          taking a TESPluginFile*. That limits their access AND clarifies when the 
            //          functions are meant to be called.
            //
            //  - The (setPos) function will need to clear all state related to (de)compressed 
            //    data.
            //
            // TODO: ALSO, WHILE YOU'RE HERE: WE NEED A DIFFERENT NAME THAN "SkyEdit," BECAUSE 
            // APPARENTLY THAT'S ALREADY IN USE FOR UESP'S ATTEMPT AT CLONING THE CREATION KIT 
            // (LAST UPDATED IN 2012).
            //
            if (this->subrecordSignature == 'EDID') {
               auto buffer = stub->allocate_editor_id(this->subrecordSize + 1);
               this->read(buffer, this->subrecordSize);
               buffer[this->subrecordSize] = '\0';
               break;
            }
         }
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

TESPluginRecord    TESPluginFile::getCurrentRecord() { return TESPluginRecord(this); }
TESPluginSubrecord TESPluginFile::getCurrentSubrecord() { return TESPluginSubrecord(this); }

bool TESPluginSubrecord::skipBytes(uint32_t count) {
   if (!this->_check(count))
      return false;
   this->file->skipBytes(count);
}
bool TESPluginSubrecord::read_wstring(std::string& field) {
   field.clear();
   uint16_t length;
   this->read(length);
   if (!this->_check(length))
      return false;
   field.resize(length);
   this->file->read(const_cast<char*>(field.data()), length);
   return true;
}
bool TESPluginSubrecord::to_string(std::string& field) {
   this->file->readStringSubrecord(field);
   return this->file->is_good();
}
bool TESPluginSubrecord::to_string(LStringRef& field) {
   this->file->readStringSubrecord(field);
   return this->file->is_good();
}

uint32_t TESPluginRecord::peek_next_subrecord_type() {
   if (this->file->subrecordEnd < this->file->recordEnd) {
      auto pos = this->file->getPos();
      this->file->setPos(this->file->subrecordEnd);
      uint32_t signature;
      this->file->read(signature);
      this->file->setPos(pos);
      return signature;
   }
   return 0;
}