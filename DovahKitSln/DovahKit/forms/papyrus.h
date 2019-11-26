#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>
#include "../helpers/scoped_enum.h"
#include "types.h"

class FormStub;
class TESPluginFile;
class TESPluginSubrecord;

// Macro used to optimize the building of Use Info for VMAD subrecords. Some Papyrus fragment data 
// types don't contain any form IDs, so if we know that the fragment data is always the last thing 
// in the VMAD subrecord, then we can just early-out for these types. If Skyrim Special is ever 
// updated to put additional data after the fragment data, you'll want to set this macro to 0 to 
// re-enable the code for skipping fragment data types that lack form IDs "by hand."
#define PAPYRUS_FRAGMENT_DATA_IS_ALWAYS_AT_THE_END_OF_VMAD 1

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

enum class papyrus_fragment_type : uint8_t {
   undefined,
   info,
   package,
   perk,
   quest,
   scene,
};
class PapyrusScriptData;
class PapyrusFragmentData {
   public:
      const papyrus_fragment_type type;
      //
      PapyrusFragmentData(const papyrus_fragment_type t) : type(t) {};
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) = 0;
};

class PapyrusScriptData {
   public:
      class Script;
      class Property;
   public:
      int16_t version;
      int16_t objectFormat; // format of "object" property values
      std::vector<Script> scripts;
      PapyrusFragmentData* fragmentData = nullptr;
      //
      bool load(TESPluginSubrecord&); // assumes we're at a VMAD subrecord // TODO: needs to load fragments
      static void generateUseInfo(TESPluginSubrecord&, FormStub*);
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
         form_id_t formID;
         uint16_t  aliasID;
         uint16_t  alwaysZero = 0;
         //
         bool load(PapyrusScriptData& owner, TESPluginSubrecord&);
      };
      class Property {
         friend Script;
         public:
            std::string name;
            PapyrusPropertyType type;
            PropertyStatus status;
            void* value = nullptr;
            //
            bool load(PapyrusScriptData& owner, TESPluginSubrecord&);
            //
            ~Property();
      };
      class Script {
         friend PapyrusScriptData;
         public:
            std::string  name;
            ScriptStatus status;
            std::vector<Property> properties;
            //
            bool load(PapyrusScriptData& owner, TESPluginSubrecord&);
      };

};

struct PapyrusBasicFragmentEntry {
   uint8_t     unknown;
   std::string script;
   std::string function;
};
SCOPE_ENUM(papyrus_topic_info_fragment_flags, enum papyrus_topic_info_fragment_flags {
   has_begin_fragment = 1,
   has_end_fragment   = 2,
});
SCOPE_ENUM(papyrus_package_fragment_flags, enum papyrus_package_fragment_flags {
   has_begin_fragment  = 1,
   has_end_fragment    = 2,
   has_change_fragment = 4,
});
SCOPE_ENUM(papyrus_scene_fragment_flags, enum papyrus_scene_fragment_flags {
      has_begin_fragment = 1,
      has_end_fragment   = 2,
});
class PapyrusTopicInfoFragmentData : PapyrusFragmentData {
   public:
      PapyrusTopicInfoFragmentData() : PapyrusFragmentData(papyrus_fragment_type::info) {};
      typedef PapyrusBasicFragmentEntry fragment_type;
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) override;
      //
      typedef papyrus_topic_info_fragment_flags Flags;
      //
      uint8_t unknown = 2;
      uint8_t flags   = 0;
      std::string   filename;
      fragment_type onBeginFragment;
      fragment_type onEndFragment;
};
class PapyrusPackageFragmentData : PapyrusFragmentData {
   public:
      PapyrusPackageFragmentData() : PapyrusFragmentData(papyrus_fragment_type::package) {};
      typedef PapyrusBasicFragmentEntry fragment_type;
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) override;
      //
      typedef papyrus_package_fragment_flags Flags;
      //
      uint8_t unknown = 2;
      uint8_t flags = 0;
      std::string filename;
      fragment_type onBeginFragment;
      fragment_type onEndFragment;
      fragment_type onChangeFragment;
};
class PapyrusPerkFragmentData : PapyrusFragmentData {
   public:
      PapyrusPerkFragmentData() : PapyrusFragmentData(papyrus_fragment_type::perk) {};
      struct fragment_type {
         uint16_t index;
         uint16_t unknown02;
         uint8_t  unknown04;
         std::string filename;
         std::string function;
      };
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) override;
      //
      uint8_t unknown = 2;
      std::string filename;
      std::vector<fragment_type> fragments;
};
class PapyrusQuestFragmentData : PapyrusFragmentData {
   public:
      PapyrusQuestFragmentData() : PapyrusFragmentData(papyrus_fragment_type::quest) {};
      struct fragment_type {
         uint16_t index;
         uint16_t unknown02;
         uint32_t logEntry;
         uint8_t  unknown08;
         std::string filename;
         std::string function;
      };
      struct alias_type {
         PapyrusScriptData::PropertyObjectValue alias;
         int16_t version;
         int16_t objFormat;
         std::vector<PapyrusScriptData::Script> scripts;
      };
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) override;
      //
      uint8_t unknown = 2;
      std::string filename;
      std::vector<fragment_type> fragments;
      std::vector<alias_type>    aliasScriptData;
};
class PapyrusSceneFragmentData : PapyrusFragmentData {
   public:
      PapyrusSceneFragmentData() : PapyrusFragmentData(papyrus_fragment_type::scene) {};
      typedef PapyrusBasicFragmentEntry fragment_type;
      struct phase_fragment_type {
         uint8_t  unknown00;
         uint32_t phase; // zero-indexed internally; one-indexed in UI
         uint8_t  unknown05;
         std::string filename;
         std::string function;
      };
      //
      virtual void load(PapyrusScriptData& owner, TESPluginSubrecord&) override;
      //
      typedef papyrus_scene_fragment_flags Flags;
      //
      uint8_t unknown = 2;
      uint8_t flags = 0;
      std::string filename;
      fragment_type onBeginFragment;
      fragment_type onEndFragment;
      std::vector<phase_fragment_type> phaseFragments;
};