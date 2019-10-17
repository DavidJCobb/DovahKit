#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"
#include "../helpers/memory.h"
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

class TESPluginFile {
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
      struct {
         TESPluginGroupHeader header;
         uint32_t pos;
         uint32_t end;
      } group;
      //
      // TODO: handle nested groups
      //
      struct {
         TESPluginRecordHeader header;
         uint32_t headPos; // position in the file
         uint32_t bodyPos; // position in the body
         uint32_t end;
         //
         cobb::generic_buffer data;
         uint32_t offset = 0;
      } record;
      struct {
         uint32_t signature = 0;
         uint32_t size = 0;
         uint32_t pos;
         uint32_t end;
      } subrecord;
      uint32_t lastPotentialGroupParent = 0; // form ID: CELL, WRLD, DIAL
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
      bool skipFromRecord(uint32_t bytes) {
         auto& r = this->record;
         if (!r.data)
            return false;
         if (r.offset + bytes > r.data.size())
            return false;
         r.offset += bytes;
         return true;
      }
      bool readFromRecord(char* buffer, uint32_t size) {
         auto& r = this->record;
         if (!r.data)
            return false;
         if (r.offset + size > r.data.size())
            return false;
         auto destination = (std::ptrdiff_t)r.data + r.offset;
         memcpy(buffer, (void*)destination, size);
         r.offset += size;
         return true;
      }
      template<typename T> bool readFromRecord(T& field) {
         return this->readFromRecord((char*)&field, sizeof(T));
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
         auto& r = this->file->record;
         return r.offset < r.data.size();
      }
      bool _check(uint32_t bytes) const {
         auto& r = this->file->record;
         return r.offset < r.data.size();
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
         this->file->readFromRecord(buffer, size);
         return true;
      }
      template<typename T> bool read(T& field, uint32_t size) {
         if (!this->_check(size))
            return false;
         this->file->readFromRecord((char*)&field, size);
         return true;
      }
      template<typename T> bool read(T& field) {
         if (!this->_check(sizeof(field)))
            return false;
         this->file->readFromRecord((char*)&field, sizeof(field));
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
         this->file->readFromRecord(buffer, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field, uint32_t size) {
         this->file->readFromRecord((char*)&field, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field) {
         this->file->readFromRecord((char*)&field, sizeof(field));
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
      uint32_t peek_next_subrecord_type();
};