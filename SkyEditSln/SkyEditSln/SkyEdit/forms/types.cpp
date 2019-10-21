#include "types.h"
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'NONE', kFormType_None, "None", FormTypeFlags::no_editor_id },
   { 'TES4', kFormType_FileHeader, "File Header" },
   { 'GRUP', kFormType_FileRecordGroup, "File Record Group" },
   { 'GMST', kFormType_GameSetting, "GameSetting" },
   { 'KYWD', kFormType_Keyword, "Keyword" },
   { 'LCRT', kFormType_LocationRefType, "LocRefType" },
   //
   { 'GLOB', kFormType_Global, "Global" },
   //
   { 'FACT', kFormType_Faction, "Faction" },
   //
   { 'NPC_', kFormType_ActorBase, "ActorBase" },
   //
   { 'CELL', kFormType_Cell,  "Cell" },
   { 'REFR', kFormType_Reference, "ObjectReference" },
   { 'ACHR', kFormType_Character, "Actor" },
   //
   { 'WRLD', kFormType_Worldspace, "Worldspace" },
   { 'LAND', kFormType_Land, "Landscape", FormTypeFlags::no_editor_id },
   { 'NAVM', kFormType_Navmesh, "Navmesh" },
   //
   { 'DIAL', kFormType_Topic, "Dialogue Topic" },
   { 'INFO', kFormType_TopicInfo, "Dialogue Topic Info" },
   { 'QUST', kFormType_Quest, "Quest" },
   //
   { 'VTYP', kFormType_Voicetype, "Voicetype" },
   //
   { 'LCTN', kFormType_Location, "Location" },
   //
   { 'DLBR', kFormType_DialogueBranch, "Dialogue Branch" },
   //
   { 'RELA', kFormType_Relationship, "Relationship" },
   { 'SCEN', kFormType_Scene, "Scene" },
   { 'ASTP', kFormType_AssociationType, "Association Type" },
};

const FormTypeInfo& formTypeFor(formtype_t ft) {
   for (uint8_t i = 0; i < std::extent<decltype(formTypes)>::value; i++) {
      auto& info = formTypes[i];
      if (info.formType == ft)
         return info;
   }
   return formTypes[0];
}
formtype_t signatureToFormType(uint32_t signature) {
   for (uint8_t i = 0; i < std::extent<decltype(formTypes)>::value; i++) {
      auto& info = formTypes[i];
      if (info.signature == signature)
         return info.formType;
   }
   return 0;
}