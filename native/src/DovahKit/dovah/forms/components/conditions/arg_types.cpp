#include "arg_types.h"
#include "../../../helpers/strings.h"

namespace dovah::loaded_forms::components {
   namespace condition_info {
      bool arg_type::load_value(condition_arg_value& out, tes_subrecord_reader& subrecord) const noexcept {
         switch (this->underlying) {
            case arg_underlying_type::formID:
               return subrecord.read(out.form);
            default:
               return subrecord.read(out.dword);
         }
      }
      void arg_type::to_string(const condition_arg_value& value, std::string& out) const noexcept {
         assert(!this->isUnion && "This should never be called directly on a type that has been flagged as a union; use resolveType to get the effective type instead.");
         if (this->isEnum) {
            for (auto& ev : this->enumValues) {
               if (value.dword == ev.value) {
                  out = ev.string;
                  return;
               }
            }
            cobb::sprintf(out, "Invalid %d", value.dword);
            return;
         }
         switch (this->underlying) {
            case arg_underlying_type::formID:
               cobb::sprintf(out, "[FORM:%08X]", value.form.formID());
               return;
            case arg_underlying_type::float32:
               cobb::sprintf(out, "%f", value.float32);
               return;
            case arg_underlying_type::character:
               out = (unsigned char)value.dword;
               return;
            case arg_underlying_type::int_signed:
               cobb::sprintf(out, "%d", value.dword);
               return;
            case arg_underlying_type::int_unsigned:
            case arg_underlying_type::quest_stage:
               cobb::sprintf(out, "%u", value.dword);
               return;
            case arg_underlying_type::string:
               out = value.string;
               return;
         }
         cobb::sprintf(out, "DWORD 0x%08X", value.dword); // should only happen if there's something we haven't finished yet
      }
      bool arg_type::is_valid_for_enum(const condition_arg_value& value) const noexcept {
         assert(!this->isUnion && "This should never be called directly on a type that has been flagged as a union; use resolveType to get the effective type instead.");
         if (this->isEnum)
            for (auto& ev : this->enumValues)
               if (ev.value == value.dword)
                  return true;
         return false;
      }
      void arg_type::get_enum_values(std::vector<condition_arg_value>& out) const noexcept {
         assert(!this->isUnion && "This should never be called directly on a type that has been flagged as a union; use resolveType to get the effective type instead.");
         if (!this->isEnum) {
            out.clear();
            return;
         }
         auto count = this->enumValues.size();
         out.resize(count);
         for (uint32_t i = 0; i < count; i++)
            out[i].dword = this->enumValues[i].value;
      }
      arg_type* arg_type::resolve_union(arg_type* previousType, const condition_arg_value* previousValue) const noexcept {
         return nullptr;
      }

