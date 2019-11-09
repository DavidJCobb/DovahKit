#include "TESPlugin.h"
#include <algorithm>
#include <cassert>
#include <filesystem>
#include <stdexcept>
#include <iostream> // for testing
#include "../output.h"
#include "../forms/components.h"
#include "../helpers/strings.h"
#include "LoadOrder.h"

#define DO_ESP_LOAD_BENCHMARKS 1
#ifdef DO_ESP_LOAD_BENCHMARKS
   #include <sys/timeb.h> // for benchmarks
#endif

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
TESPluginGroup* TESPluginGroup::getParent() const {
   assert(this->owner);
   for (uint32_t i = 1; i < std::extent<decltype(this->owner->groups)>::value; i++) {
      auto parent  = &this->owner->groups[i - 1];
      auto current = &this->owner->groups[i];
      if (current == this)
         return parent;
   }
   return nullptr;
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
   if (this->owner.uses_string_table) {
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

void TESPluginBaseReader::setPos(uint32_t pos) {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->stream_position = pos;
   #else
      clearerr(this->fileHandle);
      fseek(this->fileHandle, pos, SEEK_SET);
   #endif
}
uint32_t TESPluginBaseReader::getPos() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      return this->stream_position;
   #else
      return ftell(this->fileHandle);
   #endif
}
void TESPluginBaseReader::skipBytes(uint32_t count) {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->stream_position += count;
   #else
      fseek(this->fileHandle, count, SEEK_CUR);
   #endif
}
void TESPluginBaseReader::rewind(uint32_t by) {
   this->setPos(this->getPos() - by);
}
bool TESPluginBaseReader::isEOF() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      return !this->file->is_in_bounds(this->stream_position, 1);
   #else
      return feof(this->fileHandle);
   #endif
}
bool TESPluginBaseReader::is_good() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      return !this->isEOF();
   #else
      return !ferror(this->fileHandle) && !this->isEOF();
   #endif
}
TESPluginBaseReader::ObjectType TESPluginBaseReader::nextRecordOrGroup() {
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
      this->read(input_buffer, compressed_size);
      uint32_t out_size = decompressed_size;
      uncompress((Bytef*)record.data.raw(), (uLongf*)&out_size, (Bytef*)input_buffer, compressed_size);
      free(input_buffer);
      assert(out_size == decompressed_size);
   } else {
      record.data.allocate(record.header.size);
      this->read(record.data.raw(), record.header.size);
   }
   record.offset = 0;
   //
   return ObjectType::kObjectType_Record;
}
//
bool TESPluginBaseReader::nextSubrecord() {
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
FormStub* TESPluginBaseReader::make_stub_for_record(TESPluginFile& file) {
   auto& record = this->getCurrentRecord();
   auto  stub   = new FormStub();
   stub->file     = &file;
   stub->offset   = record.headPos;
   stub->formID   = record.formID();
   stub->formType = signatureToFormType(record.signature());
   return stub;
}
void TESPluginBaseReader::extract_editor_id_for_stub(FormStub* stub) {
   if (formTypeFor(stub->formType).flags & (uint32_t)FormTypeFlags::no_editor_id)
      return;
   auto& record = this->getCurrentRecord();
   while (auto& subrecord = record.next_subrecord()) {
      if (subrecord.signature() == 'EDID') {
         auto buffer = stub->allocate_editor_id(subrecord.size() + 1);
         record.read(buffer, subrecord.size());
         buffer[subrecord.size()] = '\0';
         return;
      }
   }
}

void TESPluginThreadedSimpleReader::_thread_handler(TESPluginThreadedSimpleReader* instance) {
   auto registration = FormStubHeap::get().register_thread();
   instance->_load();
}
void TESPluginThreadedSimpleReader::_load() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = this->owner.file;
   #else
      if (this->fileHandle) {
         fclose(this->fileHandle);
         this->fileHandle = nullptr;
      }
      this->fileHandle = _fsopen(this->owner.path.c_str(), "rb", _SH_DENYWR);
      assert(this->fileHandle && "[TESPluginThreadedSimpleReader] Unable to open the file for reading.");
   #endif
   //
   auto size = this->queue.size();
   for (uint32_t i = 0; i < size; i++) {
      auto& desired = this->queue[i];
      this->setPos(desired.pos);
      this->resetParseState();
      assert(this->nextRecordOrGroup() == kObjectType_Group);
      ObjectType ot;
      uint32_t   lastGroupLabel = 0; // for debug logging
      uint32_t   lastSignature  = 0; // shortcut to reduce the number of form type lookups we need
      formtype_t lastFormType   = 0;
      while (ot = this->nextRecordOrGroup(), ot != ObjectType::kObjectType_None) {
         if (ot == kObjectType_Group) {
            //
            // Stop if we've reached the end of the group we're meant to parse.
            //
            auto& first = this->groups[0];
            if (first && first.pos == desired.pos) {
               lastGroupLabel = _byteswap_ulong(first.header.label);
            } else {
               //char sig_buffer[5];
               //_DEBUGMSG("[TESPluginThreadedSimpleReader] Thread %08X finished parse of group %s.", std::this_thread::get_id(), FMT_SIGNATURE(lastGroupLabel, sig_buffer));
               break;
            }
         }
         if (ot == kObjectType_Record) {
            auto& record = this->getCurrentRecord();
            auto& group  = this->getCurrentGroup();
            if (record.signature() != lastSignature) {
               lastSignature = record.signature();
               lastFormType  = signatureToFormType(lastSignature);
            }
            formtype_t formType = lastFormType;
            if (!formType)
               continue;
            //
            auto  stub = this->make_stub_for_record(this->owner);
            stub->groupInfo.groupType = group.header.type;
            this->owner._insertForm(stub->formID, stub);
            this->extract_editor_id_for_stub(stub);
            continue;
         }
      }
   }
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = nullptr;
   #else
      fclose(this->fileHandle);
      this->fileHandle = nullptr;
   #endif
   //
   _DEBUGMSG("[TESPluginThreadedSimpleReader] Thread %08X finished all of its work.");
}
void TESPluginThreadedSimpleReader::add_group(uint32_t signature, uint32_t pos) {
   this->queue.emplace_back(signature, pos);
}
void TESPluginThreadedSimpleReader::start() {
   this->thread = std::thread(TESPluginThreadedSimpleReader::_thread_handler, this);
}
void TESPluginThreadedSimpleReader::wait_for() {
   this->thread.join();
}

