#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"

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
      void setPos(uint32_t pos);
      uint32_t getPos();
      void skipBytes(uint32_t count);
      bool isEOF();
      bool is_good();
      //
      void read(char* buffer, uint32_t size) {
         fread(buffer, size, 1, this->fileHandle);
      }
      template<typename T> void read(T& field, uint32_t size) {
         fread(&field, size, 1, this->fileHandle);
      }
      template<typename T> void read(T& field) {
         fread(&field, sizeof(field), 1, this->fileHandle);
      }
      void readStringSubrecord(std::string& field);
      void readStringSubrecord(LStringRef& field); // TODO: implement string table support
      void readWString(std::string& field);
      //
   protected:
      //
      // Loading state:
      //
      FILE* fileHandle;
      TESPluginGroupHeader  group;  // header for last parsed/loaded group
      TESPluginRecordHeader record; // header for last parsed/loaded record
      uint32_t groupPos;
      uint32_t groupEnd;
      uint32_t recordHeadPos;
      uint32_t recordBodyPos;
      uint32_t recordEnd;
      uint32_t subrecordPos; // position of the start of the subrecord's contents
      uint32_t subrecordEnd;
      uint32_t subrecordSignature = 0;
      uint32_t subrecordSize = 0;
      //
      bool _loadHeader();
      //
      // Loaded data:
      //
      std::map<formtype_t, std::map<uint32_t, FormStub*>> formsByType;
      //
   public:
      //
      // Loading:
      //
      inline const TESPluginGroupHeader&  getGroupHeader()  { return this->group; }
      inline const TESPluginRecordHeader& getRecordHeader() { return this->record; }
      inline uint32_t getGroupPos() const { return this->groupPos; }
      inline uint32_t getRecordHeadPos() const { return this->recordHeadPos; }
      inline uint32_t getRecordBodyPos() const { return this->recordBodyPos; }
      inline uint32_t getSubrecordPos() const { return this->subrecordPos; }
      inline uint32_t getSubrecordType() const { return this->subrecordSignature; }
      inline uint32_t getSubrecordSize() const { return this->subrecordSize; }
      //
      inline uint32_t getGroupEnd() const { return this->groupPos + this->group.size; }
      inline uint32_t getRecordEnd() const { return this->recordBodyPos + this->record.size; }
      inline uint32_t getSubrecordEnd() const { return this->subrecordPos + this->subrecordSize; }
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
         return this->file->getPos() < this->file->subrecordEnd;
      }
      bool _check(uint32_t bytes) const {
         return this->file->getPos() + bytes < this->file->subrecordEnd;
      }
   public:
      TESPluginSubrecord(TESPluginFile* f) : file(f) {};
      //
      inline operator bool() const { return this->file != nullptr; }
      //
      inline uint32_t offset() const { return file->subrecordPos; }
      inline uint32_t signature() const { return file->subrecordSignature; }
      inline uint32_t size() const { return file->subrecordSize; }
      //
      inline bool is_in_bounds() { return this->_check() && this->file->is_good(); }
      //
      inline uint32_t containing_record_signature() const { return file->record.signature; }
      //
      bool to_string(std::string& field);
      bool to_string(LStringRef& field); // TODO: implement string table support
      //
      bool skipBytes(uint32_t count);
      bool read(char* buffer, uint32_t size) {
         if (!this->_check(size))
            return false;
         this->file->read(buffer, size);
         return true;
      }
      template<typename T> bool read(T& field, uint32_t size) {
         if (!this->_check(size))
            return false;
         this->file->read(field, size);
         return true;
      }
      template<typename T> bool read(T& field) {
         if (!this->_check(sizeof(field)))
            return false;
         this->file->read(field);
         return true;
      }
      bool read_wstring(std::string& field);

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
         this->file->read(buffer, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field, uint32_t size) {
         this->file->read(field, size);
      }
      //
      // Use only if you've already called (has_bytes) to check that the data you want to read is in-bounds.
      template<typename T> void unchecked_read(T& field) {
         this->file->read(field);
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
      inline uint32_t signature() const { return this->file->record.signature; }
      inline uint32_t size() const { return this->file->record.size; }
      //
      uint32_t peek_next_subrecord_type();
};