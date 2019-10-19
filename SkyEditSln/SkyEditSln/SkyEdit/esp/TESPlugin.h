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

constexpr int MAX_ESP_FILE_GROUP_DEPTH = 5;

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
   kESPGroupType_CellChildren = 6,
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

class TESPluginGroup {
   friend TESPluginFile;
   protected:
      void initialize(TESPluginFile* f) { this->owner = f; }
      //
      void reset() {
         this->header.signature = 0;
      }
      void skip();
      //
      TESPluginFile* owner;
      //
      TESPluginGroupHeader header;
      uint32_t pos;
      uint32_t end;
      //
      operator bool() const noexcept { return this->header.signature != 0; }
      //
      uint32_t depth() const;
      void to_string(std::string&) const;
};
class TESPluginRecord {
   friend TESPluginFile;
   protected:
      TESPluginRecord(TESPluginFile& file) : owner(file) {}
      //
      TESPluginFile& owner;
      //
      TESPluginRecordHeader header;
      uint32_t headPos; // position in the file
      uint32_t bodyPos; // position in the body
      uint32_t end;
      //
      cobb::generic_buffer data;
      uint32_t offset = 0;
      //
      void reset() {
         this->data.free();
         this->offset = 0;
         this->header.signature = 0;
      }
      //
   public:
      //
      // Disallow copying to avoid bad memory management on the generic_buffer.
      //
      TESPluginRecord& operator=(const TESPluginRecord& other) = delete; // no copy
      TESPluginRecord& operator=(TESPluginRecord& other) = delete; // no copy
      TESPluginRecord(TESPluginRecord& other) = delete; // no copy
      //
      operator bool() const noexcept { return this->header.signature != 0; }
      //
      bool is_in_bounds() const noexcept {
         return this->offset < this->data.size();
      }
      bool is_in_bounds(uint32_t room_for) const noexcept {
         return this->offset + room_for <= this->data.size();
      }
      inline uint32_t flags() const noexcept { return this->header.flags; }
      inline uint32_t formID() const noexcept { return this->header.formID; }
      inline uint32_t signature() const noexcept { return this->header.signature; }
      inline uint32_t size() const noexcept { return this->header.size; }
      //
      inline uint32_t stream_pos() const noexcept { return this->offset + this->bodyPos; }
      //
      bool read(void* destination, uint32_t size);
      inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
      template<typename T> bool read(T& field) {
         return this->read(&field, sizeof(T));
      }
      bool skip(uint32_t bytes);
      //
      void unchecked_read(void* destination, uint32_t size);
      template<typename T> void unchecked_read(T& field) {
         this->unchecked_read(&field, sizeof(T));
      }
      //
      TESPluginSubrecord& next_subrecord() const;
      uint32_t peek_next_subrecord_type();
};
class TESPluginSubrecord {
   friend TESPluginFile;
   protected:
      TESPluginSubrecord(TESPluginFile& file) : owner(file) {}
      //
      TESPluginFile& owner;
      //
      struct {
         uint32_t signature = 0;
         uint32_t size = 0;
      } header;
      uint32_t pos;
      uint32_t end;
      //
   public:
      TESPluginSubrecord& operator=(const TESPluginSubrecord& other) = delete; // no copy
      TESPluginSubrecord& operator=(TESPluginSubrecord& other) = delete; // no copy
      TESPluginSubrecord(TESPluginSubrecord& other) = delete; // no copy
   public:
      TESPluginRecord& get_containing_record() const;
      //
      inline uint32_t offset() const noexcept { return this->pos; }
      inline uint32_t end_pos() const noexcept { return this->end; }
      inline uint32_t signature() const noexcept { return this->header.signature; }
      inline uint32_t size() const noexcept { return this->header.size; }
      //
      inline operator bool() const { return this->header.signature != 0; }
      //
      inline bool is_in_bounds() const {
         return this->get_containing_record().stream_pos() < this->end;
      }
      inline bool is_in_bounds(uint32_t size) const {
         return this->get_containing_record().stream_pos() + size <= this->end;
      }
      //
      inline uint32_t containing_record_signature() const { return this->get_containing_record().signature(); }
      //
      bool to_string(std::string& field);
      bool to_string(LStringRef& field); // TODO: implement string table support
      //
      inline bool skip_bytes(uint32_t count) const { return this->get_containing_record().skip(count); }
      inline bool read(void* buffer, uint32_t size) const {
         return this->get_containing_record().read(buffer, size);
      }
      inline bool read(char* buffer, uint32_t size) { return this->read((void*)buffer, size); }
      template<typename T> inline bool read(T& field) const {
         return this->get_containing_record().read(field);
      }
      template<typename T> inline void unchecked_read(T& field) const {
         return this->get_containing_record().unchecked_read(field);
      }
      bool read_wstring(std::string& field); // uint16_t length; char str[length]; // length does not include a null-terminator

