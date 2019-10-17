#include "TESPlugin.h"
#include <algorithm>
#include <stdexcept>
#include <iostream> // for testing
#include "../output.h"
#include "../forms/components.h"

void _Debug(const char* msg) {
   std::cout << msg << std::endl;
}

void TESPluginRecordBuffer::setup(bool compressed, uint32_t uncompressed_size, uint32_t compressed_size, uint32_t stream_start) {
   this->stream_start = stream_start;
   this->excerpt_size = uncompressed_size;
   this->stream_pos   = stream_start;
   //
   this->is_compressed = compressed;
   inflateEnd(&this->zlib_stream);
   if (compressed) {
      auto& zs = this->zlib_stream;
      zs.zalloc   = Z_NULL;
      zs.zfree    = Z_NULL;
      zs.opaque   = Z_NULL;
      zs.avail_in = 0;
      zs.next_in  = Z_NULL;
      this->zlib_result = inflateInit(&zs);
   }
   this->advance();
}
void TESPluginRecordBuffer::advance() {
   this->chunk_pos = 0;
   if (!this->is_compressed) {
      if (this->at_end())
         return;
      auto end = this->stream_start + this->excerpt_size;
      this->chunk_size = (std::min)(end - this->stream_pos, buffer_size);
      owner->read(this->bytes, this->chunk_size);
      return;
   }
   switch (this->zlib_result) {
      case Z_OK:
         break;
      case Z_STREAM_END:
      case Z_DATA_ERROR:
      case Z_MEM_ERROR:
         inflateEnd(&this->zlib_stream);
         this->chunk_size = 0;
         return;
   }
   char _raw[buffer_size];
   auto& zs = this->zlib_stream;
   uint32_t size_to_load = (std::min)(this->compressed_size - (this->stream_pos - this->stream_start), buffer_size);
   zs.avail_in = fread(_raw, 1, size_to_load, owner->fileHandle);
   if (ferror(owner->fileHandle)) {
      inflateEnd(&zs);
      this->chunk_size = 0; // TODO: signal error?
      return;
   }
   if (zs.avail_in == 0) {
      this->chunk_size = 0;
      return;
   }
   zs.next_in = (Bytef*)_raw;
   //
   // Run inflate() on the raw data until the output buffer has no remaining space:
   //
   uint32_t loaded_size = 0;
   do {
      zs.avail_out = buffer_size;
      zs.next_out  = this->bytes;
      this->zlib_result = inflate(&zs, Z_NO_FLUSH);
      //assert(this->zlib_result != Z_STREAM_ERROR);
      switch (this->zlib_result) {
         case Z_NEED_DICT:
            this->zlib_result = Z_DATA_ERROR;
            //
            // fall through:
            //
         case Z_DATA_ERROR:
         case Z_MEM_ERROR:
            inflateEnd(&zs);
            return; // TODO: signal error?
      }
      loaded_size = buffer_size - zs.avail_out;
   } while (zs.avail_out == 0);
}
void TESPluginRecordBuffer::read(char* destination, uint32_t size) {
   uint32_t diff = this->chunk_size - this->chunk_pos;
   if (size > diff) { // data we wish to read is larger than what's left of the current chunk
      memcpy(destination, this->bytes + this->chunk_pos, diff);
      if (!this->is_compressed)
         this->stream_pos += diff;
      destination += diff;
      size        -= diff;
      this->advance();
      //
      while (size > buffer_size) { // data left to read is larger than a chunk
         memcpy(destination, this->bytes, buffer_size);
         if (!this->is_compressed)
            this->stream_pos += diff;
         destination += buffer_size;
         size        -= buffer_size;
         this->advance();
      }
   }
   if (size) {
      memcpy(destination, this->bytes + this->chunk_pos, size);
      if (!this->is_compressed)
         this->stream_pos += diff;
      this->chunk_pos  += size;
   }
}
void TESPluginRecordBuffer::skip(uint32_t size) {
   uint32_t diff = this->chunk_size - this->chunk_pos;
   if (size > diff) { // data we wish to read is larger than what's left of the current chunk
      if (!this->is_compressed)
         this->stream_pos += diff;
      size -= diff;
      this->advance();
      //
      while (size > buffer_size) { // data left to read is larger than a chunk
         if (!this->is_compressed)
            this->stream_pos += diff;
         size -= buffer_size;
         this->advance();
      }
   }
   if (size) {
      if (!this->is_compressed)
         this->stream_pos += diff;
      this->chunk_pos  += size;
   }
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
   this->group.header.signature = 0;
   this->record.header.signature = 0;
   this->subrecord.signature = 0;
   //
   this->setPos(pos);
   this->record.headPos = pos;
   auto& r = this->record.header;
   this->read(r);
   r.signature = _byteswap_ulong(r.signature);
   this->record.bodyPos = this->getPos();
   this->record.end = this->record.bodyPos + r.size;
   if (!this->is_good())
      return false;
   if (r.body_is_compressed()) {
      uint32_t decompressed_size;
      this->read(decompressed_size);
      this->recordBuffer.setup(true, decompressed_size, r.size - 4, this->record.bodyPos + 4);
   } else {
      this->recordBuffer.setup(false, r.size, r.size, this->record.bodyPos);
   }
   return true;
}
bool TESPluginFile::nextGroup() {
   auto& g = this->group.header;
   if (g.signature) {
      //_DEBUGMSG("Skipped group spanning from %d to %d.", this->groupPos, this->getGroupEnd());
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
   auto& r = this->record.header;
   if (r.signature) {
      //_DEBUGMSG("Skipped record body spanning from %d to %d.", this->recordBodyPos, this->getRecordEnd());
      this->setPos(this->record.end);
      r.signature = 0;
      //
      this->subrecord.signature = 0;
      //
      if (!this->is_good())
         return false;
   }
   this->record.headPos = this->getPos();
   if (this->record.headPos >= this->group.end)
      return false;
   this->read(r);
   r.signature = _byteswap_ulong(r.signature);
   this->record.bodyPos = this->getPos();
   this->record.end = this->record.bodyPos + r.size;
   if (!this->is_good())
      return false;
   if (r.body_is_compressed()) {
      uint32_t decompressed_size;
      this->read(decompressed_size);
      this->recordBuffer.setup(true, decompressed_size, r.size - 4, this->record.bodyPos + 4);
   } else {
      this->recordBuffer.setup(false, r.size, r.size, this->record.bodyPos);
   }
   return true;
}
bool TESPluginFile::nextSubrecord() {
   auto& rb = this->recordBuffer;
   if (this->subrecord.signature) {
      //this->setPos(this->subrecord.end);
      rb.skip(this->subrecord.end - rb.stream_pos);
      this->subrecord.signature = 0;
      //
      if (!this->is_good())
         return false;
   }
   //if (this->getPos() >= this->record.end)
   //   return false;
   if (rb.stream_pos >= this->record.end)
      return false;
   uint16_t size;
   //this->read(this->subrecord.signature);
   //this->read(size);
   rb.read(this->subrecord.signature);
   rb.read(size);
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
      rb.read(this->subrecord.size);
      //
      // Get the next subrecord.
      //
      //this->read(this->subrecord.signature);
      //this->skipBytes(2); // an XXXX-prefixed subrecord has no length of its own
      rb.read(this->subrecord.signature);
      rb.skip(2);
   }
   this->subrecord.signature = _byteswap_ulong(this->subrecord.signature);
   //this->subrecord.pos = this->getPos();
   this->subrecord.pos = rb.stream_pos;
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
            //
            // TODO
            //
            break;
         case 'CNAM': // author/creator
            subrecord.read(this->authorName, subrecord.size());
            break;
         case 'SNAM': // description
            subrecord.read(this->description, subrecord.size());
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
            subrecord.read(this->subINTV);
            break;
         case 'INCC':
            subrecord.read(this->subINCC);
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
               this->read(buffer, this->subrecord.size);
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
   this->file->recordBuffer.skip(count);
   return true;
}
bool TESPluginSubrecord::read_wstring(std::string& field) {
   field.clear();
   uint16_t length;
   this->read(length);
   if (!this->_check(length))
      return false;
   field.resize(length);
   this->file->recordBuffer.read(const_cast<char*>(field.data()), length);
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
   if (this->file->subrecord.end < this->file->record.end) {
      auto pos = this->file->getPos();
      this->file->setPos(this->file->subrecord.end);
      uint32_t signature;
      this->file->read(signature);
      this->file->setPos(pos);
      return signature;
   }
   return 0;
}