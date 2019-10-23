#include "types.h"
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'NONE', FormType::None, "None", FormTypeFlags::no_editor_id }, // form types not in this list are effectively 'NONE'
   { 'TES4', FormType::FileHeader, "File Header", FormTypeFlags::no_connections },
   { 'GRUP', FormType::FileRecordGroup, "File Record Group", FormTypeFlags::no_connections },
   { 'GMST', FormType::GameSetting, "GameSetting", FormTypeFlags::no_connections },
   { 'KYWD', FormType::Keyword, "Keyword" },
   { 'LCRT', FormType::LocationRefType, "LocRefType" },
   //
   { 'GLOB', FormType::Global, "Global" },
   //
   { 'FACT', FormType::Faction, "Faction" },
   //
   { 'NPC_', FormType::ActorBase, "ActorBase" },
   //
   { 'CELL', FormType::Cell,  "Cell" },
   { 'REFR', FormType::Reference, "ObjectReference" },
   { 'ACHR', FormType::Character, "Actor" },
   //
   { 'WRLD', FormType::Worldspace, "Worldspace" },
   { 'LAND', FormType::Land, "Landscape", FormTypeFlags::no_editor_id },
   { 'NAVM', FormType::Navmesh, "Navmesh" },
   //
   { 'DIAL', FormType::Topic, "Dialogue Topic" },
   { 'INFO', FormType::TopicInfo, "Dialogue Topic Info" },
   { 'QUST', FormType::Quest, "Quest" },
   //
   { 'VTYP', FormType::Voicetype, "Voicetype" },
   //
   { 'LCTN', FormType::Location, "Location" },
   //
   { 'DLBR', FormType::DialogueBranch, "Dialogue Branch" },
   //
   { 'RELA', FormType::Relationship, "Relationship" },
   { 'SCEN', FormType::Scene, "Scene" },
   { 'ASTP', FormType::AssociationType, "Association Type" },
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