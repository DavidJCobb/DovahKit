#include "types.h"
#include <string>
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'NONE', FormType::None, "None/Unknown", FormTypeFlags::no_editor_id }, // form types not in this list are effectively 'NONE'
   { 'TES4', FormType::FileHeader, "File Header", FormTypeFlags::no_connections },
   { 'GRUP', FormType::FileRecordGroup, "File Record Group", FormTypeFlags::no_connections },
   { 'GMST', FormType::GameSetting, "GameSetting", FormTypeFlags::no_connections }, // Skyrim's loader handles this as a special case; there is no factory for this form type.
   { 'KYWD', FormType::Keyword, "Keyword" },
   { 'LCRT', FormType::LocationRefType, "LocRefType" },
   { 'AACT', FormType::Action, "Action" },
   { 'TXST', FormType::TextureSet, "TextureSet" },
   { 'MICN', FormType::MenuIcon, "Menu Icon" }, // BGSMenuIcon
   { 'GLOB', FormType::Global,   "Global" },
   { 'CLAS', FormType::Class,    "Class" },
   { 'FACT', FormType::Faction,  "Faction" },
   { 'HDPT', FormType::HeadPart, "HeadPart" },
   // HAIR - TESHair - removed in patch 1.2
   { 'EYES', FormType::Eyes,     "Eyes" },
   { 'RACE', FormType::Race,     "Race" },
   { 'SOUN', FormType::Sound,    "Sound" },
   { 'ASPC', FormType::AcousticSpace, "AcousticSpace" },
   { 'SKIL', FormType::Skill,       "Skill" },
   { 'MGEF', FormType::MagicEffect, "MagicEffect" },
   { 'SCPT', FormType::Script,      "Script (TES4)" },
   { 'LTEX', FormType::LandTexture, "LandTexture" },
   { 'ENCH', FormType::Enchantment, "Enchantment" },
   { 'SPEL', FormType::Spell,       "Spell" },
   { 'SCRL', FormType::Scroll,      "Scroll" }, // as in, magic scrolls
   { 'ACTI', FormType::Activator,   "Activator" },
   { 'TACT', FormType::TalkingActivator, "Talking Activator" },
   { 'ARMO', FormType::Armor,       "Armor" },
   { 'BOOK', FormType::Book,        "Book" },
   { 'CONT', FormType::Container,   "Container" },
   { 'DOOR', FormType::Door,        "Door" },
   { 'INGR', FormType::Ingredient,  "Ingredient" },
   { 'LIGH', FormType::Light,       "Light" },
   { 'MISC', FormType::MiscItem,    "Misc. Item" },
   { 'APPA', FormType::Apparatus,   "Apparatus" },
   { 'STAT', FormType::Static,      "Static" },
   { 'SCOL', FormType::StaticCollection, "Static Collection" },
   { 'MSTT', FormType::MovableStatic, "MovableStatic" },
   { 'GRAS', FormType::Grass, "Grass" },
   { 'TREE', FormType::Tree, "Tree" },
   // CLDC - BGSCloudCluster - removed in patch 1.2
   { 'FLOR', FormType::Flora, "Flora" },
   { 'FURN', FormType::Furniture, "Furniture" },
   { 'WEAP', FormType::Weapon, "Weapon" },
   { 'AMMO', FormType::Ammo, "Ammo" },
   { 'NPC_', FormType::ActorBase, "ActorBase" },
   { 'LVLN', FormType::LeveledCharacter, "Leveled Actor" },
   { 'KEYM', FormType::Key, "Key" },
   { 'ALCH', FormType::Potion, "Potion" },
   { 'IDLM', FormType::IdleMarker, "Idle Marker" },
   { 'NOTE', FormType::Note, "Note" },
   { 'COBJ', FormType::ConstructibleObject, "Constructible Object" },
   { 'PROJ', FormType::Projectile, "Projectile" },
   { 'HAZD', FormType::Hazard, "Hazard" },
   { 'SLGM', FormType::SoulGem, "Soul Gem" },
   { 'LVLI', FormType::LeveledItem, "Leveled Item" },
   { 'WTHR', FormType::Weather, "Weather" },
   { 'CLMT', FormType::Climate, "Climate" },
   { 'SPGD', FormType::ShaderParticleGeometry, "Shader Particle Geometry" },
   { 'RFCT', FormType::VisualEffect, "VisualEffect" }, // "ReferenceEffect," internally
   { 'REGN', FormType::Region, "Region" },
   { 'NAVI', FormType::NavmeshInfo,  "Navmesh Info Map" },
   { 'CELL', FormType::Cell,  "Cell" },
   { 'REFR', FormType::Reference, "ObjectReference" },
   { 'ACHR', FormType::Character, "Actor" },
   { 'PMIS', FormType::PlacedMissileProjectile, "Placed Missile Projectile" },
   { 'PARW', FormType::PlacedArrowProjectile,   "Placed Arrow Projectile" },
   { 'PGRE', FormType::PlacedGrenadeProjectile, "Placed Grenade Projectile" },
   { 'PBEA', FormType::PlacedBeamProjectile,    "Placed Beam Projectile" },
   { 'PFLA', FormType::PlacedFlameProjectile,   "Placed Flame Projectile" },
   { 'PCON', FormType::PlacedBarrierProjectile, "Placed Barrier Projectile" },
   { 'PBAR', FormType::PlacedConeProjectile,    "Placed Cone Projectile" },
   { 'PHZD', FormType::PlacedHazard,            "Placed Hazard" },
   { 'WRLD', FormType::Worldspace, "Worldspace" },
   { 'LAND', FormType::Land, "Landscape", FormTypeFlags::no_editor_id },
   { 'NAVM', FormType::Navmesh, "Navmesh" },
   // TLOD: Unknown. Not found in any file, and not loaded by the game (the form factory table has a null entry for this form type).
   { 'DIAL', FormType::Topic, "Dialogue Topic" },
   { 'INFO', FormType::TopicInfo, "Dialogue Topic Info" },
   { 'QUST', FormType::Quest, "Quest" },
   { 'IDLE', FormType::Idle, "Idle" },
   { 'PACK', FormType::Package, "Package" },
   { 'CSTY', FormType::CombatStyle, "Combat Style" },
   { 'LSCR', FormType::LoadingScreen, "Loading Screen" },
   { 'LVSP', FormType::LeveledSpell, "Leveled Spell" },
   { 'ANIO', FormType::AnimationProp, "Animation Prop" }, // a.k.a. AnimObject
   { 'WATR', FormType::WaterType, "Water Type" },
   { 'EFSH', FormType::EffectShader, "EffectShader" },
   // TOFT: Unknown. Not found in any file, and not loaded by the game (the form factory table has a null entry for this form type).
   { 'EXPL', FormType::Explosion, "Explosion" },
   { 'DEBR', FormType::Debris, "Debris" },
   { 'IMGS', FormType::ImageSpace, "ImageSpace" },
   { 'IMAD', FormType::ImageSpaceModifier, "ImageSpace Modifier" },
   { 'FLST', FormType::FormList, "FormList" },
   { 'PERK', FormType::Perk, "Perk" },
   { 'BPTD', FormType::BodyPartData, "Body Part Data" },
   { 'ADDN', FormType::AddonNode, "Add-on Node" },
   { 'AVIF', FormType::ActorValueInfo, "ActorValue Info" },
   { 'CAMS', FormType::CameraShot, "Camera Shot" },
   { 'CPTH', FormType::CameraPath, "Camera Path" },
   { 'VTYP', FormType::Voicetype, "Voicetype" },
   { 'MATT', FormType::MaterialType, "Material Type" },
   { 'IPCT', FormType::ImpactData, "Impact Data" },
   { 'IPDS', FormType::ImpactDataSet, "Impact Data Set" },
   { 'ARMA', FormType::ArmorAddon, "Armor Addon" },
   { 'ECZN', FormType::EncounterZone, "Encounter Zone" },
   { 'LCTN', FormType::Location, "Location" },
   { 'MESG', FormType::Message, "Message" },
   { 'RGDL', FormType::Ragdoll, "Ragdoll" }, // BGSRagdoll
   { 'DOBJ', FormType::DefaultObjectManager, "Default Objects" }, // Skyrim's loader handles this as a special case; there is no factory for this form type.
   { 'LGTM', FormType::LightingTemplate, "Lighting Template" },
   { 'MUSC', FormType::MusicType, "MusicType" },
   { 'FSTP', FormType::Footstep, "Footstep" },
   { 'FSTS', FormType::FootstepSet, "Footstep Set" },
   { 'SMBN', FormType::StoryManagerBranchNode, "Story Manager Branch Node" },
   { 'SMQN', FormType::StoryManagerQuestNode, "Story Manager Quest Node" },
   { 'SMEN', FormType::StoryManagerEventNode, "Story Manager Event Node" },
   { 'DLBR', FormType::DialogueBranch, "Dialogue Branch" },
   { 'MUST', FormType::MusicTrack, "Music Track" },
   { 'DLVW', FormType::DialogueView, "Dialogue View" }, // CK only; not loaded by the game (the form factory table has a null entry for this form type).
   { 'WOOP', FormType::WordOfPower, "Word Of Power" },
   { 'SHOU', FormType::Shout, "Shout" },
   { 'EQUP', FormType::EquipSlot, "Equip Slot" },
   { 'RELA', FormType::Relationship, "Relationship" },
   { 'SCEN', FormType::Scene, "Scene" },
   { 'ASTP', FormType::AssociationType, "Association Type" },
   { 'OTFT', FormType::Outfit, "Outfit" },
   { 'ARTO', FormType::ArtObject, "Art Object" },
   { 'MATO', FormType::MaterialObject, "Material Object" },
   { 'MOVT', FormType::MovementType, "Movement Type" },
   { 'SNDR', FormType::SoundDescriptor, "Sound Descriptor" },
   { 'DUAL', FormType::DualCastData, "Dual-Cast Data" },
   { 'SNCT', FormType::SoundCategory, "Sound Category" },
   { 'SOPM', FormType::SoundOutputModel, "Sound Output Model" },
   { 'COLL', FormType::CollisionLayer, "Collision Layer" }, // BGSCollisionLayer
   { 'CLFM', FormType::Color, "Color" },
   { 'REVB', FormType::ReverbParameters, "Reverb Parameters" },
};

