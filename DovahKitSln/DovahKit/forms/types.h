#pragma once
#include <cstdint>
#include "../helpers/scoped_enum.h"

typedef uint8_t formtype_t;

extern constexpr uint32_t hardcoded_form_id_mask = 0x000007FF; // Mask for form IDs that are hardcoded forms.
extern constexpr uint32_t plugin_form_id_mask    = 0x00FFF800; // Mask for form IDs that are not hardcoded forms.
extern constexpr uint32_t minimum_plugin_form_id = 0x00000800; // Minimum non-load-order-prefixed form ID for a non-hardcoded form.

SCOPE_ENUM(FormType, enum FormType : formtype_t {
   None = 0x00,
   FileHeader = 0x01, 
   FileRecordGroup = 0x02, 
   GameSetting = 0x03, 
   Keyword = 0x04,
   LocationRefType = 0x05,
   Action = 0x06, 
   TextureSet = 0x07, 
   MenuIcon = 0x08, 
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
   ActorBase = 0x2B, 
   LeveledCharacter = 0x2C,
   Key = 0x2D,
   Potion = 0x2E,
   IdleMarker = 0x2F, 
   Note = 0x30,
   ConstructibleObject = 0x31,
   Projectile = 0x32,
   Hazard = 0x33,
   SoulGem = 0x34,
   LeveledItem = 0x35,
   Weather = 0x36,
   Climate = 0x37,
   ShaderParticleGeometry = 0x38,
   VisualEffect = 0x39, SCOPED_ENUM_COMMENT("BGSReferenceEffect")
   Region = 0x3A,
   NavmeshInfo = 0x3B,
   Cell = 0x3C,
   Reference = 0x3D,
   Character = 0x3E, Actor = Character,
   PlacedMissileProjectile = 0x3F,
   PlacedArrowProjectile = 0x40,
   PlacedGrenadeProjectile = 0x41,
   PlacedBeamProjectile = 0x42,
   PlacedFlameProjectile = 0x43,
   PlacedConeProjectile = 0x44,
   PlacedBarrierProjectile = 0x45,
   PlacedHazard = 0x46,
   Worldspace = 0x47,
   Land = 0x48, SCOPED_ENUM_COMMENT("heightmapped terrain; exists as a child of a cell (TESObjectLAND)")
   Navmesh = 0x49,
   SCOPED_ENUM_COMMENT("TLOD: Unknown form type; apparently not found in any file.")
   Topic = 0x4B,
   TopicInfo = 0x4C,
   Quest = 0x4D,
   Idle = 0x4E,
   Package = 0x4F,
   CombatStyle = 0x50,
   LoadingScreen = 0x51,
   LeveledSpell = 0x52,
   AnimationProp = 0x53, SCOPED_ENUM_COMMENT("a.k.a. AnimObject")
   WaterType = 0x54,
   EffectShader = 0x55,
   SCOPED_ENUM_COMMENT("TOFT: Unknown form type; apparently not found in any file.")
   Explosion = 0x57,
   Debris = 0x58,
   ImageSpace = 0x59,
   ImageSpaceModifier = 0x5A,
   FormList = 0x5B,
   Perk = 0x5C,
   BodyPartData = 0x5D,
   AddonNode = 0x5E,
   ActorValueInfo = 0x5F,
   CameraShot = 0x60,
   CameraPath = 0x61,
   Voicetype = 0x62,
   MaterialType = 0x63,
   ImpactData = 0x64,
   ImpactDataSet = 0x65,
   ArmorAddon = 0x66,
   EncounterZone = 0x67,
   Location = 0x68,
   Message = 0x69,
   Ragdoll = 0x6A,
   DefaultObjectManager = 0x6B,
   LightingTemplate = 0x6C,
   MusicType = 0x6D,
   Footstep = 0x6E,
   FootstepSet = 0x6F,
   StoryManagerBranchNode = 0x70,
   StoryManagerQuestNode = 0x71,
   StoryManagerEventNode = 0x72,
   DialogueBranch = 0x73,
   MusicTrack = 0x74,
   DialogueView = 0x75, SCOPED_ENUM_COMMENT("Creation Kit only; the game doesn't load these.")
   WordOfPower = 0x76,
   Shout = 0x77,
   EquipSlot = 0x78,
   Relationship = 0x79,
   Scene = 0x7A,
   AssociationType = 0x7B,
   Outfit = 0x7C,
   ArtObject = 0x7D,
   MaterialObject = 0x7E,
   MovementType = 0x7F,
   SoundDescriptor = 0x80,
   DualCastData = 0x81,
   SoundCategory = 0x82,
   SoundOutputModel = 0x83,
   CollisionLayer = 0x84,
   Color = 0x85,
   ReverbParameters = 0x86,
   SCOPED_ENUM_COMMENT("-----------------------")
   SCOPED_ENUM_COMMENT("0x87: Unknown")
   SCOPED_ENUM_COMMENT("0x88: (Run-Time Only) Alias")
   SCOPED_ENUM_COMMENT("0x89: (Run-Time Only) Reference Alias")
   SCOPED_ENUM_COMMENT("0x8A: (Run-Time Only) Location Alias")
   SCOPED_ENUM_COMMENT("0x8B: (Run-Time Only) ActiveMagicEffect")
   SCOPED_ENUM_COMMENT("-----------------------")
   Max   = 0x87,
   Count = Max + 1,
});