void TESPluginThreadedInteriorCellReader::_thread_handler(TESPluginThreadedInteriorCellReader* instance) {
   auto registration = FormStubHeap::get().register_thread();
   instance->_load();
}
void TESPluginThreadedInteriorCellReader::_load() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = this->owner.file;
   #else
      if (this->fileHandle) {
         fclose(this->fileHandle);
         this->fileHandle = nullptr;
      }
      this->fileHandle = _fsopen(this->owner.path.c_str(), "rb", _SH_DENYWR);
      assert(this->fileHandle && "[TESPluginThreadedInteriorCellReader] Unable to open the file for reading.");
   #endif
   //
   auto size = this->queue.size();
   for (uint32_t i = 0; i < size; i++) {
      auto& desired = this->queue[i];
      //_DEBUGMSG("[TESPluginThreadedInteriorCellReader] Thread %08X beginning with interior-cell-block %d at position %08X.", std::this_thread::get_id(), desired.blockNumber, desired.pos);
      this->setPos(desired.pos);
      this->resetParseState();
      assert(this->nextRecordOrGroup() == kObjectType_Group);
      ObjectType ot;
      uint32_t   lastBlockNumber = 0; // for debug logging
      uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
      formtype_t lastFormType  = 0;
      while (ot = this->nextRecordOrGroup(), ot != ObjectType::kObjectType_None) {
         if (ot == kObjectType_Group) {
            //
            // Stop if we've reached the end of the group we're meant to parse.
            //
            auto& first = this->groups[0];
            if (first && first.pos == desired.pos) {
               lastBlockNumber = first.header.label;
            } else {
               //_DEBUGMSG("[TESPluginThreadedInteriorCellReader] Thread %08X finished parse of interior-cell block %d.", std::this_thread::get_id(), lastBlockNumber);
               break;
            }
         }
         if (ot == kObjectType_Record) {
            auto& record = this->getCurrentRecord();
            auto& group  = this->getCurrentGroup();
            if (record.signature() != lastSignature) {
               lastSignature = record.signature();
               lastFormType = signatureToFormType(lastSignature);
            }
            formtype_t formType = lastFormType;
            if (!formType)
               continue;
            //
            auto  stub = this->make_stub_for_record(this->owner);
            stub->groupInfo.groupType = group.header.type;
            if (record.signature() == 'CELL') {
               stub->groupInfo.cellBlock.interior    = this->groups[0].header.label;
               stub->groupInfo.cellSubBlock.interior = this->groups[1].header.label;
            }
            this->owner._insertForm(stub->formID, stub);
            this->extract_editor_id_for_stub(stub);
            continue;
         }
      }
   }
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = nullptr;
   #else
      fclose(this->fileHandle);
      this->fileHandle = nullptr;
   #endif
   //
   _DEBUGMSG("[TESPluginThreadedInteriorCellReader] Thread %08X finished all of its work.");
}
void TESPluginThreadedInteriorCellReader::add_group(uint32_t blockNumber, uint32_t pos) {
   this->queue.emplace_back(blockNumber, pos);
}
void TESPluginThreadedInteriorCellReader::start() {
   this->thread = std::thread(TESPluginThreadedInteriorCellReader::_thread_handler, this);
}
void TESPluginThreadedInteriorCellReader::wait_for() {
   this->thread.join();
}

