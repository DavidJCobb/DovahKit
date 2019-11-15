#include "types.h"
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'NONE', FormType::None, "None/Unknown", FormTypeFlags::no_editor_id }, // form types not in this list are effectively 'NONE'
   { 'TES4', FormType::FileHeader, "File Header", FormTypeFlags::no_connections },
   { 'GRUP', FormType::FileRecordGroup, "File Record Group", FormTypeFlags::no_connections },
   { 'GMST', FormType::GameSetting, "GameSetting", FormTypeFlags::no_connections },
   { 'KYWD', FormType::Keyword, "Keyword" },
   { 'LCRT', FormType::LocationRefType, "LocRefType" },
   { 'AACT', FormType::Action, "Action" },
   { 'TXST', FormType::TextureSet, "TextureSet" },
   { 'MICN', FormType::MenuIcon, "Menu Icon" },
   { 'GLOB', FormType::Global, "Global" },
   { 'CLAS', FormType::Class, "Class" },
   { 'FACT', FormType::Faction, "Faction" },
   { 'HDPT', FormType::HeadPart, "HeadPart" },
   // HAIR - TESHair - removed in patch 1.2
   { 'EYES', FormType::Eyes, "Eyes" },
   { 'RACE', FormType::Race, "Race" },
   { 'SOUN', FormType::Sound, "Sound" },
   { 'ASPC', FormType::AcousticSpace, "AcousticSpace" },
   { 'SKIL', FormType::Skill, "Skill" },
   { 'MGEF', FormType::MagicEffect, "MagicEffect" },
   { 'SCPT', FormType::Script, "Script (TES4)" },
   { 'LTEX', FormType::LandTexture, "LandTexture" },
   { 'ENCH', FormType::Enchantment, "Enchantment" },
   { 'SPEL', FormType::Spell, "Spell" },
   { 'SCRL', FormType::Scroll, "Scroll" }, // as in, magic scrolls
   { 'ACTI', FormType::Activator, "Activator" },
   { 'TACT', FormType::TalkingActivator, "Talking Activator" },
   { 'ARMO', FormType::Armor, "Armor" },
   { 'BOOK', FormType::Book, "Book" },
   { 'CONT', FormType::Container, "Container" },
   { 'DOOR', FormType::Door, "Door" },
   { 'INGR', FormType::Ingredient, "Ingredient" },
   { 'LIGH', FormType::Light, "Light" },
   { 'MISC', FormType::MiscItem, "Misc. Item" },
   { 'APPA', FormType::Apparatus, "Apparatus" },
   { 'STAT', FormType::Static, "Static" },
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
   { 'PARW', FormType::PlacedArrowProjectile, "Placed Arrow Projectile" },
   { 'PGRE', FormType::PlacedGrenadeProjectile, "Placed Grenade Projectile" },
   { 'PBEA', FormType::PlacedBeamProjectile, "Placed Beam Projectile" },
   { 'PFLA', FormType::PlacedFlameProjectile, "Placed Flame Projectile" },
   { 'PCON', FormType::PlacedBarrierProjectile, "Placed Barrier Projectile" },
   { 'PBAR', FormType::PlacedConeProjectile, "Placed Cone Projectile" },
   { 'PHZD', FormType::PlacedHazard, "Placed Hazard" },
   { 'WRLD', FormType::Worldspace, "Worldspace" },
   { 'LAND', FormType::Land, "Landscape", FormTypeFlags::no_editor_id },
   { 'NAVM', FormType::Navmesh, "Navmesh" },
   // TLOD - unknown; apparently not found in any file
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
   // TOFT - unknown; apparently not found in any file
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
   { 'MESH', FormType::Message, "Message" },
   { 'RGDL', FormType::Ragdoll, "Ragdoll" },
   { 'DOBJ', FormType::DefaultObjectManager, "Default Objects" },
   { 'LGTM', FormType::LightingTemplate, "Lighting Template" },
   { 'MUSC', FormType::MusicType, "MusicType" },
   { 'FSTP', FormType::Footstep, "Footstep" },
   { 'FSTS', FormType::FootstepSet, "Footstep Set" },
   { 'SMBN', FormType::StoryManagerBranchNode, "Story Manager Branch Node" },
   { 'SMQN', FormType::StoryManagerQuestNode, "Story Manager Quest Node" },
   { 'SMEN', FormType::StoryManagerEventNode, "Story Manager Event Node" },
   { 'DLBR', FormType::DialogueBranch, "Dialogue Branch" },
   { 'MUST', FormType::MusicTrack, "Music Track" },
   { 'DLVW', FormType::DialogueView, "Dialogue View" }, // CK only; game doesn't seem to load it
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
   { 'COLL', FormType::CollisionLayer, "Collision Layer" },
   { 'CLFM', FormType::Color, "Color" },
   { 'REVB', FormType::ReverbParameters, "Reverb Parameters" },
};

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