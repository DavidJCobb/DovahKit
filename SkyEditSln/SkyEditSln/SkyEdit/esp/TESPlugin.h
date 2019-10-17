#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"
extern "C" {
   #include "../../zlib/zlib.h" // interproject ref
}

class TESPluginFile;
class TESPluginSubrecord;
class TESPluginRecord;
struct LStringRef;

enum ESPGroupType : int32_t {
   kESPGroupType_FormsOfType = 0,
   kESPGroupType_WorldChildren = 1,
   kESPGroupType_InteriorCellBlock = 2,
   kESPGroupType_InteriorCellSubBlock = 3,
   kESPGroupType_ExteriorCellBlock = 4,
   kESPGroupType_ExteriorCellSubBlock = 5,
   kESPGroupType_CellChilren = 6,
   kESPGroupType_TopicChildren = 7, // DIAL -> INFO
   kESPGroupType_CellPersistentChildren = 8,
   kESPGroupType_CellTemporaryChildren = 9,
};

struct TESPluginGroupHeader {
   public:
      uint32_t signature = 0; // should always be 'GRUP'
      uint32_t size;
      uint32_t label;
      ESPGroupType type;
      union {
         struct {
            uint8_t vcDay;
            uint8_t vcMonth;
            uint8_t vcLastEditor;
            uint8_t vcCurrentEditor;
         };
         uint32_t versionControl;
      };
      uint32_t unknown;
};
struct TESPluginRecordHeader {
   public:
      enum {
         kFlag_Compressed = 0x00040000,
      };
      //
      uint32_t signature = 0;
      uint32_t size;
      uint32_t flags;
      uint32_t formID = 0;
      union {
         struct {
            uint8_t vcDay;
            uint8_t vcMonth;
            uint8_t vcLastEditor;
            uint8_t vcCurrentEditor;
         };
         uint32_t versionControl;
      };
      uint16_t version;
      uint16_t unknown;
      //
      inline bool body_is_compressed() { return (bool)(this->flags & kFlag_Compressed); }
};

class TESPluginRecordBuffer {
   //
   // Record data can be compressed. This class exists to provide a 
   // uniform interface for loading record data whether or not the 
   // data is compressed.
   //
   // We load small amounts of the record's data into a buffer; if 
   // the record is compressed, then we decompress it while loading. 
   // This means that actually accessing the data works the same 
   // either way: pull from the buffer; if we reach its end, load 
   // the next chunk of data into the buffer.
   //
   friend TESPluginFile;
   public:
      TESPluginRecordBuffer(TESPluginFile* o) : owner(o) {}
   private:
      static constexpr uint32_t buffer_size = 256;
      //
      TESPluginFile* owner;
      uint8_t  bytes[buffer_size];
      bool     is_compressed = false;
      z_stream zlib_stream;
      int      zlib_result = Z_OK;
      uint32_t compressed_size = 0;
      //
      uint32_t chunk_pos    = 0; // position in loaded chunk
      uint32_t chunk_size   = 0; // size of loaded chunk (<= buffer_size)
      uint32_t stream_pos   = 0; // current position in source stream
      uint32_t stream_start = 0; // start position in the source stream
      uint32_t excerpt_size = 0; // size of the portion of the stream we're looking at
      //
      void advance();
   public:
      inline bool at_end() const { return this->stream_pos > this->stream_start + this->excerpt_size; }
      //
      void read(char* destination, uint32_t size);
      void skip(uint32_t size);
      void setup(bool compressed, uint32_t uncompressed_size, uint32_t compressed_size, uint32_t stream_start);
      //
      template<typename T> inline void read(T& field) { this->read((char*)&field, sizeof(field)); }
};

class TESPluginFile {
   friend TESPluginRecordBuffer;
   friend TESPluginRecord;
   friend TESPluginSubrecord;
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200, // SSE only
      };
   public:
      TESPluginFile();
      ~TESPluginFile();
      //
      // NOTE: We currently filter which GRUPs we load forms from. Look for a switch-
      // case on form signatures inside of (load).
      //
      bool load(const char* filepath);
      //
      // These next three functions are only useful during initial parsing; they rely 
      // on state. For example, (nextRecord) fails if we are not inside of a group.
      //
      bool nextGroup();
      bool nextRecord();
      bool nextSubrecord();
      //
      bool loadRecordAt(uint32_t pos); // use for TES4 during load, or use to load any record on-demand after all forms are known
      //
      void     setPos(uint32_t pos);
      uint32_t getPos();
      void skipBytes(uint32_t count);
      bool isEOF();
      bool is_good();
      //
   protected:
      FILE* fileHandle;
      TESPluginRecordBuffer recordBuffer = TESPluginRecordBuffer(this);
      struct {
         TESPluginGroupHeader header;
         uint32_t pos;
         uint32_t end;
      } group;
      struct {
         TESPluginRecordHeader header;
         uint32_t headPos; // position in the file
         uint32_t bodyPos; // position in the body
         uint32_t end;
      } record;
      struct {
         uint32_t signature = 0;
         uint32_t size = 0;
         uint32_t pos;
         uint32_t end;
      } subrecord;
      //
      bool _loadHeader();
      void read(char* buffer, uint32_t size) {
         fread(buffer, size, 1, this->fileHandle);
      }
      template<typename T> void read(T& field, uint32_t size) {
         fread(&field, size, 1, this->fileHandle);
      }
      template<typename T> void read(T& field) {
         fread(&field, sizeof(field), 1, this->fileHandle);
      }
      //
      // Loaded data:
      //
      std::map<formtype_t, std::map<uint32_t, FormStub*>> formsByType;
      //
   public:
      //
      // Loading:
      //
      inline const TESPluginGroupHeader&  getGroupHeader()  { return this->group.header; }
      inline const TESPluginRecordHeader& getRecordHeader() { return this->record.header; }
      //
      TESPluginRecord    getCurrentRecord();
      TESPluginSubrecord getCurrentSubrecord();
      //
      // Loaded data:
      //
      uint32_t flags = 0;
      float    fileVersion = 0.94F;
      uint32_t recordCount = 0;
      uint32_t nextFormID;
      char     authorName[512];
      char     description[512];
      std::vector<std::string> masters;
      // TODO: ONAM
      uint32_t subINTV;
      uint32_t subINCC;
      //
      FormStub* getForm(formtype_t formType, uint32_t formID) const;
      void forEachFormOfType(formtype_t formType, std::function<bool(FormStub*)>);
};

