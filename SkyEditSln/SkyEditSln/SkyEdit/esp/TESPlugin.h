#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"

class  TESPluginFile;
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
      uint32_t recordHeadPos;
      uint32_t recordBodyPos;
      uint32_t subrecordPos;
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