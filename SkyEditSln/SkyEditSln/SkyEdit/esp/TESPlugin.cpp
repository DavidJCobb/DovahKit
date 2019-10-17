#include "TESPlugin.h"
#include <algorithm>
#include <cassert>
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
//
bool TESPluginFile::loadRecordAt(uint32_t pos) {
   auto& r  = this->record;
   auto& rh = r.header;
   r.data.free();
   this->group.header.signature = 0;
   this->record.header.signature = 0;
   this->subrecord.signature = 0;
   //
   this->setPos(pos);
   this->record.headPos = pos;
   this->read(rh);
   rh.signature = _byteswap_ulong(rh.signature);
   this->record.bodyPos = this->getPos();
   this->record.end = this->record.bodyPos + rh.size;
   if (!this->is_good())
      return false;
   if (rh.body_is_compressed()) {
      uint32_t decompressed_size;
      this->read(decompressed_size);
      r.data.allocate(decompressed_size);
      //
      // TODO: decode
      //
      //this->recordBuffer.setup(true, decompressed_size, r.size - 4, this->record.bodyPos + 4);
   } else {
      r.data.allocate(rh.size);
      fread(r.data, 1, rh.size, this->fileHandle);
   }
   r.offset = 0;
   return true;
}
bool TESPluginFile::nextGroup() {
   auto& g = this->group.header;
   if (g.signature) {
      //_DEBUGMSG("Skipped group spanning from %08X to %08X.", this->group.pos, this->group.end);
      this->setPos(this->group.end);
      g.signature = 0;
      //
      this->record.header.signature = 0;
      this->subrecord.signature = 0;
   }
   if (!this->is_good())
      return false;
   this->group.pos = this->getPos(); // group size includes the header, so use the start of the header as the offset
   this->read(g);
   this->group.end = this->group.pos + g.size;
   g.signature = _byteswap_ulong(g.signature);
   if (!this->is_good())
      return false;
   return true;
}
bool TESPluginFile::nextRecord() {
   auto& r  = this->record;
   auto& rh = r.header;
   r.data.free();
   if (rh.signature) {
      //_DEBUGMSG("Skipped record %s with body spanning from %08X to %08X.", FMT_SIGNATURE(rh.signature), this->record.bodyPos, this->record.end);
      this->setPos(r.end);
      rh.signature = 0;
      //
      this->subrecord.signature = 0;
      //
      if (!this->is_good())
         return false;
   }
   r.headPos = this->getPos();
   if (r.headPos >= this->group.end)
      return false;
   this->read(rh);
   rh.signature = _byteswap_ulong(rh.signature);
   //
   if (rh.signature == 'GRUP') { // GRUPs can be nested... *sigh*
      this->setPos(r.headPos);
      rh.signature = 0;
      return false;
   }
   //
   r.bodyPos = this->getPos();
   r.end = r.bodyPos + rh.size;
   if (!this->is_good())
      return false;
   {
      switch (rh.signature) {
         case 'CELL':
         case 'DIAL':
         case 'WRLD':
            this->lastPotentialGroupParent = rh.formID;
            break;
         default:
            this->lastPotentialGroupParent = 0;
      }
   }
   if (rh.body_is_compressed()) {
      uint32_t decompressed_size;
      uint32_t compressed_size = rh.size - sizeof(decompressed_size);
      this->read(decompressed_size);
      r.data.allocate(decompressed_size);
      //
      auto input_buffer = malloc(compressed_size);
      fread(input_buffer, 1, compressed_size, this->fileHandle);
      uint32_t out_size = decompressed_size;
      uncompress((Bytef*)r.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
      free(input_buffer);
      assert(out_size == decompressed_size);
   } else {
      r.data.allocate(rh.size);
      fread(r.data, 1, rh.size, this->fileHandle);
   }
   r.offset = 0;
   return true;
}
bool TESPluginFile::nextSubrecord() {
   auto& r = this->record;
   if (this->subrecord.signature) {
      //this->setPos(this->subrecord.end);
      this->skipFromRecord(this->subrecord.end - (this->record.bodyPos + this->record.offset));
      this->subrecord.signature = 0;
      //
      if (!this->is_good())
         return false;
   }
   if (this->record.bodyPos + this->record.offset >= this->record.end)
      return false;
   uint16_t size;
   this->readFromRecord(this->subrecord.signature);
   this->readFromRecord(size);
   this->subrecord.size = size;
   if (this->subrecord.signature == 'XXXX') {
      //
      // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
      // larger than what can be represented with the usual two-byte length.
      //
      if (this->subrecord.size != 4) {
         return false; // ERROR
      }
      //this->read(this->subrecord.size); // the contents of the XXXX subrecord are the length
      this->readFromRecord(this->subrecord.size);
      //
      // Get the next subrecord.
      //
      //this->read(this->subrecord.signature);
      //this->skipBytes(2); // an XXXX-prefixed subrecord has no length of its own
      this->readFromRecord(this->subrecord.signature);
      this->skipFromRecord(2);
   }
   this->subrecord.signature = _byteswap_ulong(this->subrecord.signature);
   //this->subrecord.pos = this->getPos();
   this->subrecord.pos = this->record.bodyPos + this->record.offset;
   this->subrecord.end = this->subrecord.pos + size;
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
   auto r = this->getCurrentRecord();
   if (r.signature() != 'TES4') {
      _DEBUGMSG("Expected TES4 record; got something else.");
      return false;
   }
   this->flags = r.flags();
   //
   while (auto subrecord = r.next_subrecord()) {
      switch (subrecord.signature()) {
         case 'HEDR': // required subrecord; TODO: fail if this isn't present
            if (!subrecord.has_bytes(12))
               return false;
            subrecord.unchecked_read(this->fileVersion);
            subrecord.unchecked_read(this->recordCount);
            subrecord.unchecked_read(this->nextFormID);
            break;
         case 'CNAM': // author/creator
            if (!subrecord.read(this->authorName, subrecord.size()))
               return false;
            break;
         case 'SNAM': // description
            if (!subrecord.read(this->description, subrecord.size()))
               return false;
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
            if (!subrecord.read(this->subINTV))
               return false;
            break;
         case 'INCC':
            if (!subrecord.read(this->subINCC))
               return false;
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
      auto& group = this->group.header;
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
         auto r = this->getCurrentRecord();
         if (r.signature() != lastSignature) {
            lastSignature = r.signature();
            lastFormType = signatureToFormType(lastSignature);
         }
         formtype_t formType = lastFormType;
         if (!formType)
            continue;
         //
         auto& list = this->formsByType[formType];
         auto  stub = new FormStub();
         stub->file     = this;
         stub->offset   = this->record.headPos;
         stub->formID   = r.formID();
         stub->formType = formType;
         list[stub->formID] = stub;
         //
         while (auto subrecord = r.next_subrecord()) {
            if (subrecord.signature() == 'EDID') {
               auto buffer = stub->allocate_editor_id(this->subrecord.size + 1);
               this->readFromRecord(buffer, this->subrecord.size);
               buffer[this->subrecord.size] = '\0';
               break;
            }
         }
         //
         // TODO: ALSO, WHILE YOU'RE HERE: WE NEED A DIFFERENT NAME THAN "SkyEdit," BECAUSE 
         // APPARENTLY THAT'S ALREADY IN USE FOR UESP'S ATTEMPT AT CLONING THE CREATION KIT 
         // (LAST UPDATED IN 2012).
         //
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

bool TESPluginSubrecord::skip_bytes(uint32_t count) {
   if (!this->_check(count))
      return false;
   this->file->skipFromRecord(count);
   return true;
}
bool TESPluginSubrecord::read_wstring(std::string& field) {
   field.clear();
   uint16_t length;
   this->read(length);
   if (!this->_check(length))
      return false;
   field.resize(length);
   this->file->readFromRecord(const_cast<char*>(field.data()), length);
   return true;
}
bool TESPluginSubrecord::to_string(std::string& field) {
   field.clear();
   auto length = this->size();
   field.resize(length);
   this->read(const_cast<char*>(field.data()), length);
   return this->file->is_good();
}
bool TESPluginSubrecord::to_string(LStringRef& field) {
   field.value.clear();
   if (this->file->flags & TESPluginFile::kFlag_LocalizedStringTable) {
      this->read(field.index);
      //
      // TODO: implement reading from the string table
      //
      field.value = "<THE LOADING OF LSTRINGS IS NOT YET IMPLEMENTED>";
      //
      field.exists = true;
      return this->file->is_good();
   }
   return this->to_string(field.value);
}

uint32_t TESPluginRecord::peek_next_subrecord_type() {
   auto pos = this->file->subrecord.end;
   if (pos < this->file->record.end) {
      pos -= this->file->record.bodyPos;
      auto addr = (std::ptrdiff_t)this->file->record.data;
      addr += pos;
      return *(uint32_t*)addr;
   }
   return 0;
}