void TESPluginThreadedWorldspaceSubBlockReader::_thread_handler(TESPluginThreadedWorldspaceSubBlockReader* instance) {
   auto registration = FormStubHeap::get().register_thread();
   instance->_load();
}
void TESPluginThreadedWorldspaceSubBlockReader::_load() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = this->owner.file;
   #else
      if (this->fileHandle) {
         fclose(this->fileHandle);
         this->fileHandle = nullptr;
      }
      this->fileHandle = _fsopen(this->owner.path.c_str(), "rb", _SH_DENYWR);
      assert(this->fileHandle && "[TESPluginThreadedWorldspaceSubBlockReader] Unable to open the file for reading.");
   #endif
   //
   auto size = this->queue.size();
   for (uint32_t i = 0; i < size; i++) {
      auto& desired = this->queue[i];
      //_DEBUGMSG("[TESPluginThreadedWorldspaceSubBlockReader] Thread %08X beginning with [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
      this->setPos(desired.pos);
      this->resetParseState();
      assert(this->nextRecordOrGroup() == kObjectType_Group);
      ObjectType ot;
      uint32_t   lastBlockNumber = 0; // for debug logging
      uint32_t   lastSignature = 0; // shortcut to reduce the number of form type lookups we need
      formtype_t lastFormType = 0;
      while (ot = this->nextRecordOrGroup(), ot != ObjectType::kObjectType_None) {
         if (ot == kObjectType_Group) {
            //
            // Stop if we've reached the end of the group we're meant to parse.
            //
            auto& first = this->groups[0];
            if (!first || first.pos != desired.pos) {
               //_DEBUGMSG("[TESPluginThreadedWorldspaceSubBlockReader] Thread %08X finished parse of [WRLD:%08X]/(%d, %d)/(%d, %d) at position %08X.", std::this_thread::get_id(), desired.worldspaceID, desired.blockX, desired.blockY, desired.subBlockX, desired.subBlockY, desired.pos);
               break;
            }
         }
         if (ot == kObjectType_Record) {
            auto& record = this->getCurrentRecord();
            auto& group  = this->getCurrentGroup();
            if (record.signature() != lastSignature) {
               lastSignature = record.signature();
               lastFormType = signatureToFormType(lastSignature);
            }
            formtype_t formType = lastFormType;
            if (!formType)
               continue;
            //
            auto  stub = this->make_stub_for_record(this->owner);
            stub->groupInfo.groupType = group.header.type;
            if (record.signature() == 'CELL') {
               stub->groupInfo.parentFormID = desired.worldspaceID;
               stub->groupInfo.cellBlock.exterior.x = desired.blockX;
               stub->groupInfo.cellBlock.exterior.y = desired.blockY;
               stub->groupInfo.cellSubBlock.exterior.x = desired.subBlockX;
               stub->groupInfo.cellSubBlock.exterior.y = desired.subBlockY;
            }
            this->owner._insertForm(stub->formID, stub);
            this->extract_editor_id_for_stub(stub);
            continue;
         }
      }
   }
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = nullptr;
   #else
      fclose(this->fileHandle);
      this->fileHandle = nullptr;
   #endif
   //
   _DEBUGMSG("[TESPluginThreadedWorldspaceSubBlockReader] Thread %08X finished all of its work.");
}
void TESPluginThreadedWorldspaceSubBlockReader::add_group(uint32_t worldID, int16_t bx, int16_t by, int16_t sbx, int16_t sby, uint32_t pos) {
   this->queue.emplace_back(worldID, bx, by, sbx, sby, pos);
}
void TESPluginThreadedWorldspaceSubBlockReader::start() {
   this->thread = std::thread(TESPluginThreadedWorldspaceSubBlockReader::_thread_handler, this);
}
void TESPluginThreadedWorldspaceSubBlockReader::wait_for() {
   this->thread.join();
}