class TESPluginSubrecord { // interface for the currently-loaded subrecord
   private:
      TESPluginFile* const file;
      //
      bool _check() const {
         return !(this->file->recordBuffer.at_end());
         //return this->file->getPos() < this->file->subrecord.end;
      }
      bool _check(uint32_t bytes) const {
         return !(this->file->recordBuffer.at_end());
         //return this->file->getPos() + bytes < this->file->subrecord.end;
      }
   public:
      TESPluginSubrecord(TESPluginFile* f) : file(f) {};
      //
      inline operator bool() const { return this->file != nullptr; }
      //
      inline uint32_t offset() const { return file->subrecord.pos; }
      inline uint32_t signature() const { return file->subrecord.signature; }
      inline uint32_t size() const { return file->subrecord.size; }
      //
      inline bool is_in_bounds() { return this->_check() && this->file->is_good(); }
      //
      inline uint32_t containing_record_signature() const { return file->record.header.signature; }
      //
      bool to_string(std::string& field);
      bool to_string(LStringRef& field); // TODO: implement string table support
      //
      bool skip_bytes(uint32_t count);
      bool read(char* buffer, uint32_t size) {
         if (!this->_check(size))
            return false;
         this->file->recordBuffer.read(buffer, size);
         return true;
      }
      template<typename T> bool read(T& field, uint32_t size) {
         if (!this->_check(size))
            return false;
         this->file->recordBuffer.read((char*)&field, size);
         return true;
      }
      template<typename T> bool read(T& field) {
         if (!this->_check(sizeof(field)))
            return false;
         this->file->recordBuffer.read((char*)&field, sizeof(field));
         return true;
      }
      bool read_wstring(std::string& field); // uint16_t length; char str[length]; // length does not include a null-terminator

      //
      // The functions below allow you to manually manage bounds-checking: if you need to read multiple 
      // fields in sequence, then it might be a millisecond or two faster to do a single bounds-check 
      // at the start, and then do unchecked reads for the fields, e.g.
      //
      //    uint32_t foo;
      //    uint32_t bar;
      //    if (!subrecord.has_bytes(sizeof(foo) + sizeof(bar)))
      //       return false;
      //    subrecord.unchecked_read(foo);
      //    subrecord.unchecked_read(bar);
      //
      // Of course, you'll have to be careful if you go copying and pasting read code. Is that risk 
      // worth a few milliseconds per form, over thousands of forms? Sounds like it to me, but I can 
      // always redesign if it turns out to cause too many problems to be worth it.
      //

      inline bool has_bytes(uint32_t count) const { return this->_check(count); }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      void unchecked_read(char* buffer, uint32_t size) {
         this->file->recordBuffer.read(buffer, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field, uint32_t size) {
         this->file->recordBuffer.read((char*)&field, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field) {
         this->file->recordBuffer.read((char*)&field, sizeof(field));
      }
};
class TESPluginRecord { // interface for the currently-loaded record
   private:
      TESPluginFile* const file;
   public:
      TESPluginRecord(TESPluginFile* f) : file(f) {};
      //
      inline operator bool() const { return this->file != nullptr; }
      //
      inline TESPluginSubrecord next_subrecord() {
         if (this->file->nextSubrecord())
            return TESPluginSubrecord(this->file);
         return TESPluginSubrecord(nullptr);
      }
      //
      inline uint32_t flags() const { return file->record.header.flags; }
      inline uint32_t formID() const { return file->record.header.formID; }
      inline uint32_t signature() const { return this->file->record.header.signature; }
      inline uint32_t size() const { return this->file->record.header.size; }
      //
      uint32_t peek_next_subrecord_type(); // TODO: THIS IS BROKEN FOR COMPRESSED RECORDS
};