GroupSequenceList::GroupSequenceList() {
   uint32_t list[] = {
      'GMST',
      'KYWD',
      'LCRT',
      'AACT',
      'TXST',
      'GLOB',
      'CLAS',
      'FACT',
      'HDPT',
      'HAIR',
      'EYES',
      'RACE',
      'SOUN',
      'ASPC',
      'MGEF',
      'SCPT',
      'LTEX',
      'ENCH',
      'SPEL',
      'SCRL',
      'ACTI',
      'TACT',
      'ARMO',
      'BOOK',
      'CONT',
      'DOOR',
      'INGR',
      'LIGH',
      'MISC',
      'APPA',
      'STAT',
      'SCOL',
      'MSTT',
      'PWAT',
      'GRAS',
      'TREE',
      'CLDC',
      'FLOR',
      'FURN',
      'WEAP',
      'AMMO',
      'NPC_',
      'PLYR',
      'LVLN',
      'KEYM',
      'ALCH',
      'IDLM',
      'COBJ',
      'PROJ',
      'HAZD',
      'SLGM',
      'LVLI',
      'WTHR',
      'CLMT',
      'SPGD',
      'RFCT',
      'REGN',
      'NAVI',
      'CELL',
      'WRLD',
      'DIAL',
      'QUST',
      'IDLE',
      'PACK',
      'CSTY',
      'LSCR',
      'LVSP',
      'ANIO',
      'WATR',
      'EFSH',
      'EXPL',
      'DEBR',
      'IMGS',
      'IMAD',
      'FLST',
      'PERK',
      'BPTD',
      'ADDN',
      'AVIF',
      'CAMS',
      'CPTH',
      'VTYP',
      'MATT',
      'IPCT',
      'IPDS',
      'ARMA',
      'ECZN',
      'LCTN',
      'MESG',
      'RGDL',
      'DOBJ',
      'LGTM',
      'MUSC',
      'FSTP',
      'FSTS',
      'SMBN',
      'SMQN',
      'SMEN',
      'DLBR',
      'MUST',
      'DLVW',
      'WOOP',
      'SHOU',
      'EQUP',
      'RELA',
      'SCEN',
      'ASTP',
      'OTFT',
      'ARTO',
      'MATO',
      'VOLI', // SSE
      'MOVT',
      'SNDR',
      'DUAL',
      'SNCT',
      'SOPM',
      'COLL',
      'CLFM',
      'REVB',
      'LENS', // SSE
   };
   this->signatures = (uint32_t*)malloc(sizeof(list));
   memcpy(this->signatures, &list, sizeof(list));
   this->count      = std::extent<decltype(list)>::value;
}
GroupSequenceList::~GroupSequenceList() {
   if (this->signatures)
      free(this->signatures);
}
GroupSequenceList groupSequence;