TESPluginFile::TESPluginFile() :
   simpleReaders{ *this, *this, *this, *this }, // NOT a typo; this is needed to initialize the array
   interiorCellReaders{ *this, *this, *this, *this },
   worldspaceReaders{ *this, *this, *this, *this, *this, *this },
   complexReader(*this)
{
   this->authorName[511]  = '\0';
   this->description[511] = '\0';
   //
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file = new cobb::mapped_file();
   #endif
}
TESPluginFile::~TESPluginFile() {
   #ifdef COBB_ESP_USE_MAPPED_FILES
      if (this->file) {
         delete this->file;
         this->file = nullptr;
      }
   #else
      if (this->fileHandle) {
         fclose(this->fileHandle);
         this->fileHandle = nullptr;
      }
   #endif
}
bool TESPluginFile::loadRecordAt(uint32_t pos) {
   this->resetParseState();
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
   uint32_t last_subrecord = 0;
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
            if (last_subrecord == 'MAST') { // wrong; should be separated by 'DATA'
               _DEBUGMSG("Warning: a 'MAST' subrecord in the file header lacked a matching 'DATA' subrecord.");
            }
            {
               this->masters.emplace_back();
               MasterEntry& last = *this->masters.rbegin();
               if (!subrecord.to_string(last.master)) {
                  _DEBUGMSG("Failed to read a 'MAST' subrecord in the file header.");
                  return false;
               }
               if (this->masters.size() > 253) {
                  _DEBUGMSG("This file claims to have more than 253 masters.");
                  return false;
               }
               //
               // TODO: Check if the specified master is in the load order. If not, then we need 
               // to add it to the load order just before this file. Not yet sure how we oughta 
               // do that.
               //
               // Actually, it might be easier to:
               //
               //  - Have LoadOrder pre-load the headers of all relevant files, grab masters as 
               //    necessary, and construct a final load order. TESPluginFile should not be 
               //    used to get the headers.
               //
               //  - Have LoadOrder load that final load order using TESPluginFile, which will 
               //    lead to this function being called.
               //
               //  - Have the load process fail if this function encounters any unexpected 
               //    masters.
               //
            }
            break;
         case 'DATA': // always follows a MAST; vestigial; doesn't appear to be used
            if (last_subrecord != 'MAST') {
               if (last_subrecord)
                  _DEBUGMSG("Unexpected 'DATA' subrecord in the file header following %s.", FMT_SIGNATURE(last_subrecord));
               else
                  _DEBUGMSG("Unexpected 'DATA' subrecord at the start of the file header.");
               return false;
            } else {
               MasterEntry& last = *this->masters.rbegin();
               if (!subrecord.read(last.data)) {
                  _DEBUGMSG("Warning: failed to read the 'DATA' subrecord for master: %s", last.master.c_str());
               }
            }
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
      last_subrecord = subrecord.signature();
   }
   return true;
}
void TESPluginFile::_insertForm(uint32_t formID, FormStub* stub) {
   LoadOrder::get().acceptFormStub(stub);
   /*//
   auto& type = this->formsByType[stub->formType];
   std::lock_guard<std::mutex> guard(type.lock);
   type.forms[formID] = stub;
   //*/
}
bool TESPluginFile::load(const char* filepath) {
   this->path.clear();
   #ifdef COBB_ESP_USE_MAPPED_FILES
      this->file->open(filepath);
   #else
      this->fileHandle = _fsopen(filepath, "rb", _SH_DENYWR);
      if (!this->fileHandle) {
         _DEBUGMSG("Unable to open file for reading.");
         return false;
      }
   #endif
   _DEBUGMSG("Opened file.");
   this->path = filepath;
   this->name = std::filesystem::path(filepath).filename().string();
   if (!this->_loadHeader()) {
      _DEBUGMSG("Unable to read header.");
      return false;
   }
   _DEBUGMSG("Read file header.");
   this->uses_string_table = (bool)(this->flags & kFlag_LocalizedStringTable);
   //
   // TODO: need to define hardcoded forms so that references to them don't break, OR 
   // special-case them in whatever code we write to handle references between forms
   //
   {
      // we *do* load *some* records in this function, so we need to register with the allocator
      // use an unnamed block so we unregister before firing off the other threads
      auto registration = FormStubHeap::get().register_thread();
      //
      ObjectType ot;
      uint32_t   which_simple = 0;
      uint32_t   which_intcell = 0;
      uint32_t   which_world = 0;
      uint32_t   last_worldspace_id = 0;
      int16_t    last_ext_block_x = 0;
      int16_t    last_ext_block_y = 0;
      while (ot = this->nextRecordOrGroup(), ot != ObjectType::kObjectType_None) {
         auto& group = this->getCurrentGroup();
         if (ot == kObjectType_Record) {
            //
            // We should only hit records when we choose not to skip a group's contents. 
            // We use this to load worldspaces and their persistent/temporary cells.
            //
            auto& record = this->getCurrentRecord();
            if (record.signature() == 'WRLD') {
               last_worldspace_id = record.formID();
               continue;
            }
            formtype_t formType = signatureToFormType(record.signature());
            auto  stub = this->make_stub_for_record(*this);
            stub->groupInfo.groupType = group.header.type;
            switch (group.header.type) {
               case kESPGroupType_WorldChildren:
                  stub->groupInfo.parentFormID = last_worldspace_id;
                  break;
            }
            this->_insertForm(stub->formID, stub);
         }
         if (ot == kObjectType_Group) {
            if (group.header.type == kESPGroupType_WorldChildren) {
               //
               // Parse direct children of the worldspace (i.e. the persistent cell).
               //
               continue;
            } else if (group.header.type == kESPGroupType_ExteriorCellBlock) {
               last_ext_block_y = group.header.label & 0xFFFF;
               last_ext_block_x = group.header.label >> 0x10;
               continue;
            } else if (group.header.type == kESPGroupType_ExteriorCellSubBlock) {
               assert(last_worldspace_id && "Exterior Cell Block GRUP must follow a WRLD record.");
               auto& loader = this->worldspaceReaders[which_world];
               if (++which_world >= std::extent<decltype(this->worldspaceReaders)>::value)
                  which_world = 0;
               int16_t sub_x = group.header.label >> 0x10;
               int16_t sub_y = group.header.label & 0xFFFF;
               loader.add_group(last_worldspace_id, last_ext_block_x, last_ext_block_y, sub_x, sub_y, group.pos);
               group.skip();
               continue;
            } else if (group.header.type == kESPGroupType_InteriorCellBlock) { // Interior Cell Block
               {
                  auto parent = group.getParent();
                  assert(parent && "Bad interior-cell-block group nesting.");
                  assert(parent->header.type == kESPGroupType_FormsOfType && "Bad interior-cell-block group nesting.");
                  assert(_byteswap_ulong(parent->header.label) == 'CELL' && "Bad interior-cell-block group nesting.");
               }
               auto& loader = this->interiorCellReaders[which_intcell];
               if (++which_intcell >= std::extent<decltype(this->interiorCellReaders)>::value)
                  which_intcell = 0;
               loader.add_group(group.header.label, group.pos);
               group.skip();
               continue;
            } else if (group.header.type != kESPGroupType_FormsOfType) {
               group.skip();
               continue;
            }
            bool is_complex = false;
            switch (_byteswap_ulong(group.header.label)) {
               case 'CELL': // handled by the Interior Cell Block readers.
               case 'WRLD': // handled by the Worldspace readers.
                  //
                  // Don't skip the group; we want to read at least some of the content inside of it.
                  //
                  continue;
               case 'DIAL':
                  is_complex = true;
            }
            if (is_complex) {
               //
               // TODO: Consider splitting complex group structures up, e.g. bucketing worldspaces 
               // into different threads
               //
               this->complexReader.add_group(_byteswap_ulong(group.header.label), group.pos);
            } else {
               auto& loader = this->simpleReaders[which_simple];
               if (++which_simple >= std::extent<decltype(this->simpleReaders)>::value)
                  which_simple = 0;
               loader.add_group(_byteswap_ulong(group.header.label), group.pos);
            }
            //
            // Skip the group's actual content; the main thread only cares about locating the groups 
            // themselves and setting their contents up to be parsed on multiple threads.
            //
            group.skip();
            continue;
         }
      }
   }
   this->complexReader.start();
   for (uint32_t i = 0; i < std::extent<decltype(this->interiorCellReaders)>::value; i++)
      this->interiorCellReaders[i].start();
   for (uint32_t i = 0; i < std::extent<decltype(this->worldspaceReaders)>::value; i++)
      this->worldspaceReaders[i].start();
   for (uint32_t i = 0; i < std::extent<decltype(this->simpleReaders)>::value; i++)
      this->simpleReaders[i].start();
   for (uint32_t i = 0; i < std::extent<decltype(this->simpleReaders)>::value; i++)
      this->simpleReaders[i].wait_for();
   for (uint32_t i = 0; i < std::extent<decltype(this->interiorCellReaders)>::value; i++)
      this->interiorCellReaders[i].wait_for();
   for (uint32_t i = 0; i < std::extent<decltype(this->worldspaceReaders)>::value; i++)
      this->worldspaceReaders[i].wait_for();
   this->complexReader.wait_for();
   return true;
}
//
void TESPluginFile::modify_config(bool set, uint32_t flags) {
   if (set)
      this->config |= flags;
   else
      this->config &= ~flags;
}