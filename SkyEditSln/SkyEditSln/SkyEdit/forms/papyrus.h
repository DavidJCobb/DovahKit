#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

class TESPluginFile;

enum PapyrusPropertyType : uint8_t {
   kPapyrusPropertyType_Object = 1,
   kPapyrusPropertyType_String = 2,
   kPapyrusPropertyType_Int    = 3,
   kPapyrusPropertyType_Float  = 4,
   kPapyrusPropertyType_Bool   = 5,
   kPapyrusPropertyType_ArrayObject = 11,
   kPapyrusPropertyType_ArrayString = 12,
   kPapyrusPropertyType_ArrayInt    = 13,
   kPapyrusPropertyType_ArrayFloat  = 14,
   kPapyrusPropertyType_ArrayBool   = 15,
};

class PapyrusScriptData {
   public:
      class Script;
      class Property;
   public:
      int16_t version;
      int16_t objectFormat; // format of "object" property values
      std::vector<Script> scripts;
      // TODO: vector of fragments
      //
      bool load(TESPluginFile*); // assumes we're at a VMAD subrecord // TODO: needs to load fragments
      //
      void forEachScript(std::function<bool(Script*)>);
   public:
      enum ScriptStatus : uint8_t {
         kScriptStatus_Local   = 0,
         kScriptStatus_Altered = 1,
         kScriptStatus_Removed = 3,
      };
      enum PropertyStatus : uint8_t {
         kPropertyStatus_Edited  = 1,
         kPropertyStatus_Removed = 3,
      };
      struct PropertyObjectValue { // if objectFormat == 2, then the order of fields is reversed in the file
         uint32_t formID;
         uint16_t aliasID;
         uint16_t alwaysZero = 0;
         //
         bool load(PapyrusScriptData& owner, TESPluginFile*);
      };
      class Property {
         friend Script;
         public:
            std::string name;
            PapyrusPropertyType type;
            PropertyStatus status;
            void* value = nullptr;
            //
         private:
            bool load(PapyrusScriptData& owner, TESPluginFile*);
            //
         public:
            ~Property();
      };
      class Script {
         friend PapyrusScriptData;
         public:
            std::string  name;
            ScriptStatus status;
            std::vector<Property> properties;
            //
         private:
            bool load(PapyrusScriptData& owner, TESPluginFile*);
      };

};