#include "TESPlugin.h"
#include <algorithm>
#include <cassert>
#include <stdexcept>
#include <iostream> // for testing
#include "../output.h"
#include "../forms/components.h"
#include "../helpers/strings.h"

void _Debug(const char* msg) {
   std::cout << msg << std::endl;
}

void TESPluginGroup::skip() {
   assert(this->owner);
   this->owner->setPos(this->end);
}
uint32_t TESPluginGroup::depth() const {
   assert(this->owner);
   for (uint32_t i = 0; i < std::extent<decltype(this->owner->groups)>::value; i++) {
      auto& other = this->owner->groups[i];
      if (&other == this)
         return i;
   }
   return std::numeric_limits<uint32_t>::max();
}
void TESPluginGroup::to_string(std::string& output) const {
   output.clear();
   const char* desc = "?";
   switch (this->header.type) {
      case kESPGroupType_FormsOfType:
         desc = FMT_SIGNATURE(_byteswap_ulong(this->header.label));
         break;
      case kESPGroupType_WorldChildren:
         desc = "World Children";
         break;
      case kESPGroupType_InteriorCellBlock:
         desc = "Interior Cell Block";
         break;
      case kESPGroupType_InteriorCellSubBlock:
         desc = "Interior Cell Sub-Block";
         break;
      case kESPGroupType_ExteriorCellBlock:
         desc = "Exterior Cell Block";
         break;
      case kESPGroupType_ExteriorCellSubBlock:
         desc = "Exterior Cell Sub-Block";
         break;
      case kESPGroupType_CellChildren:
         desc = "Cell Children";
         break;
      case kESPGroupType_TopicChildren:
         desc = "Topic Children";
         break;
      case kESPGroupType_CellPersistentChildren:
         desc = "Cell Persistent Children";
         break;
      case kESPGroupType_CellTemporaryChildren:
         desc = "Cell Temporary Children";
         break;
   }
   cobb::sprintf(output, "group of type %s at depth %d, from %08X to %08X", desc, this->depth(), this->pos, this->end);
}

void TESPluginRecord::unchecked_read(void* destination, uint32_t size) {
   auto source = (std::ptrdiff_t)this->data + this->offset;
   memcpy(destination, (void*)source, size);
   this->offset += size;
}
bool TESPluginRecord::read(void* destination, uint32_t size) {
   if (!this->is_in_bounds(size))
      return false;
   this->unchecked_read(destination, size);
   return true;
}
bool TESPluginRecord::skip(uint32_t bytes) {
   if (!this->is_in_bounds(bytes))
      return false;
   this->offset += bytes;
   return true;
}
TESPluginSubrecord& TESPluginRecord::next_subrecord() const {
   this->owner.nextSubrecord();
   return this->owner.getCurrentSubrecord();
}
uint32_t TESPluginRecord::peek_next_subrecord_type() {
   auto pos = this->owner.subrecord.end_pos();
   if (pos < this->end) {
      pos -= this->bodyPos;
      auto addr = (std::ptrdiff_t)this->data;
      addr += pos;
      return *(uint32_t*)addr;
   }
   return 0;
}

TESPluginRecord& TESPluginSubrecord::get_containing_record() const {
   return this->owner.record;
}
bool TESPluginSubrecord::read_wstring(std::string& field) {
   field.clear();
   uint16_t length;
   this->read(length);
   field.resize(length);
   return this->read(const_cast<char*>(field.data()), length);
}
bool TESPluginSubrecord::to_string(std::string& field) {
   field.clear();
   auto length = this->size();
   field.resize(length);
   return this->read(const_cast<char*>(field.data()), length);
}
bool TESPluginSubrecord::to_string(LStringRef& field) {
   field.value.clear();
   if (this->owner.flags & TESPluginFile::kFlag_LocalizedStringTable) {
      bool result = this->read(field.index);
      //
      // TODO: implement reading from the string table
      //
      field.value = "<THE LOADING OF LSTRINGS IS NOT YET IMPLEMENTED>";
      //
      field.exists = true;
      return result;
   }
   return this->to_string(field.value);
}