      namespace arg_types {
         arg_type      None            = arg_type("None", arg_underlying_type::none);
         arg_form_type Actor           = arg_form_type("Actor", { form_type::actor }, true);
         arg_form_type ActorBase       = arg_form_type("actor_base", { form_type::actor_base });
         arg_enum_type ActorValue      = arg_enum_type("ActorValue", 0, {
            "Aggression",
            "Confidence",
            "Energy",
            "Morality",
            "Mood",
            "Assistance",
            "OneHanded",
            "TwoHanded",
            "Marksman",
            "Block",
            "Smithing",
            "HeavyArmor",
            "LightArmor",
            "Pickpocket",
            "Lockpicking",
            "Sneak",
            "Alchemy",
            "Speechcraft",
            "Alteration",
            "Conjuration",
            "Destruction",
            "Illusion",
            "Restoration",
            "Enchanting",
            "Health",
            "Magicka",
            "Stamina",
            "HealRate",
            "MagickaRate",
            "StaminaRate",
            "SpeedMult",
            "InventoryWeight",
            "CarryWeight",
            "CritChance",
            "MeleeDamage",
            "UnarmedDamage",
            "Mass",
            "VoicePoints",
            "VoiceRate",
            "DamageResist",
            "PoisonResist",
            "FireResist",
            "ElectricResist",
            "FrostResist",
            "MagicResist",
            "DiseaseResist",
            "PerceptionCondition",
            "EnduranceCondition",
            "LeftAttackCondition",
            "RightAttackCondition",
            "LeftMobilityCondition",
            "RightMobilityCondition",
            "BrainCondition",
            "Paralysis",
            "Invisibility",
            "NightEye",
            "DetectLifeRange",
            "WaterBreathing",
            "WaterWalking",
            "IgnoreCrippledLims",
            "Fame",
            "Infamy",
            "JumpingBonus",
            "WardPower",
            "RightItemCharge",
            "ArmorPerks",
            "ShieldPerks",
            "WardDeflection",
            "Variable01",
            "Variable02",
            "Variable03",
            "Variable04",
            "Variable05",
            "Variable06",
            "Variable07",
            "Variable08",
            "Variable09",
            "Variable10",
            "BowSpeedBonuns",
            "FavorActive",
            "FavorsPerDay",
            "FavorsPerDayTimer",
            "LeftItemCharge",
            "AbsorbChance",
            "Blindness",
            "WeaponSpeedMult",
            "ShoutRecoveryMult",
            "BowStaggerBonus",
            "Telekinesis",
            "FavorPointsBonus",
            "LastBribedIntimidated",
            "LastFlattered",
            "MovementNoiseMult",
            "BypassVendorStolenCheck",
            "BypassVendorKeywordCheck",
            "WaitingForPlayer",
            "OneHandedMod",
            "TwoHandedMod",
            "MarksmanMod",
            "BlockMod",
            "SmithingMod",
            "HeavyArmorMod",
            "LightArmorMod",
            "PickPocketMod",
            "LockpickingMod",
            "SneakMod",
            "AlchemyMod",
            "SpeechcraftMod",
            "AlterationMod",
            "ConjurationMod",
            "DestructionMod",
            "IllusionMod",
            "RestorationMod",
            "EnchantingMod",
            "OneHandedSkillAdvance",
            "TwoHandedSkillAdvance",
            "MarksmanSkillAdvance",
            "BlockSkillAdvance",
            "SmithingSkillAdvance",
            "HeavyArmorSkillAdvance",
            "LightArmorSkillAdvance",
            "PickPocketSkillAdvance",
            "LockpickingSkillAdvance",
            "SneakSkillAdvance",
            "AlchemySkillAdvance",
            "SpeechcraftSkillAdvance",
            "AlterationSkillAdvance",
            "ConjurationSkillAdvance",
            "DestructionSkillAdvance",
            "IllusionSkillAdvance",
            "RestorationSkillAdvance",
            "EnchantingSkillAdvance",
            "LeftWeaponSpeedMult",
            "DragonSouls",
            "CombatHealthRegenMult",
            "OneHandedPowerMod",
            "TwoHAndedPowerMod",
            "MarksmanPowerMod",
            "BlockPowerMod",
            "SmithingPowerMod",
            "HeavyArmorPowerMod",
            "LightArmorPowerMod",
            "PickPocketPowerMod",
            "LockpickingPowerMod",
            "SneakPowerMod",
            "AlchemyPowerMod",
            "SpeechcraftPowerMod",
            "AlterationPowerMod",
            "ConjurationPowerMod",
            "DestructionPowerMod",
            "IllusionPowerMod",
            "RestorationPowerMod",
            "EnchantingPowerMod",
            "DragonRend",
            "AttackDamageMult",
            "HealRateMult",
            "MagickaRateMult",
            "StaimnaRateMult",
            "WerewolfPerks",
            "VampirePerks",
            "GrabActorOffset",
            "Grabbed",
            "DEPRECATED05",
            "ReflectDamage",
         });
         arg_type      AdvanceAction   = arg_type("Advance Action", arg_underlying_type::int_signed, {
            enum_value_definition(0, "Normal Usage"),
            enum_value_definition(1, "Power Attack"),
            enum_value_definition(2, "Bash"),
            enum_value_definition(3, "Lockpick Success"),
            enum_value_definition(4, "Lockpick Broken"),
         });
         arg_type      Alias           = arg_type("Alias", arg_underlying_type::aliasID);
         arg_type      Alignment       = arg_type("Alignment", arg_underlying_type::int_signed, {
            enum_value_definition(0, "Good"),
            enum_value_definition(1, "Neutral"),
            enum_value_definition(2, "Evil"),
            enum_value_definition(3, "Very Good"),
            enum_value_definition(4, "Very Evil"),
         });
         arg_form_type AssociationType = arg_form_type("Association Type", { form_type::association_type });
         arg_type      Axis            = arg_type("Axis", arg_underlying_type::character, {
            enum_value_definition('X', "X"),
            enum_value_definition('Y', "Y"),
            enum_value_definition('Z', "Z"),
         });
         arg_form_type BaseForm        = arg_form_type("Base Form", {
            form_type::acoustic_space, // Confirmed in CK. Strange, since these aren't placeable.
            form_type::activator,
            form_type::actor_base,
            form_type::container,
            form_type::door,
            form_type::flora,
            form_type::furniture,
            form_type::grass,
            form_type::hazard,
            form_type::idle_marker,
            form_type::light,
            form_type::movable_static,
            form_type::projectile,
            form_type::sound,
            form_type::statik,
            form_type::talking_activator,
            form_type::tree,
            // Items:
            form_type::ammo,
            form_type::armor,
            form_type::armor_addon,
            form_type::book,
            form_type::key,
            form_type::leveled_item,
            form_type::misc_item,
            form_type::potion,
            form_type::scroll,
            form_type::soul_gem,
            form_type::weapon,
            // Magic:
            form_type::enchantment,
            form_type::leveled_spell,
            form_type::shout,
            form_type::spell,
            // Other:
            form_type::formlist,
         });
         arg_type      CastingSource   = arg_type("Casting Source", arg_underlying_type::int_signed, {
            enum_value_definition(0, "Left"),
            enum_value_definition(1, "Right"),
            enum_value_definition(2, "Voice"),
            enum_value_definition(3, "Instant"),
         });
         arg_form_type Cell            = arg_form_type("Cell", { form_type::cell });
         arg_form_type Class           = arg_form_type("Class", { form_type::combat_class });
         arg_type      CrimeType       = arg_type("Crime Type", arg_underlying_type::int_signed, {
            enum_value_definition(-1, "None"),
            enum_value_definition( 0, "Steal"),
            enum_value_definition( 1, "Pickpocket"),
            enum_value_definition( 2, "Trespass"),
            enum_value_definition( 3, "Attack"),
            enum_value_definition( 4, "Murder"),
            enum_value_definition( 5, "Escape Jail"),
            enum_value_definition( 6, "Werewolf Transformation"),
         });
         arg_type      CriticalStage   = arg_type("Critical Stage", arg_underlying_type::int_signed, {
            enum_value_definition(0, "None"),
            enum_value_definition(1, "Goo Start"),
            enum_value_definition(2, "Goo End"),
            enum_value_definition(3, "Disintegrate Start"),
            enum_value_definition(4, "Disintegrate End"),
         });
         arg_form_type EffectItem      = arg_form_type("Effect Item", { form_type::spell, form_type::potion, form_type::enchantment, form_type::ingredient, form_type::scroll });
         arg_form_type EncounterZone   = arg_form_type("Encounter Zone", { form_type::encounter_zone });
         arg_type      EquipType       = arg_type("Equip Type (Deprecated/Broken)", arg_underlying_type::int_unsigned);
         arg_type      Event           = arg_type("Event", arg_underlying_type::event);
         arg_type      EventData       = arg_type("Event Data", arg_underlying_type::event_data);
         arg_form_type Faction         = arg_form_type("Faction", { form_type::faction });
         arg_type      Float           = arg_type("Float", arg_underlying_type::float32);
         arg_form_type FormList        = arg_form_type("Form List", { form_type::formlist });
         arg_enum_type FormType        = arg_enum_type("Form Type", 0, {
            "Activator",
            "Armor",
            "Book",
            "Container",
            "Door",
            "Ingredient",
            "Light",
            "MiscItem",
            "Static",
            "Grass",
            "Tree",
            "Weapon",
            "Actor",
            "LeveledCharacter",
            "Spell",
            "Enchantment",
            "Potion",
            "LeveledItem",
            "Key",
            "Ammo",
            "Flora",
            "Furniture",
            "Sound Marker",
            "LandTexture",
            "CombatStyle",
            "LoadScreen",
            "LeveledSpell",
            "AnimObject",
            "WaterType",
            "IdleMarker",
            "EffectShader",
            "Projectile",
            "TalkingActivator",
            "Explosion",
            "TextureSet",
            "Debris",
            "MenuIcon",
            "formlist",
            "Perk",
            "BodyPartData",
            "AddOnNode",
            "MovableStatic",
            "CameraShot",
            "ImpactData",
            "ImpactDataSet",
            "Quest",
            "Package",
            "VoiceType",
            "Class",
            "Race",
            "Eyes",
            "HeadPart",
            "Faction",
            "Note",
            "Weather",
            "Climate",
            "ArmorAddon",
            "Global",
            "Imagespace",
            "Imagespace Modifier",
            "Encounter Zone",
            "Message",
            "Constructible Object",
            "Acoustic Space",
            "Ragdoll",
            "Script",
            "Magic Effect",
            "Music Type",
            "Static Collection",
            "Keyword",
            "Location",
            "Location Ref Type",
            "Footstep",
            "Footstep Set",
            "Material Type",
            "Actor Action",
            "Music Track",
            "Word of Power",
            "Shout",
            "Relationship",
            "Equip Slot",
            "Association Type",
            "Outfit",
            "Art Object",
            "Material Object",
            "Lighting Template",
            "Shader Particle Geometry",
            "Visual Effect",
            "Apparatus",
            "Movement Type",
            "Hazard",
            "SM Event Node",
            "Sound Descriptor",
            "Dual Cast Data",
            "Sound Category",
            "Soul Gem",
            "Sound Output Model",
            "Collision Layer",
            "Scroll",
            "ColorForm",
            "Reverb Parameters",
         });
         arg_form_type Furniture       = arg_form_type("Furniture", { form_type::furniture });
         arg_type      FurnitureAnim   = arg_type("Furniture Anim", arg_underlying_type::int_signed, {
            enum_value_definition(1, "Sit"),
            enum_value_definition(2, "Sleep"),
            enum_value_definition(4, "Lean"),
         });
         arg_type      FurnitureEntry  = arg_type("Furniture Entry", arg_underlying_type::int_signed, {
            enum_value_definition(0x01, "Front"),
            enum_value_definition(0x02, "Back"),
            enum_value_definition(0x04, "Left"),
            enum_value_definition(0x08, "Right"),
            enum_value_definition(0x10, "Up"),
         });
         arg_form_type Global          = arg_form_type("Global Variable", { form_type::global });
         arg_form_type Idle            = arg_form_type("Idle", { form_type::idle });
         arg_type      Integer         = arg_type("Integer", arg_underlying_type::int_signed);
         arg_form_type InventoryItem   = arg_form_type("Inventory Item", {
            form_type::ammo,
            form_type::armor,
            form_type::book,
            form_type::constructible_object,
            form_type::formlist,
            form_type::ingredient,
            form_type::key,
            form_type::leveled_item,
            form_type::light, // light forms can be flagged as "carryable;" this is how torches work
            form_type::misc_item,
            form_type::potion,
            form_type::scroll,
            form_type::soul_gem,
            form_type::weapon,
         });
         arg_form_type Keyword         = arg_form_type("Keyword", { form_type::keyword });
         arg_form_type KnowableForm    = arg_form_type("Knowable Form", { form_type::magic_effect, form_type::word_of_power });
         arg_form_type Location        = arg_form_type("Location", { form_type::location });
         arg_form_type LocRefType      = arg_form_type("Location Ref Type", { form_type::location_ref_type });
         arg_form_type MagicEffect     = arg_form_type("Magic Effect", { form_type::magic_effect });
         arg_type      MiscStat        = arg_type("Misc Stat", arg_underlying_type::int_unsigned, {
            enum_value_definition(0xFCDD5011, "Animals Killed"),
            enum_value_definition(0x366D84CF, "Armor Improved"),
            enum_value_definition(0x023497E6, "Armor Made"),
            enum_value_definition(0x8E20D7C9, "Assaults"),
            enum_value_definition(0x579FFA75, "Automations Killed"),
            enum_value_definition(0xB9B50725, "Backstabs"),
            enum_value_definition(0xED6A0EF2, "Barters"),
            enum_value_definition(0xCCB952CE, "Books Read"),
            enum_value_definition(0x317E8B4C, "Brawls Won"),
            enum_value_definition(0x1D79006B, "Bribes"),
            enum_value_definition(0x3602DE8F, "Bunnies Slaughtered"),
            enum_value_definition(0x53D9E9B5, "Chests Looted"),
            enum_value_definition(0x683C1980, "Civil War Quests Completed"),
            enum_value_definition(0x66CCC50A, "College of Winterhold Quests Completed"),
            enum_value_definition(0x40B11EFE, "Creatures Killed"),
            enum_value_definition(0x22D5BA38, "Critical Strikes"),
            enum_value_definition(0xA930980F, "Daedra Killed"),
            enum_value_definition(0x3558374B, "Daedric Quests Completed"),
            enum_value_definition(0x37A76425, "Dawnguard Quests Completed"),
            enum_value_definition(0x2BDAC36F, "Days as a Vampire"),
            enum_value_definition(0x6E684590, "Days as a Werewolf"),
            enum_value_definition(0xB6F118DB, "Days Jailed"),
            enum_value_definition(0x3C626A90, "Days Passed"),
            enum_value_definition(0x8556AD88, "Diseases Contracted"),
            enum_value_definition(0x46D6FBBC, "Dragon Souls Collected"),
            enum_value_definition(0x8D115F78, "Dragonborn Quests Completed"),
            enum_value_definition(0xAA444695, "Dungeons Cleared"),
            enum_value_definition(0x1A37F336, "Eastmarch Bounty"),
            enum_value_definition(0x5AC3A8ED, "Falkreath Bounty"),
            enum_value_definition(0x87B12ECC, "Favorite School"),
            enum_value_definition(0x518BBC4E, "Favorite Shout"),
            enum_value_definition(0x41DD77A6, "Favorite Spell"),
            enum_value_definition(0x171C5391, "Favorite Weapon"),
            enum_value_definition(0x4F041AA2, "Fines Paid"),
            enum_value_definition(0x9311B22B, "Food Eaten"),
            enum_value_definition(0x57C089F7, "Gold Found"),
            enum_value_definition(0xD20EDA4F, "Haafingar Bounty"),
            enum_value_definition(0x516C486D, "Hjaalmarch Bounty"),
            enum_value_definition(0xB0A1E32E, "Horses Owned"),
            enum_value_definition(0xEBAE35E8, "Horses Stolen"),
            enum_value_definition(0xFA024018, "Hours Slept"),
            enum_value_definition(0xCAD2ECA1, "Hours Waiting"),
            enum_value_definition(0x527DF857, "Houses Owned"),
            enum_value_definition(0x47B4A015, "Ingredients Eaten"),
            enum_value_definition(0xCE842356, "Ingredients Harvested"),
            enum_value_definition(0x7D2E57C0, "Intimidations"),
            enum_value_definition(0xC21702B5, "Items Pickpocketed"),
            enum_value_definition(0x82F190C2, "Items Stolen"),
            enum_value_definition(0x6627464B, "Jail Escapes"),
            enum_value_definition(0x3520E710, "Largest Bounty"),
            enum_value_definition(0x8A24FDE2, "Locations Discovered"),
            enum_value_definition(0x5829CC2E, "Locks Picked"),
            enum_value_definition(0x88089979, "Magic Items Made"),
            enum_value_definition(0x7EA26C2D, "Main Quests Completed"),
            enum_value_definition(0x7187A208, "Mauls"),
            enum_value_definition(0x98EE55DC, "Misc Objectives Completed"),
            enum_value_definition(0xFA06230B, "Most Gold Carried"),
            enum_value_definition(0xD37C6909, "Murders"),
            enum_value_definition(0x22C2CBD0, "Necks Bitten"),
            enum_value_definition(0xBEEBCC87, "Nirnroots Found"),
            enum_value_definition(0x56CCFC54, "NumVampirePerks"),
            enum_value_definition(0x76A1A5C0, "NumWerewolfPerks"),
            enum_value_definition(0xF22A8133, "People Killed"),
            enum_value_definition(0x47A78467, "Persuasions"),
            enum_value_definition(0xF2BAC234, "Pockets Picked"),
            enum_value_definition(0x17C64668, "Poisons Mixed"),
            enum_value_definition(0x7D8F2EA6, "Poisons Used"),
            enum_value_definition(0x4228DE85, "Potions Mixed"),
            enum_value_definition(0x9631EC11, "Potions Used"),
            enum_value_definition(0xDE6C73FE, "Questlines Completed"),
            enum_value_definition(0x0D7B8B16, "Quests Completed"),
            enum_value_definition(0xBB39399E, "Shouts Learned"),
            enum_value_definition(0x731B5333, "Shouts Mastered"),
            enum_value_definition(0xF921D8BA, "Shouts Unlocked"),
            enum_value_definition(0xB1AE4792, "Side Quests Completed"),
            enum_value_definition(0xACE470D7, "Skill Books Read"),
            enum_value_definition(0xF33130CE, "Skill Increases"),
            enum_value_definition(0xB556CC52, "Sneak Attacks"),
            enum_value_definition(0x9E8F2530, "Solstheim Locations Discovered"),
            enum_value_definition(0xA74CBE83, "Soul Gems Used"),
            enum_value_definition(0xC2C9E233, "Souls Trapped"),
            enum_value_definition(0x5EC89F1A, "Spells Learned"),
            enum_value_definition(0x593E44F8, "StalhrimItemsCrafted"),
            enum_value_definition(0xB251A346, "Standing Stones Found"),
            enum_value_definition(0x05D45702, "Stores Invested In"),
            enum_value_definition(0xD0FE7031, "The Companions Quests Completed"),
            enum_value_definition(0x52BA68CB, "The Dark Brotherhood Quests Completed"),
            enum_value_definition(0x3E267D77, "The Pale Bounty"),
            enum_value_definition(0x69B48177, "The Reach Bounty"),
            enum_value_definition(0x50A23F69, "The Rift Bounty"),
            enum_value_definition(0x62B2E95D, "Thieves' Guild Quests Completed"),
            enum_value_definition(0x944CEA93, "Times Jailed"),
            enum_value_definition(0x50AAB633, "Times Shouted"),
            enum_value_definition(0x99BB86D8, "Total Lifetime Bounty"),
            enum_value_definition(0x4C252391, "Training Sessions"),
            enum_value_definition(0x7AEA9C2B, "Trespasses"),
            enum_value_definition(0xA67626F4, "Tribal Orcs Bounty"),
            enum_value_definition(0x41D4BC0F, "Undead Killed"),
            enum_value_definition(0xF39260A1, "Vampirism Cures"),
            enum_value_definition(0x61A5C5A9, "Weapons Disarmed"),
            enum_value_definition(0x1D3BA844, "Weapons Improved"),
            enum_value_definition(0x25F1EA25, "Weapons Made"),
            enum_value_definition(0x38A2DD66, "Werewolf Transformations"),
            enum_value_definition(0x4231FA4F, "Whiterun Bounty"),
            enum_value_definition(0x92565767, "Wings Plucked"),
            enum_value_definition(0xC7FC518D, "Winterhold Bounty"),
            enum_value_definition(0x949FA7BC, "Words of Power Learned"),
            enum_value_definition(0x2C6E3FC0, "Words of Power Unlocked"),
         });
         arg_form_type ObjectReference = arg_form_type("ObjectReference", {
            form_type::actor,
            form_type::reference,
            form_type::arrow,
            form_type::barrier,
            form_type::beam,
            form_type::cone,
            form_type::flame,
            form_type::grenade,
            form_type::missile,
            form_type::placed_hazard,
         }, true);
         arg_form_type OwnerForm       = arg_form_type("Owner", { form_type::actor_base, form_type::faction });
         arg_form_type Package         = arg_form_type("Package", { form_type::package }, true);
         arg_type      PackageData     = arg_type("Package Data", arg_underlying_type::package_data);
         arg_form_type Perk            = arg_form_type("Perk", { form_type::perk });
         arg_form_type Quest           = arg_form_type("Quest", { form_type::quest });
         arg_type      QuestStage      = arg_type("Quest Stage", arg_underlying_type::quest_stage);
         arg_form_type Race            = arg_form_type("Race", { form_type::race });
         arg_form_type Region          = arg_form_type("Region", { form_type::region });
         arg_form_type Scene           = arg_form_type("Scene", { form_type::scene });
         arg_type      Sex             = arg_type("Axis", arg_underlying_type::int_signed, {
            enum_value_definition(0, "Male"),
            enum_value_definition(1, "Female"),
         });
         arg_form_type Shout           = arg_form_type("Shout", { form_type::shout });
         arg_form_type Spell           = arg_form_type("Spell", { form_type::spell });
         arg_type      String          = arg_type("String", arg_underlying_type::string);
         arg_type      VATSValueFunction = arg_type("VATS Value Function", arg_underlying_type::int_signed, {
            enum_value_definition( 0, "Weapon Is"),
            enum_value_definition( 1, "Weapon In List"),
            enum_value_definition( 2, "Target Is"),
            enum_value_definition( 3, "Target In List"),
            enum_value_definition( 4, "Target Distance"),
            enum_value_definition( 5, "Target Part"),
            enum_value_definition( 6, "VATS Action"),
            enum_value_definition( 7, "Is Success"),
            enum_value_definition( 8, "Is Critical"),
            enum_value_definition( 9, "Critical Effect Is"),
            enum_value_definition(10, "Critical Effect In List"),
            enum_value_definition(11, "Is Fatal"),
            enum_value_definition(12, "Explode Part"),
            enum_value_definition(13, "Dismember Part"),
            enum_value_definition(14, "Cripple Part"),
            enum_value_definition(15, "Weapon Type Is"),
            enum_value_definition(16, "Is Stranger"),
            enum_value_definition(17, "Is Paralyzing Palm"),
            enum_value_definition(18, "Projectile Type Is"),
            enum_value_definition(19, "Delivery Type Is"),
            enum_value_definition(20, "Casting Type Is"),
         });
         arg_form_type Voicetype       = arg_form_type("Voicetype", { form_type::voicetype });
         arg_type      WardState       = arg_type("Ward State", arg_underlying_type::int_signed, {
            enum_value_definition(0, "None"),
            enum_value_definition(1, "Absorb"),
            enum_value_definition(2, "Break"),
         });
         arg_form_type Weather         = arg_form_type("Weather", { form_type::weather });
         arg_form_type Worldspace      = arg_form_type("Worldspace", { form_type::worldspace });
         //
         namespace _VATSValueTypes {
            arg_form_type Weapon     = arg_form_type("Weapon", { form_type::weapon });
            arg_form_type WeaponList = arg_form_type("Weapon List", { form_type::formlist });
            arg_form_type Target     = arg_form_type("Target", { form_type::actor_base });
            arg_form_type TargetList = arg_form_type("Target List", { form_type::formlist });
            arg_type      TargetPart = ActorValue;
            arg_type      VATSAction = arg_type("VATS Action", arg_underlying_type::int_unsigned, {
               enum_value_definition(0, "Unarmed"),
               enum_value_definition(1, "One-Handed Melee"),
               enum_value_definition(2, "Two-Handed Melee"),
               enum_value_definition(3, "Magic"),
               enum_value_definition(4, "Ranged"),
               enum_value_definition(5, "Reload"),
               enum_value_definition(6, "Crouch"),
               enum_value_definition(7, "Stand"),
               enum_value_definition(8, "Switch Weapon"),
               enum_value_definition(9, "Draw/Sheathe Weapon"),
               enum_value_definition(10, "Heal"),
               enum_value_definition(11, "Player Death"),
            });
            arg_form_type CriticalEffect = arg_form_type("Critical Effect", { form_type::spell });
            arg_form_type CriticalEffectList = arg_form_type("Critical Effect List", { form_type::formlist });
            arg_enum_type WeaponAnimType = arg_enum_type("Weapon Animation Type", 0, {
               "Hand-to-Hand",
               "Sword (1HM)",
               "Dagger (1HM)",
               "War Axe (1HM)",
               "Mace (1HM)",
               "Greatsword (2HM)",
               "Battleaxe (2HM)",
               "Bow",
               "Staff",
               "Crossbow",
            });
            arg_type  ProjectileType = arg_type("Projectile Type", arg_underlying_type::int_unsigned, {
               enum_value_definition(0, "Missile"),
               enum_value_definition(1, "Lobber"),
               enum_value_definition(2, "Beam"),
               enum_value_definition(3, "Flame"),
               enum_value_definition(4, "Cone"),
               enum_value_definition(5, "Barrier"),
               enum_value_definition(6, "Arrow"),
            });
            arg_type  DeliveryType = arg_type("Delivery Type", arg_underlying_type::int_unsigned, {
               enum_value_definition(0, "Self"),
               enum_value_definition(1, "Touch"),
               enum_value_definition(2, "Aimed"),
               enum_value_definition(3, "Target Actor"),
               enum_value_definition(4, "Target Location"),
            });
            arg_type  CastingType = arg_type("Casting Type", arg_underlying_type::int_unsigned, {
               enum_value_definition(0, "Constant Effect"),
               enum_value_definition(1, "Fire and Forget"),
               enum_value_definition(2, "Concentration"),
               enum_value_definition(3, "Scroll"),
            });
         }
      }
      arg_type* arg_vats_type::resolve_union(arg_type* previousType, const condition_arg_value* previousValue) const noexcept {
         if (previousType != &arg_types::VATSValueFunction)
            return nullptr;
         //
         using namespace arg_types::_VATSValueTypes;
         //
         switch (previousValue->dword) {
            case 0:
               return &Weapon;
            case 1:
               return &WeaponList;
            case 2:
               return &Target;
            case 3:
               return &TargetList;
            case 4: // Target Distance; has no second argument
               return nullptr;
            case 5:
               return &TargetPart;
            case 6:
               return &VATSAction;
            case 7: // Is Success; has no second argument
            case 8: // Is Critical; has no second argument
               return nullptr;
            case 9:
               return &CriticalEffect;
            case 10:
               return &CriticalEffectList;
            case 11: // Is Fatal; has no second argument
               return nullptr;
            case 12: // Explode Part; has no second argument
            case 13: // Dismember Part; has no second argument
            case 14: // Cripple Part; has no second argument
               return nullptr;
            case 15:
               return &WeaponAnimType;
            case 16: // Is Stranger; has no second argument
            case 17: // Is Paralyzing Palm; has no second argument
               return nullptr;
            case 18:
               return &ProjectileType;
            case 19:
               return &DeliveryType;
            case 20:
               return &CastingType;
         }
         return nullptr;
      }
      namespace arg_types {
         arg_vats_type VATSValue = arg_vats_type();
      }
   }
}