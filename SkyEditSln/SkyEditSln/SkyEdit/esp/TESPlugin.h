#pragma once
#include <cstdint>
#include <functional>
#include <fstream>
#include <map>
#include <string>
#include <vector>
#include "../formstub.h"
#include "../forms/types.h"

class TESPlugin;

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
      bool load(std::ifstream& stream);
};

class esp_istream : public std::ifstream {
   private:
      TESPlugin& owner;
      //
      TESPluginGroupHeader  currentGroup;
      uint32_t currentGroupOffset  = 0;
      //
      TESPluginRecordHeader currentRecord;
      uint32_t currentRecordOffset = 0;
      uint32_t recordHeaderOffset = 0;
      //
      uint32_t subrecordOffset    = 0;
      uint32_t subrecordSignature = 0;
      uint32_t subrecordSize      = 0;
      uint32_t lastFieldExSize    = 0; // used for XXXX fields
      //
      uint32_t getGroupEndPos() {
         return this->currentGroupOffset + this->currentGroup.size;
      }
      uint32_t getRecordEndPos() {
         return this->currentRecordOffset + this->currentRecord.size;
      }
      uint32_t getSubrecordEndPos() {
         return this->subrecordOffset + this->subrecordSize;
      }
      //
      bool isPastRecord();
      bool isPastSubrecord();
      //
   public:
      esp_istream(TESPlugin& plugin) : owner(plugin) {};
      bool nextGroup();
      bool nextRecord(bool notInGroup = false);
      bool nextSubrecord();
      //
      void clearParseState();
      //
      const TESPluginGroupHeader& getGroupHeader() {
         return this->currentGroup;
      }
      uint32_t getGroupType() {
         return this->currentGroup.signature;
      }
      const TESPluginRecordHeader& getRecordHeader() {
         return this->currentRecord;
      }
      uint32_t getRecordHeaderOffset() {
         return this->recordHeaderOffset;
      }
      uint32_t getRecordBodyOffset() {
         return this->currentRecordOffset;
      }
      uint32_t getRecordType() {
         return this->currentRecord.signature;
      }
      uint32_t getSubrecordType() {
         return this->subrecordSignature;
      }
      uint32_t getSubrecordSize() {
         return this->subrecordSize;
      }
      //
      template<typename T> void read_value(T& field) {
         this->read((char*)&field, sizeof(field));
      }
      void read_string_subrecord(std::string& field) {
         field.clear();
         if (!this->subrecordSignature)
            return;
         field.resize(this->subrecordSize);
         this->read(const_cast<char*>(field.data()), this->subrecordSize);
      }
};

class TESPlugin {
   friend FormStub;
   //
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200,
      };
      TESPlugin();
      ~TESPlugin();
   private:
      esp_istream file;
      std::map<formtype_t, std::map<uint32_t, FormStub*>> formsByType;
   public:
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
      //
      void load(const char* filepath);
   private:
      bool loadHeader(esp_istream& stream);
};