#pragma once
#include <cstdint>

typedef uint8_t formtype_t;

enum FormType : formtype_t {
   kFormType_None = 0x00,
   kFormType_FileHeader = 0x01, // TES4
   kFormType_FileRecordGroup = 0x02, // GRUP
   kFormType_GameSetting = 0x03, // GMST
   kFormType_Keyword = 0x04,
   kFormType_LocationRefType = 0x05,
   //
   kFormType_Global = 0x09,
   //
   kFormType_Faction = 0x0B,
   //
   kFormType_ActorBase = 0x2B, // TESNPC
   //
   kFormType_Cell  = 0x3C,
   kFormType_Reference = 0x3D,
   kFormType_Character = 0x3E,
   //
   kFormType_Worldspace = 0x47,
   kFormType_Land = 0x48, // heightmapped terrain - TESObjectLAND
   kFormType_Navmesh = 0x49,
   //
   kFormType_Topic = 0x4B,
   kFormType_TopicInfo = 0x4C,
   kFormType_Quest = 0x4D,
   //
   kFormType_Voicetype = 0x62,
   //
   kFormType_Location = 0x68,
   //
   kFormType_DialogueBranch = 0x73,
   //
   kFormType_Relationship = 0x79,
   kFormType_Scene = 0x7A,
   kFormType_AssociationType = 0x7B,
};
struct FormTypeInfo {
   uint32_t    signature;
   uint8_t     formType;
   const char* name;
};

extern FormTypeInfo formTypes[];

extern formtype_t signatureToFormType(uint32_t signature);

class TESForm {
   public:
      const formtype_t formType;
      TESForm(formtype_t ft) : formType(ft) {};
};