      //
      // The functions below allow you to manually manage bounds-checking: if you need to read multiple 
      // fields in sequence, then it might be a millisecond or two faster to do a single bounds-check 
      // at the start, and then do unchecked reads for the fields, e.g.
      //
      //    uint32_t foo;
      //    uint32_t bar;
      //    if (!subrecord.is_in_bounds(sizeof(foo) + sizeof(bar)))
      //       return false;
      //    subrecord.unchecked_read(foo);
      //    subrecord.unchecked_read(bar);
      //
      // Of course, you'll have to be careful if you go copying and pasting read code. Is that risk 
      // worth a few milliseconds per form, over thousands of forms? Sounds like it to me, but I can 
      // always redesign if it turns out to cause too many problems to be worth it.
      //
};

class TESPluginFile {
   friend TESPluginGroup;
   friend TESPluginRecord;
   friend TESPluginSubrecord;
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200, // SSE only
      };
      enum ObjectType {
         kObjectType_None,
         kObjectType_Group,
         kObjectType_Record,
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
      ObjectType nextRecordOrGroup();
      bool nextSubrecord();
      bool loadRecordAt(uint32_t pos); // for FormStub
      //
      void     setPos(uint32_t pos);
      uint32_t getPos();
      void     rewind(uint32_t by);
      void skipBytes(uint32_t count);
      bool isEOF();
      bool is_good();
      //
   protected:
      FILE* fileHandle;
      //
      // These next structs contain parsing state for groups, records, and subrecords; 
      // they are also provided (through getters) to form-loading code as interfaces. 
      // In fact, under the hood, they're just interfaces to this class.
      //
      // Note that their constructors require a const reference to the containing 
      // TESPluginFile, but they don't support copying, so the constructor for 
      // TESPluginFile must use the NAME : field(value) {} syntax at its definition 
      // (NOT the declaration; check the CPP file, not this H file).
      //
      TESPluginGroup groups[MAX_ESP_FILE_GROUP_DEPTH];
      TESPluginRecord    record;
      TESPluginSubrecord subrecord;
      //
      uint32_t lastPotentialGroupParent = 0; // form ID: CELL, WRLD, DIAL
      //
   protected:
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
      inline const TESPluginRecordHeader& getRecordHeader() { return this->record.header; }
      //
      inline TESPluginGroup& getCurrentGroup() {
         for (signed int i = std::extent<decltype(this->groups)>::value - 1; i >= 0; i--) {
            auto& group = this->groups[i];
            if (group)
               return group;
         }
         //
         // We have to return a group& even if we're not in one, but groups have an 
         // operator bool, so you can do
         //
         // if (auto g = file->getCurrentGroup()) {
         //    //
         //    // ...
         //    //
         // }
         //
         return this->groups[0];
      }
      inline TESPluginRecord& getCurrentRecord() { return this->record; }
      inline TESPluginSubrecord& getCurrentSubrecord() { return this->subrecord; }
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