SCOPE_ENUM(FormTypeFlags, enum FormTypeFlags : uint32_t {
   none           = 0, SCOPED_ENUM_COMMENT("Forms of this type cannot have editor IDs.")
   no_editor_id   = 1, SCOPED_ENUM_COMMENT("Forms of this type cannot refer to or be referred to by other forms.")
   no_connections = 2,
});

struct FormTypeInfo {
   uint32_t    signature;
   uint8_t     formType;
   const char* name  = "<unknown>";
   uint32_t    flags = FormTypeFlags::none;
};
struct GroupSequenceList {
   uint32_t* signatures = nullptr;
   uint32_t  count;
   //
   GroupSequenceList();
   ~GroupSequenceList();
   uint32_t operator[](int i) const noexcept;
};

extern FormTypeInfo formTypes[];
extern GroupSequenceList groupSequence;

extern const FormTypeInfo& formTypeFor(formtype_t ft) noexcept;
extern formtype_t signatureToFormType(uint32_t signature) noexcept;
extern bool signatureIsSuspicious(uint32_t signature) noexcept;
//
extern bool formTypeIsReference(formtype_t ft) noexcept;
extern bool signatureIsReference(uint32_t signature) noexcept;

struct form_id_t {
   //
   // A struct to wrap form IDs. This exists so that TESPluginSubrecord::read and similar 
   // functions can be templated on it to normalize and resolve form IDs.
   //
   uint32_t value = 0;
   //
   form_id_t() {};
   form_id_t(uint32_t i) : value(i) {};
   //
   inline operator uint32_t() const noexcept { return this->value; };
   inline form_id_t& operator=(const uint32_t& other) { this->value = other; return *this; };
   inline form_id_t& operator=(const int& other) { this->value = other; return *this; };
   //
   inline bool operator>(const uint32_t& other) { return this->value > other; };
   inline bool operator<(const uint32_t& other) { return this->value < other; };
   inline bool operator>=(const uint32_t& other) { return this->value >= other; };
   inline bool operator<=(const uint32_t& other) { return this->value <= other; };
   inline bool operator==(const uint32_t& other) { return this->value == other; };
   inline bool operator!=(const uint32_t& other) { return this->value != other; };
   //
   inline bool operator>(const form_id_t& other) { return this->value > other.value; };
   inline bool operator<(const form_id_t& other) { return this->value < other.value; };
   inline bool operator>=(const form_id_t& other) { return this->value >= other.value; };
   inline bool operator<=(const form_id_t& other) { return this->value <= other.value; };
   inline bool operator==(const form_id_t& other) { return this->value == other.value; };
   inline bool operator!=(const form_id_t& other) { return this->value != other.value; };
};
struct struct_form_id_t : form_id_t {
   //
   // In some cases, the game reads entire structs from the file by blindly copying bytes. 
   // If these structs contain form IDs, then they will differ in Skyrim Special. Skyrim 
   // treats references from one form to another as unions of form IDs and form pointers; 
   // loading happens in two stages, with the first stage pulling form IDs into the places 
   // where the pointers would be, and the second stage replacing all form IDs with pointers. 
   // Skyrim Special is 64-bit, so its pointers are eight bytes instead of four bytes; as 
   // such, structs that are blindly copied will have their layouts change, with four 
   // padding bytes following each four-byte form ID.
   //
   uint32_t padding = 0;
};

namespace LoadedForms {
   class Form;
}

