#include "types.h"
#include <type_traits>

FormTypeInfo formTypes[] = {
   { 'NONE', FormType::None, "None", FormTypeFlags::no_editor_id }, // form types not in this list are effectively 'NONE'
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
   //
   { 'NOTE', FormType::Note, "Note" },
   { 'COBJ', FormType::ConstructibleObject, "Constructible Object" },
   { 'PROJ', FormType::Projectile, "Projectile" },
   { 'HAZD', FormType::Hazard, "Hazard" },
   { 'SLGM', FormType::SoulGem, "Soul Gem" },
   { 'LVLI', FormType::LeveledItem, "Leveled Item" },
   { 'WTHR', FormType::Weather, "Weather" },
   { 'CLMT', FormType::Climate, "Climate" },
   //
   { 'REGN', FormType::Region, "Region" },
   //
   { 'CELL', FormType::Cell,  "Cell" },
   { 'REFR', FormType::Reference, "ObjectReference" },
   { 'ACHR', FormType::Character, "Actor" },
   //
   { 'PHZD', FormType::PlacedHazard, "Hazard Reference" },
   { 'WRLD', FormType::Worldspace, "Worldspace" },
   { 'LAND', FormType::Land, "Landscape", FormTypeFlags::no_editor_id },
   { 'NAVM', FormType::Navmesh, "Navmesh" },
   //
   { 'DIAL', FormType::Topic, "Dialogue Topic" },
   { 'INFO', FormType::TopicInfo, "Dialogue Topic Info" },
   { 'QUST', FormType::Quest, "Quest" },
   { 'IDLE', FormType::Idle, "Idle" },
   { 'PACK', FormType::Package, "Package" },
   { 'CSTY', FormType::CombatStyle, "Combat Style" },
   { 'LSCR', FormType::LoadingScreen, "Loading Screen" },
   { 'LVSP', FormType::LeveledSpell, "Leveled Spell" },
   //
   { 'WATR', FormType::WaterType, "Water Type" },
   { 'EFSH', FormType::EffectShader, "EffectShader" },
   //
   { 'EXPL', FormType::Explosion, "Explosion" },
   { 'DEBR', FormType::Debris, "Debris" },
   { 'IMGS', FormType::ImageSpace, "ImageSpace" },
   { 'IMAD', FormType::ImageSpaceModifier, "ImageSpace Modifier" },
   { 'FLST', FormType::FormList, "FormList" },
   { 'PERK', FormType::Perk, "Perk" },
   { 'BPTD', FormType::BodyPartData, "Body Part Data" },
   //
   { 'VTYP', FormType::Voicetype, "Voicetype" },
   { 'MATT', FormType::MaterialType, "Material Type" },
   { 'IPCT', FormType::ImpactData, "Impact Data" },
   { 'IPDS', FormType::ImpactDataSet, "Impact Data Set" },
   { 'ARMA', FormType::ArmorAddon, "Armor Addon" },
   { 'ECZN', FormType::EncounterZone, "Encounter Zone" },
   { 'LCTN', FormType::Location, "Location" },
   { 'MESH', FormType::Message, "Message" },
   //
   { 'LGTM', FormType::LightingTemplate, "Lighting Template" },
   { 'MUSC', FormType::MusicType, "MusicType" },
   { 'FSTP', FormType::Footstep, "Footstep" },
   { 'FSTS', FormType::FootstepSet, "Footstep Set" },
   //
   { 'DLBR', FormType::DialogueBranch, "Dialogue Branch" },
   //
   { 'WOOP', FormType::WordOfPower, "Word Of Power" },
   { 'SHOU', FormType::Shout, "Shout" },
   { 'EQUP', FormType::EquipSlot, "Equip Slot" },
   { 'RELA', FormType::Relationship, "Relationship" },
   { 'SCEN', FormType::Scene, "Scene" },
   { 'ASTP', FormType::AssociationType, "Association Type" },
   { 'OTFT', FormType::Outfit, "Outfit" },
   //
   { 'COLL', FormType::CollisionLayer, "Collision Layer" },
   { 'CLFM', FormType::Color, "Color" },
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