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
      Action = 0x06, // AACT
      TextureSet = 0x07, // TXST
      MenuIcon = 0x08, // MICN
      Global = 0x09,
      Class = 0x0A,
      Faction = 0x0B,
      HeadPart = 0x0C,
      Eyes = 0x0D,
      Race = 0x0E,
      Sound = 0x0F,
      AcousticSpace = 0x10,
      Skill = 0x11,
      MagicEffect = 0x12,
      Script = 0x13,
      LandTexture = 0x14,
      Enchantment = 0x15,
      Spell = 0x16,
      Scroll = 0x17,
      Activator = 0x18,
      TalkingActivator = 0x19,
      Armor = 0x1A,
      Book = 0x1B,
      Container = 0x1C,
      Door = 0x1D,
      Ingredient = 0x1E,
      Light = 0x1F,
      MiscItem = 0x20,
      Apparatus = 0x21,
      Static = 0x22,
      StaticCollection = 0x23,
      MovableStatic = 0x24,
      Grass = 0x25,
      Tree = 0x26,
      Flora = 0x27,
      Furniture = 0x28,
      Weapon = 0x29,
      Ammo = 0x2A,
      ActorBase = 0x2B, // TESNPC
      LeveledCharacter = 0x2C,
      Key = 0x2D,
      Potion = 0x2E,
      //
      Note = 0x30,
      ConstructibleObject = 0x31,
      Projectile = 0x32,
      Hazard = 0x33,
      SoulGem = 0x34,
      LeveledItem = 0x35,
      Weather = 0x36,
      Climate = 0x37,
      //
      Region = 0x3A,
      //
      Cell = 0x3C,
      Reference = 0x3D,
      Character = 0x3E,
      //
      PlacedHazard = 0x46,
      Worldspace = 0x47,
      Land = 0x48, // heightmapped terrain - TESObjectLAND
      Navmesh = 0x49,
      //
      Topic = 0x4B,
      TopicInfo = 0x4C,
      Quest = 0x4D,
      Idle = 0x4E,
      Package = 0x4F,
      CombatStyle = 0x50,
      LoadingScreen = 0x51,
      LeveledSpell = 0x52,
      //
      WaterType = 0x54,
      EffectShader = 0x55,
      //
      Explosion = 0x57,
      Debris = 0x58,
      ImageSpace = 0x59,
      ImageSpaceModifier = 0x5A,
      FormList = 0x5B,
      Perk = 0x5C,
      BodyPartData = 0x5D,
      //
      Voicetype = 0x62,
      MaterialType = 0x63,
      ImpactData = 0x64,
      ImpactDataSet = 0x65,
      ArmorAddon = 0x66,
      EncounterZone = 0x67,
      Location = 0x68,
      Message = 0x69,
      //
      LightingTemplate = 0x6C,
      MusicType = 0x6D,
      Footstep = 0x6E,
      FootstepSet = 0x6F,
      //
      DialogueBranch = 0x73,
      //
      WordOfPower = 0x76,
      Shout = 0x77,
      EquipSlot = 0x78,
      Relationship = 0x79,
      Scene = 0x7A,
      AssociationType = 0x7B,
      Outfit = 0x7C,
      //
      CollisionLayer = 0x84,
      Color = 0x85,
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

namespace LoadedForms {
   class Form;
}