SCOPE_ENUM(ActorValueIndex, enum ActorValueIndex {
   Aggression,
   Confidence,
   Energy,
   Morality,
   Mood,
   Assistance,
   OneHanded,
   TwoHanded,
   Marksman,
   Block,
   Smithing,
   HeavyArmor,
   LightArmor,
   Pickpocket,
   Lockpicking,
   Sneak,
   Alchemy,
   Speechcraft,
   Alteration,
   Conjuration,
   Destruction = 20,
   Illusion,
   Restoration,
   Enchanting,
   Health,
   Magicka,
   Stamina,
   HealRate,
   MagickaRate,
   StaminaRate,
   SpeedMult,
   InventoryWeight,
   CarryWeight,
   CritChance,
   MeleeDamage,
   UnarmedDamage,
   Mass,
   VoicePoints,
   VoiceRate,
   DamageResist,
   PoisonResist = 40,
   FireResist,
   ElectricResist,
   FrostResist,
   MagicResist,
   DiseaseResist,
   PerceptionCondition,
   EnduranceCondition,
   LeftAttackCondition,
   RightAttackCondition,
   LeftMobilityCondition,
   RightMobilityCondition,
   BrainCondition,
   Paralysis,
   Invisibility,
   NightEye,
   DetectLifeRange,
   WaterBreathing,
   WaterWalking,
   IgnoreCrippledLims,
   Fame = 60,
   Infamy,
   JumpingBonus,
   WardPower,
   RightItemCharge,
   ArmorPerks,
   ShieldPerks,
   WardDeflection,
   Variable01,
   Variable02,
   Variable03,
   Variable04,
   Variable05,
   Variable06,
   Variable07,
   Variable08,
   Variable09,
   Variable10,
   BowSpeedBonuns,
   FavorActive,
   FavorsPerDay = 80,
   FavorsPerDayTimer,
   LeftItemCharge,
   AbsorbChance,
   Blindness,
   WeaponSpeedMult,
   ShoutRecoveryMult,
   BowStaggerBonus,
   Telekinesis,
   FavorPointsBonus,
   LastBribedIntimidated,
   LastFlattered,
   MovementNoiseMult,
   BypassVendorStolenCheck,
   BypassVendorKeywordCheck,
   WaitingForPlayer,
   OneHandedMod,
   TwoHandedMod,
   MarksmanMod,
   BlockMod,
   SmithingMod = 100,
   HeavyArmorMod,
   LightArmorMod,
   PickPocketMod,
   LockpickingMod,
   SneakMod,
   AlchemyMod,
   SpeechcraftMod,
   AlterationMod,
   ConjurationMod,
   DestructionMod,
   IllusionMod,
   RestorationMod,
   EnchantingMod,
   OneHandedSkillAdvance,
   TwoHandedSkillAdvance,
   MarksmanSkillAdvance,
   BlockSkillAdvance,
   SmithingSkillAdvance,
   HeavyArmorSkillAdvance,
   LightArmorSkillAdvance = 120,
   PickPocketSkillAdvance,
   LockpickingSkillAdvance,
   SneakSkillAdvance,
   AlchemySkillAdvance,
   SpeechcraftSkillAdvance,
   AlterationSkillAdvance,
   ConjurationSkillAdvance,
   DestructionSkillAdvance,
   IllusionSkillAdvance,
   RestorationSkillAdvance,
   EnchantingSkillAdvance,
   LeftWeaponSpeedMult,
   DragonSouls,
   CombatHealthRegenMult,
   OneHandedPowerMod,
   TwoHAndedPowerMod,
   MarksmanPowerMod,
   BlockPowerMod,
   SmithingPowerMod,
   HeavyArmorPowerMod = 140,
   LightArmorPowerMod,
   PickPocketPowerMod,
   LockpickingPowerMod,
   SneakPowerMod,
   AlchemyPowerMod,
   SpeechcraftPowerMod,
   AlterationPowerMod,
   ConjurationPowerMod,
   DestructionPowerMod,
   IllusionPowerMod,
   RestorationPowerMod,
   EnchantingPowerMod,
   DragonRend,
   AttackDamageMult,
   HealRateMult,
   MagickaRateMult,
   StaimnaRateMult,
   WerewolfPerks,
   VampirePerks,
   GrabActorOffset = 160,
   Grabbed,
   DEPRECATED05,
   ReflectDamage,
});