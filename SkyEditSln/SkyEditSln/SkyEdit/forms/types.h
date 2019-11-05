#pragma once
#include <cstdint>

typedef uint8_t formtype_t;

struct FormType { // enum; a struct-wrapped enum is scoped like enum class but allows implicit casts to number types
   FormType() = delete;
   enum : formtype_t {
      None = 0x00,
      FileHeader = 0x01, // TES4
      FileRecordGroup = 0x02, // GRUP
      GameSetting = 0x03, // GMST
      Keyword = 0x04,
      LocationRefType = 0x05,
      //
      Global = 0x09,
      //
      Faction = 0x0B,
      //
      ActorBase = 0x2B, // TESNPC
      //
      Cell = 0x3C,
      Reference = 0x3D,
      Character = 0x3E,
      //
      Worldspace = 0x47,
      Land = 0x48, // heightmapped terrain - TESObjectLAND
      Navmesh = 0x49,
      //
      Topic = 0x4B,
      TopicInfo = 0x4C,
      Quest = 0x4D,
      //
      Voicetype = 0x62,
      //
      Location = 0x68,
      //
      DialogueBranch = 0x73,
      //
      Relationship = 0x79,
      Scene = 0x7A,
      AssociationType = 0x7B,
      //
      Max   = 0x8B,
      Count = Max + 1,
   };
};
struct FormTypeFlags { // enum; a struct-wrapped enum is scoped like enum class but allows implicit casts to number types
   FormTypeFlags() = delete;
   enum : uint32_t {
      none = 0,
      //
      // (no_editor_id)
      // Forms of this type cannot have editor IDs.
      //
      no_editor_id = 1,
      //
      // (no_connections)
      // Forms of this type cannot refer to or be referred to by other forms.
      //
      no_connections = 2,
   };
};
struct FormTypeInfo {
   uint32_t    signature;
   uint8_t     formType;
   const char* name;
   uint32_t    flags = FormTypeFlags::none;
};

extern FormTypeInfo formTypes[];

extern const FormTypeInfo& formTypeFor(formtype_t ft);
extern formtype_t signatureToFormType(uint32_t signature);

class TESForm {
   public:
      const formtype_t formType;
      TESForm(formtype_t ft) : formType(ft) {};
};