TESPluginFile::TESPluginFile() : record(*this), subrecord(*this) {
   this->authorName[511]  = '\0';
   this->description[511] = '\0';
   //
   for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++)
      this->groups[i].initialize(this);
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
void TESPluginFile::rewind(uint32_t by) {
   this->setPos(this->getPos() - by);
}
bool TESPluginFile::isEOF() {
   return feof(this->fileHandle);
}
bool TESPluginFile::is_good() {
   return !ferror(this->fileHandle) && !this->isEOF();
}
//
TESPluginFile::ObjectType TESPluginFile::nextRecordOrGroup() {
   auto& record = this->record;
   if (record) {
      /*//
      if (this->getPos() == record.end)
         _DEBUGMSG("Reached the end of record of type %s from %08X to %08X...", FMT_SIGNATURE(record.signature()), record.headPos, record.end);
      else
         _DEBUGMSG("Skipping record of type %s from %08X to %08X...", FMT_SIGNATURE(record.signature()), record.headPos, record.end);
      //*/
      this->setPos(record.end);
      record.reset();
   }
   //
   // Make sure we properly handle passing the end of a group:
   //
   auto pos = this->getPos();
   for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++) {
      auto& group = this->groups[i];
      if (!group)
         break;
      if (pos <= group.pos || pos >= group.end)
         group.reset();
   }
   //
   if (!this->is_good())
      return ObjectType::kObjectType_None;
   uint32_t signature;
   this->read(signature);
   signature = _byteswap_ulong(signature);
   if (signature == 'GRUP') {
      this->rewind(4);
      //
      auto pos = this->getPos();
      int32_t parent = -1;
      for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++) {
         auto& group = this->groups[i];
         if (!group)
            break;
         parent = i;
      }
      assert(parent + 1 < std::extent<decltype(this->groups)>::value);
      auto& group = this->groups[parent + 1];
      group.pos   = this->getPos();
      this->read(group.header);
      group.header.signature = _byteswap_ulong(group.header.signature);
      group.end   = group.pos + group.header.size;
      /*{
         std::string log;
         group.to_string(log);
         _DEBUGMSG("Found %s.", log.c_str());
      }*/
      return ObjectType::kObjectType_Group;
   }
   //
   // else it must be a record
   //
   this->rewind(4);
   //
   record.headPos = this->getPos();
   if (auto& group = this->getCurrentGroup())
      if (record.headPos >= group.end)
         return ObjectType::kObjectType_None;
   this->read(record.header);
   record.header.signature = _byteswap_ulong(record.header.signature);
   record.bodyPos = this->getPos();
   record.end = record.bodyPos + record.header.size;
   if (!this->is_good())
      return ObjectType::kObjectType_None;
   {
      switch (record.header.signature) {
         case 'CELL':
         case 'DIAL':
         case 'WRLD':
            this->lastPotentialGroupParent = record.header.formID;
            break;
         default:
            this->lastPotentialGroupParent = 0;
      }
   }
   if (record.header.body_is_compressed()) {
      uint32_t decompressed_size;
      uint32_t compressed_size = record.header.size - sizeof(decompressed_size);
      this->read(decompressed_size);
      record.data.allocate(decompressed_size);
      //
      auto input_buffer = malloc(compressed_size);
      fread(input_buffer, 1, compressed_size, this->fileHandle);
      uint32_t out_size = decompressed_size;
      uncompress((Bytef*)record.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
      free(input_buffer);
      assert(out_size == decompressed_size);
   } else {
      record.data.allocate(record.header.size);
      fread(record.data, 1, record.header.size, this->fileHandle);
   }
   record.offset = 0;
   //
   return ObjectType::kObjectType_Record;
}
//
bool TESPluginFile::nextSubrecord() {
   auto& r = this->record;
   if (this->subrecord.header.signature) {
      //this->setPos(this->subrecord.end);
      this->record.skip(this->subrecord.end - this->record.stream_pos());
      this->subrecord.header.signature = 0;
      //
      if (!this->is_good())
         return false;
   }
   if (this->record.stream_pos() >= this->record.end)
      return false;
   uint16_t size;
   this->record.read(this->subrecord.header.signature);
   this->record.read(size);
   this->subrecord.header.size = size;
   if (this->subrecord.header.signature == 'XXXX') {
      //
      // An 'XXXX' subrecord is used as a prefix for a subrecord whose size is 
      // larger than what can be represented with the usual two-byte length.
      //
      if (this->subrecord.header.size != 4) {
         return false; // ERROR
      }
      //this->read(this->subrecord.size); // the contents of the XXXX subrecord are the length
      static_assert(sizeof(this->subrecord.header.size) == 4, "XXXX subrecords store a four-byte subrecord length.");
      this->record.read(this->subrecord.header.size);
      //
      // Get the next subrecord.
      //
      //this->read(this->subrecord.signature);
      //this->skipBytes(2); // an XXXX-prefixed subrecord has no length of its own
      this->record.read(this->subrecord.header.signature);
      this->record.skip(2);
   }
   this->subrecord.header.signature = _byteswap_ulong(this->subrecord.header.signature);
   //this->subrecord.pos = this->getPos();
   this->subrecord.pos = this->record.bodyPos + this->record.offset;
   this->subrecord.end = this->subrecord.pos + size;
   if (!this->is_good() || !this->record.is_in_bounds())
      return false;
   return true;
}
bool TESPluginFile::loadRecordAt(uint32_t pos) {
   for (uint32_t i = 0; i < std::extent<decltype(this->groups)>::value; i++)
      this->groups[i].reset();
   this->record.reset();
   //
   this->setPos(pos);
   return this->nextRecordOrGroup() == kObjectType_Record;
}
//
bool TESPluginFile::_loadHeader() {
   if (this->nextRecordOrGroup() != ObjectType::kObjectType_Record) {
      _DEBUGMSG("Expected TES4 record; no record found.");
      return false;
   }
   auto& r = this->getCurrentRecord();
   if (r.signature() != 'TES4') {
      _DEBUGMSG("Expected TES4 record; got something else.");
      return false;
   }
   this->flags = r.flags();
   //
   while (auto& subrecord = r.next_subrecord()) {
      switch (subrecord.signature()) {
         case 'HEDR': // required subrecord; TODO: fail if this isn't present
            if (!subrecord.is_in_bounds(12)) {
               _DEBUGMSG("Unable to read header HEDR.");
               return false;
            }
            subrecord.unchecked_read(this->fileVersion);
            subrecord.unchecked_read(this->recordCount);
            subrecord.unchecked_read(this->nextFormID);
            break;
         case 'CNAM': // author/creator
            if (!subrecord.read(this->authorName, subrecord.size())) {
               _DEBUGMSG("Unable to read header CNAM (creator name).");
               return false;
            }
            break;
         case 'SNAM': // description
            if (!subrecord.read(this->description, subrecord.size())) {
               _DEBUGMSG("Unable to read header SNAM (description).");
               return false;
            }
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
            if (!subrecord.read(this->subINTV)) {
               _DEBUGMSG("Unable to read header INTV.");
               return false;
            }
            break;
         case 'INCC':
            if (!subrecord.read(this->subINCC)) {
               _DEBUGMSG("Unable to read header INCC.");
               return false;
            }
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
   ObjectType ot;
   uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
   formtype_t lastFormType  = 0;
   while (ot = this->nextRecordOrGroup(), ot != ObjectType::kObjectType_None) {
      //
      // It may be tempting to skip the loading of top-groups that aren't of interest. 
      // However, it would be unsafe to do that: we need to know what form IDs are 
      // taken, and the only way to do that is to actually load the forms.
      //
      if (ot == kObjectType_Record) {
         auto& record = this->record;
         if (record.signature() != lastSignature) {
            lastSignature = record.signature();
            lastFormType  = signatureToFormType(lastSignature);
         }
         formtype_t formType = lastFormType;
         if (!formType)
            continue;
         //
         auto& list = this->formsByType[formType];
         auto  stub = new FormStub();
         stub->file     = this;
         stub->offset   = record.headPos;
         stub->formID   = record.formID();
         stub->formType = formType;
         list[stub->formID] = stub;
         //
         while (auto& subrecord = record.next_subrecord()) {
            if (subrecord.signature() == 'EDID') {
               auto buffer = stub->allocate_editor_id(this->subrecord.header.size + 1);
               this->record.read(buffer, this->subrecord.header.size);
               buffer[this->subrecord.header.size] = '\0';
               break;
            }
         }
         continue;
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