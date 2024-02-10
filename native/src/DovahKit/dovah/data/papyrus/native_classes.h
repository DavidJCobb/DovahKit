#pragma once
#include <array>
#include <string_view>
#include "../../form_types.h"

namespace dovah::papyrus {
   struct native_class {
      dovah::form_type_t form_type;
      std::string_view   name;
   };

   constexpr const std::array<native_class, 71> native_classes = {{
      { dovah::form_type::alias,                "Alias" },
      { dovah::form_type::reference_alias,      "ReferenceAlias" },
      { dovah::form_type::location_alias,       "LocationAlias" },

      { dovah::form_type::active_magic_effect,  "ActiveMagicEffect" },

      { dovah::form_type::none,                 "Form" },
      
      { dovah::form_type::action,               "Action" },
      { dovah::form_type::activator,            "Activator" },
      { dovah::form_type::actor,                "Actor" },
      { dovah::form_type::actor_base,           "ActorBase" },
      { dovah::form_type::ammo,                 "Ammo" },
      { dovah::form_type::apparatus,            "Apparatus" },
      { dovah::form_type::armor,                "Armor" },
      { dovah::form_type::association_type,     "AssociationType" },
      { dovah::form_type::book,                 "Book" },
      { dovah::form_type::cell,                 "Cell" },
      { dovah::form_type::combat_class,         "Class" },
      { dovah::form_type::constructible_object, "ConstructibleObject" },
      { dovah::form_type::container,            "Container" },
      { dovah::form_type::door,                 "Door" },
      { dovah::form_type::effect_shader,        "EffectShader" },
      { dovah::form_type::enchantment,          "Enchantment" },
      { dovah::form_type::encounter_zone,       "EncounterZone" },
      { dovah::form_type::explosion,            "Explosion" },
      { dovah::form_type::faction,              "Faction" },
      { dovah::form_type::flora,                "Flora" },
      { dovah::form_type::formlist,             "FormList" },
      { dovah::form_type::furniture,            "Furniture" },
      { dovah::form_type::global,               "GlobalVariable" },
      { dovah::form_type::hazard,               "Hazard" },
      { dovah::form_type::idle,                 "Idle" },
      { dovah::form_type::imagespace_modifier,  "ImageSpaceModifier" },
      { dovah::form_type::impact_data_set,      "ImpactDataSet" },
      { dovah::form_type::ingredient,           "Ingredient" },
      { dovah::form_type::key,                  "Key" },
      { dovah::form_type::keyword,              "Keyword" },
      { dovah::form_type::location_ref_type,    "LocationRefType" },
      { dovah::form_type::leveled_character,    "LeveledActor" },
      { dovah::form_type::leveled_item,         "LeveledItem" },
      { dovah::form_type::leveled_spell,        "LeveledSpell" },
      { dovah::form_type::light,                "Light" },
      { dovah::form_type::location,             "Location" },
      { dovah::form_type::magic_effect,         "MagicEffect" },
      { dovah::form_type::message,              "Message" },
      { dovah::form_type::misc_item,            "MiscObject" },
      { dovah::form_type::music_type,           "MusicType" },
      { dovah::form_type::reference,            "ObjectReference" },
      { dovah::form_type::outfit,               "Outfit" },
      { dovah::form_type::package,              "Package" },
      { dovah::form_type::perk,                 "Perk" },
      { dovah::form_type::potion,               "Potion" },
      { dovah::form_type::projectile,           "Projectile" },
      { dovah::form_type::quest,                "Quest" },
      { dovah::form_type::race,                 "Race" },
      { dovah::form_type::scene,                "Scene" },
      { dovah::form_type::scroll,               "Scroll" },
      { dovah::form_type::shout,                "Shout" },
      { dovah::form_type::sound,                "Sound" },
      { dovah::form_type::sound_category,       "SoundCategory" },
      { dovah::form_type::soul_gem,             "SoulGem" },
      { dovah::form_type::spell,                "Spell" },
      { dovah::form_type::statik,               "Static" },
      { dovah::form_type::talking_activator,    "TalkingActivator" },
      { dovah::form_type::texture_set,          "TextureSet" },
      { dovah::form_type::topic,                "Topic" },
      { dovah::form_type::topic_info,           "TopicInfo" },
      { dovah::form_type::reference_effect,     "VisualEffect" },
      { dovah::form_type::voicetype,            "VoiceType" },
      { dovah::form_type::weapon,               "Weapon" },
      { dovah::form_type::weather,              "Weather" },
      { dovah::form_type::word_of_power,        "WordOfPower" },
      { dovah::form_type::worldspace,           "Worldspace" },
   }};
}