const FormTypeInfo& formTypeFor(formtype_t ft) noexcept {
   for (uint8_t i = 0; i < std::extent<decltype(formTypes)>::value; i++) {
      auto& info = formTypes[i];
      if (info.formType == ft)
         return info;
   }
   return formTypes[0];
}
formtype_t signatureToFormType(uint32_t signature) noexcept {
   for (uint8_t i = 0; i < std::extent<decltype(formTypes)>::value; i++) {
      auto& info = formTypes[i];
      if (info.signature == signature)
         return info.formType;
   }
   return 0;
}
bool formTypeIsReference(formtype_t ft) noexcept {
   switch (ft) {
      case FormType::Character:
      case FormType::Reference:
      case FormType::PlacedArrowProjectile:
      case FormType::PlacedBarrierProjectile:
      case FormType::PlacedBeamProjectile:
      case FormType::PlacedConeProjectile:
      case FormType::PlacedFlameProjectile:
      case FormType::PlacedGrenadeProjectile:
      case FormType::PlacedHazard:
      case FormType::PlacedMissileProjectile:
         return true;
   }
   return false;
}
bool signatureIsReference(uint32_t signature) noexcept {
   switch (signature) {
      case 'ACHR':
      case 'REFR':
      case 'PMIS':
      case 'PARW':
      case 'PGRE':
      case 'PBEA':
      case 'PFLA':
      case 'PCON':
      case 'PBAR':
      case 'PHZD':
         return true;
   }
   return false;
}

namespace {
   inline bool _isNumber(char c) { return c >= '0' && c <= '9'; }
   inline bool _isLetter(char c) { return c >= 'A' && c <= 'Z'; }
}
bool signatureIsSuspicious(uint32_t signature) noexcept {
   unsigned char c;
   c = signature & 0xFF;
   if (!_isNumber(c) && !_isLetter(c) && c != '_')
      return true;
   c = (signature >> 0x08) & 0xFF;
   if (!_isNumber(c) && !_isLetter(c) && c != '_')
      return true;
   c = (signature >> 0x10) & 0xFF;
   if (!_isNumber(c) && !_isLetter(c) && c != '_')
      return true;
   c = (signature >> 0x18) & 0xFF;
   if (!_isNumber(c) && !_isLetter(c) && c != '_')
      return true;
   return false;
}