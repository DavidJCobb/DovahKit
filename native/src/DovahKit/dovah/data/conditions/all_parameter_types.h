#pragma once
#include <array>
#include "./parameter_typeinfo.h"

namespace dovah::conditions::parameter_types {
   namespace _enums {
      inline constexpr const auto ActorValue        = members{
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
         "ReflectDamage"
      };
      inline constexpr const auto AdvanceAction     = std::array{
         enum_member{0, "Normal Usage"},
         enum_member{1, "Power Attack"},
         enum_member{2, "Bash"},
         enum_member{3, "Lockpick Success"},
         enum_member{4, "Lockpick Broken"},
      };
      inline constexpr const auto Alignment         = members{
         "Good",
         "Neutral",
         "Evil",
         "Very Good",
         "Very Evil",
      };
      inline constexpr const auto CastingSource     = members{
         "Left",
         "Right",
         "Voice",
         "Instant",
      };
      inline constexpr const auto CrimeType         = std::array{
         enum_member{-1, "None" },
         enum_member{ 0, "Steal" },
         enum_member{ 1, "Pickpocket" },
         enum_member{ 2, "Trespass" },
         enum_member{ 3, "Attack" },
         enum_member{ 4, "Murder" },
         enum_member{ 5, "Escape Jail" },
         enum_member{ 6, "Werewolf Transformation" },
      };
      inline constexpr const auto CriticalStage     = members{
         "None",
         "Goo Start",
         "Goo End",
         "Disintegrate Start",
         "Disintegrate End",
      };
      inline constexpr const auto FormType          = members{
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
      };
      inline constexpr const auto FurnitureAnim     = std::array{
         enum_member{1, "Sit"},
         enum_member{2, "Sleep"},
         enum_member{4, "Lean"},
      };
      inline constexpr const auto FurnitureEntry    = std::array{
         enum_member{0x00010000, "Front"},
         enum_member{0x00020000, "Back"},
         enum_member{0x00040000, "Right"},
         enum_member{0x00080000, "Left"},
         enum_member{0x00100000, "Up"},
      };
      inline constexpr const auto MiscStat          = std::array{
         enum_member{ uint32_t(0xFCDD5011), "Animals Killed" },
         enum_member{ uint32_t(0x366D84CF), "Armor Improved" },
         enum_member{ uint32_t(0x023497E6), "Armor Made" },
         enum_member{ uint32_t(0x8E20D7C9), "Assaults" },
         enum_member{ uint32_t(0x579FFA75), "Automations Killed" },
         enum_member{ uint32_t(0xB9B50725), "Backstabs" },
         enum_member{ uint32_t(0xED6A0EF2), "Barters" },
         enum_member{ uint32_t(0xCCB952CE), "Books Read" },
         enum_member{ uint32_t(0x317E8B4C), "Brawls Won" },
         enum_member{ uint32_t(0x1D79006B), "Bribes" },
         enum_member{ uint32_t(0x3602DE8F), "Bunnies Slaughtered" },
         enum_member{ uint32_t(0x53D9E9B5), "Chests Looted" },
         enum_member{ uint32_t(0x683C1980), "Civil War Quests Completed" },
         enum_member{ uint32_t(0x66CCC50A), "College of Winterhold Quests Completed" },
         enum_member{ uint32_t(0x40B11EFE), "Creatures Killed" },
         enum_member{ uint32_t(0x22D5BA38), "Critical Strikes" },
         enum_member{ uint32_t(0xA930980F), "Daedra Killed" },
         enum_member{ uint32_t(0x3558374B), "Daedric Quests Completed" },
         enum_member{ uint32_t(0x37A76425), "Dawnguard Quests Completed" },
         enum_member{ uint32_t(0x2BDAC36F), "Days as a Vampire" },
         enum_member{ uint32_t(0x6E684590), "Days as a Werewolf" },
         enum_member{ uint32_t(0xB6F118DB), "Days Jailed" },
         enum_member{ uint32_t(0x3C626A90), "Days Passed" },
         enum_member{ uint32_t(0x8556AD88), "Diseases Contracted" },
         enum_member{ uint32_t(0x46D6FBBC), "Dragon Souls Collected" },
         enum_member{ uint32_t(0x8D115F78), "Dragonborn Quests Completed" },
         enum_member{ uint32_t(0xAA444695), "Dungeons Cleared" },
         enum_member{ uint32_t(0x1A37F336), "Eastmarch Bounty" },
         enum_member{ uint32_t(0x5AC3A8ED), "Falkreath Bounty" },
         enum_member{ uint32_t(0x87B12ECC), "Favorite School" },
         enum_member{ uint32_t(0x518BBC4E), "Favorite Shout" },
         enum_member{ uint32_t(0x41DD77A6), "Favorite Spell" },
         enum_member{ uint32_t(0x171C5391), "Favorite Weapon" },
         enum_member{ uint32_t(0x4F041AA2), "Fines Paid" },
         enum_member{ uint32_t(0x9311B22B), "Food Eaten" },
         enum_member{ uint32_t(0x57C089F7), "Gold Found" },
         enum_member{ uint32_t(0xD20EDA4F), "Haafingar Bounty" },
         enum_member{ uint32_t(0x516C486D), "Hjaalmarch Bounty" },
         enum_member{ uint32_t(0xB0A1E32E), "Horses Owned" },
         enum_member{ uint32_t(0xEBAE35E8), "Horses Stolen" },
         enum_member{ uint32_t(0xFA024018), "Hours Slept" },
         enum_member{ uint32_t(0xCAD2ECA1), "Hours Waiting" },
         enum_member{ uint32_t(0x527DF857), "Houses Owned" },
         enum_member{ uint32_t(0x47B4A015), "Ingredients Eaten" },
         enum_member{ uint32_t(0xCE842356), "Ingredients Harvested" },
         enum_member{ uint32_t(0x7D2E57C0), "Intimidations" },
         enum_member{ uint32_t(0xC21702B5), "Items Pickpocketed" },
         enum_member{ uint32_t(0x82F190C2), "Items Stolen" },
         enum_member{ uint32_t(0x6627464B), "Jail Escapes" },
         enum_member{ uint32_t(0x3520E710), "Largest Bounty" },
         enum_member{ uint32_t(0x8A24FDE2), "Locations Discovered" },
         enum_member{ uint32_t(0x5829CC2E), "Locks Picked" },
         enum_member{ uint32_t(0x88089979), "Magic Items Made" },
         enum_member{ uint32_t(0x7EA26C2D), "Main Quests Completed" },
         enum_member{ uint32_t(0x7187A208), "Mauls" },
         enum_member{ uint32_t(0x98EE55DC), "Misc Objectives Completed" },
         enum_member{ uint32_t(0xFA06230B), "Most Gold Carried" },
         enum_member{ uint32_t(0xD37C6909), "Murders" },
         enum_member{ uint32_t(0x22C2CBD0), "Necks Bitten" },
         enum_member{ uint32_t(0xBEEBCC87), "Nirnroots Found" },
         enum_member{ uint32_t(0x56CCFC54), "NumVampirePerks" },
         enum_member{ uint32_t(0x76A1A5C0), "NumWerewolfPerks" },
         enum_member{ uint32_t(0xF22A8133), "People Killed" },
         enum_member{ uint32_t(0x47A78467), "Persuasions" },
         enum_member{ uint32_t(0xF2BAC234), "Pockets Picked" },
         enum_member{ uint32_t(0x17C64668), "Poisons Mixed" },
         enum_member{ uint32_t(0x7D8F2EA6), "Poisons Used" },
         enum_member{ uint32_t(0x4228DE85), "Potions Mixed" },
         enum_member{ uint32_t(0x9631EC11), "Potions Used" },
         enum_member{ uint32_t(0xDE6C73FE), "Questlines Completed" },
         enum_member{ uint32_t(0x0D7B8B16), "Quests Completed" },
         enum_member{ uint32_t(0xBB39399E), "Shouts Learned" },
         enum_member{ uint32_t(0x731B5333), "Shouts Mastered" },
         enum_member{ uint32_t(0xF921D8BA), "Shouts Unlocked" },
         enum_member{ uint32_t(0xB1AE4792), "Side Quests Completed" },
         enum_member{ uint32_t(0xACE470D7), "Skill Books Read" },
         enum_member{ uint32_t(0xF33130CE), "Skill Increases" },
         enum_member{ uint32_t(0xB556CC52), "Sneak Attacks" },
         enum_member{ uint32_t(0x9E8F2530), "Solstheim Locations Discovered" },
         enum_member{ uint32_t(0xA74CBE83), "Soul Gems Used" },
         enum_member{ uint32_t(0xC2C9E233), "Souls Trapped" },
         enum_member{ uint32_t(0x5EC89F1A), "Spells Learned" },
         enum_member{ uint32_t(0x593E44F8), "StalhrimItemsCrafted" },
         enum_member{ uint32_t(0xB251A346), "Standing Stones Found" },
         enum_member{ uint32_t(0x05D45702), "Stores Invested In" },
         enum_member{ uint32_t(0xD0FE7031), "The Companions Quests Completed" },
         enum_member{ uint32_t(0x52BA68CB), "The Dark Brotherhood Quests Completed" },
         enum_member{ uint32_t(0x3E267D77), "The Pale Bounty" },
         enum_member{ uint32_t(0x69B48177), "The Reach Bounty" },
         enum_member{ uint32_t(0x50A23F69), "The Rift Bounty" },
         enum_member{ uint32_t(0x62B2E95D), "Thieves' Guild Quests Completed" },
         enum_member{ uint32_t(0x944CEA93), "Times Jailed" },
         enum_member{ uint32_t(0x50AAB633), "Times Shouted" },
         enum_member{ uint32_t(0x99BB86D8), "Total Lifetime Bounty" },
         enum_member{ uint32_t(0x4C252391), "Training Sessions" },
         enum_member{ uint32_t(0x7AEA9C2B), "Trespasses" },
         enum_member{ uint32_t(0xA67626F4), "Tribal Orcs Bounty" },
         enum_member{ uint32_t(0x41D4BC0F), "Undead Killed" },
         enum_member{ uint32_t(0xF39260A1), "Vampirism Cures" },
         enum_member{ uint32_t(0x61A5C5A9), "Weapons Disarmed" },
         enum_member{ uint32_t(0x1D3BA844), "Weapons Improved" },
         enum_member{ uint32_t(0x25F1EA25), "Weapons Made" },
         enum_member{ uint32_t(0x38A2DD66), "Werewolf Transformations" },
         enum_member{ uint32_t(0x4231FA4F), "Whiterun Bounty" },
         enum_member{ uint32_t(0x92565767), "Wings Plucked" },
         enum_member{ uint32_t(0xC7FC518D), "Winterhold Bounty" },
         enum_member{ uint32_t(0x949FA7BC), "Words of Power Learned" },
         enum_member{ uint32_t(0x2C6E3FC0), "Words of Power Unlocked" },
      };
      inline constexpr const auto Sex               = std::array{
         enum_member{0, "Male"},
         enum_member{1, "Female"},
      };
      inline constexpr const auto VATSValueFunction = std::array{
         enum_member{ 0, "Weapon Is"},
         enum_member{ 1, "Weapon In List"},
         enum_member{ 2, "Target Is"},
         enum_member{ 3, "Target In List"},
         enum_member{ 4, "Target Distance"},
         enum_member{ 5, "Target Part"},
         enum_member{ 6, "VATS Action"},
         enum_member{ 7, "Is Success"},
         enum_member{ 8, "Is Critical"},
         enum_member{ 9, "Critical Effect Is"},
         enum_member{10, "Critical Effect In List"},
         enum_member{11, "Is Fatal"},
         enum_member{12, "Explode Part"},
         enum_member{13, "Dismember Part"},
         enum_member{14, "Cripple Part"},
         enum_member{15, "Weapon Type Is"},
         enum_member{16, "Is Stranger"},
         enum_member{17, "Is Paralyzing Palm"},
         enum_member{18, "Projectile Type Is"},
         enum_member{19, "Delivery Type Is"},
         enum_member{20, "Casting Type Is"},
      };
      inline constexpr const auto WardState         = members{
         "None",
         "Absorb",
         "Break",
      };

      // VATS enums:

      inline constexpr const auto CastingType    = members{
         "Constant Effect",
         "Fire and Forget",
         "Concentration",
         "Scroll",
      };
      inline constexpr const auto DeliveryType   = members{
         "Self",
         "Touch",
         "Aimed",
         "Target Actor",
         "Target Location",
      };
      inline constexpr const auto ProjectileType = members{
         "Missile",
         "Lobber",
         "Beam",
         "Flame",
         "Cone",
         "Barrier",
         "Arrow",
      };
      inline constexpr const auto VATSAction     = std::array{
         enum_member{ 0, "Unarmed"},
         enum_member{ 1, "One-Handed Melee"},
         enum_member{ 2, "Two-Handed Melee"},
         enum_member{ 3, "Magic"},
         enum_member{ 4, "Ranged"},
         enum_member{ 5, "Reload"},
         enum_member{ 6, "Crouch"},
         enum_member{ 7, "Stand"},
         enum_member{ 8, "Switch Weapon"},
         enum_member{ 9, "Draw/Sheathe Weapon"},
         enum_member{10, "Heal"},
         enum_member{11, "Player Death"},
      };
      inline constexpr const auto WeaponAnimType = members{
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
      };
   }
   namespace _form_type_lists {
      inline constexpr const auto BaseForm         = std::array{
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
      };
      inline constexpr const auto EffectItem       = std::array{
         form_type::spell,
         form_type::potion,
         form_type::enchantment,
         form_type::ingredient,
         form_type::scroll,
      };
      inline constexpr const auto InventoryItem    = std::array{
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
      };
      inline constexpr const auto KnowableForm     = std::array{
         form_type::magic_effect,
         form_type::word_of_power,
      };
      inline constexpr const auto ObjectReference  = std::array{
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
      };
      inline constexpr const auto OwnerForm        = std::array{
         form_type::actor_base,
         form_type::faction,
      };
      inline constexpr const auto VoicetypeOrList  = std::array{
         form_type::voicetype,
         form_type::formlist,
      };
      inline constexpr const auto WorldspaceOrList = std::array{
         form_type::worldspace,
         form_type::formlist,
      };
   }
   
   //inline constexpr const auto None              = parameter_typeinfo("None",                parameter_underlying_type::none);
   // Defined in `parameter_typeinfo.h`

   inline constexpr const auto Actor             = parameter_typeinfo("Actor",               form_type::actor, true);
   inline constexpr const auto ActorBase         = parameter_typeinfo("ActorBase",           form_type::actor_base);
   inline constexpr const auto ActorValue        = parameter_typeinfo("ActorValue",          _enums::ActorValue);
   inline constexpr const auto AdvanceAction     = parameter_typeinfo("Advance Action",      _enums::AdvanceAction);
   inline constexpr const auto Alias             = parameter_typeinfo("Alias",               parameter_underlying_type::alias);
   inline constexpr const auto Alignment         = parameter_typeinfo("Alignment",           _enums::Alignment);
   inline constexpr const auto AssociationType   = parameter_typeinfo("Association Type",    form_type::association_type);
   inline constexpr const auto Axis              = parameter_typeinfo::make_character_enum("Axis", "XYZ");
   inline constexpr const auto BaseForm          = parameter_typeinfo("Base Form",           _form_type_lists::BaseForm);
   inline constexpr const auto CastingSource     = parameter_typeinfo("Casting Source",      _enums::CastingSource);
   inline constexpr const auto Cell              = parameter_typeinfo("Cell",                form_type::cell);
   inline constexpr const auto Class             = parameter_typeinfo("Class",               form_type::combat_class);
   inline constexpr const auto CrimeType         = parameter_typeinfo("Crime Type",          _enums::CrimeType);
   inline constexpr const auto CriticalStage     = parameter_typeinfo("Critical Stage",      _enums::CriticalStage);
   inline constexpr const auto EffectItem        = parameter_typeinfo("Effect Item",         _form_type_lists::EffectItem);
   inline constexpr const auto EncounterZone     = parameter_typeinfo("Encounter Zone",      form_type::encounter_zone);
   inline constexpr const auto EquipType         = parameter_typeinfo("Equip Type (Deprecated/Broken)", parameter_underlying_type::int_unsigned);
   inline constexpr const auto EventData         = parameter_typeinfo("Event Data",          parameter_underlying_type::form);
   inline constexpr const auto Faction           = parameter_typeinfo("Faction",             form_type::faction);
   inline constexpr const auto Float             = parameter_typeinfo("Float",               parameter_underlying_type::float32);
   inline constexpr const auto FormList          = parameter_typeinfo("Form List",           form_type::formlist);
   inline constexpr const auto FormType          = parameter_typeinfo("Form Type",           _enums::FormType);
   inline constexpr const auto Furniture         = parameter_typeinfo("Furniture",           form_type::furniture);
   inline constexpr const auto FurnitureAnim     = parameter_typeinfo("Furniture Anim",      _enums::FurnitureAnim);
   inline constexpr const auto FurnitureEntry    = parameter_typeinfo("Furniture Entry",     _enums::FurnitureEntry);
   inline constexpr const auto Global            = parameter_typeinfo("Global Variable",     form_type::global);
   inline constexpr const auto Idle              = parameter_typeinfo("Idle",                form_type::idle);
   inline constexpr const auto Integer           = parameter_typeinfo("Integer",             parameter_underlying_type::int_signed);
   inline constexpr const auto InventoryItem     = parameter_typeinfo("Inventory Item",      _form_type_lists::InventoryItem);
   inline constexpr const auto Keyword           = parameter_typeinfo("Keyword",             form_type::keyword);
   inline constexpr const auto KnowableForm      = parameter_typeinfo("Knowable Form",       _form_type_lists::KnowableForm);
   inline constexpr const auto Location          = parameter_typeinfo("Location",            form_type::location);
   inline constexpr const auto LocRefType        = parameter_typeinfo("Location Ref Type",   form_type::location_ref_type);
   inline constexpr const auto MagicEffect       = parameter_typeinfo("Magic Effect",        form_type::magic_effect);
   inline constexpr const auto MiscStat          = parameter_typeinfo("Misc Stat",           _enums::MiscStat);
   inline constexpr const auto Note              = parameter_typeinfo("Note",                form_type::note);
   inline constexpr const auto ObjectReference   = parameter_typeinfo("ObjectReference",     _form_type_lists::ObjectReference, true);
   inline constexpr const auto OwnerForm         = parameter_typeinfo("Owner",               _form_type_lists::OwnerForm);
   inline constexpr const auto Package           = parameter_typeinfo("Package",             form_type::package, true);
   inline constexpr const auto PackageData       = parameter_typeinfo("Package Data",        parameter_underlying_type::package_data);
   inline constexpr const auto Perk              = parameter_typeinfo("Perk",                form_type::perk);
   inline constexpr const auto Quest             = parameter_typeinfo("Quest",               form_type::quest);
   inline constexpr const auto QuestStage        = parameter_typeinfo("Quest Stage",         parameter_underlying_type::quest_stage);
   inline constexpr const auto Race              = parameter_typeinfo("Race",                form_type::race);
   inline constexpr const auto Region            = parameter_typeinfo("Region",              form_type::region);
   inline constexpr const auto Scene             = parameter_typeinfo("Scene",               form_type::scene);
   inline constexpr const auto Sex               = parameter_typeinfo("Sex",                 _enums::Sex);
   inline constexpr const auto Shout             = parameter_typeinfo("Shout",               form_type::shout);
   inline constexpr const auto Spell             = parameter_typeinfo("Spell",               form_type::spell);
   inline constexpr const auto String            = parameter_typeinfo("String",              parameter_underlying_type::string);
   inline constexpr const auto VATSValueFunction = parameter_typeinfo("VATS Value Function", _enums::VATSValueFunction);
   inline constexpr const auto VoicetypeOrList   = parameter_typeinfo("Voicetype",           _form_type_lists::VoicetypeOrList);
   inline constexpr const auto WardState         = parameter_typeinfo("Ward State",          _enums::WardState);
   inline constexpr const auto Weather           = parameter_typeinfo("Weather",             form_type::weather);
   inline constexpr const auto WorldspaceOrList  = parameter_typeinfo("Worldspace",          _form_type_lists::WorldspaceOrList);
   
   namespace _VATSValueTypes {
      inline constexpr const auto Weapon             = parameter_typeinfo("Weapon",      form_type::weapon);
      inline constexpr const auto WeaponList         = parameter_typeinfo("Weapon List", form_type::formlist);
      inline constexpr const auto Target             = parameter_typeinfo("Target", form_type::actor_base);
      inline constexpr const auto TargetList         = parameter_typeinfo("Target List", form_type::formlist);
      inline constexpr const auto TargetPart         = ActorValue;
      inline constexpr const auto VATSAction         = parameter_typeinfo("VATS Action", _enums::VATSAction);
      inline constexpr const auto CriticalEffect     = parameter_typeinfo("Critical Effect", form_type::spell);
      inline constexpr const auto CriticalEffectList = parameter_typeinfo("Critical Effect List", form_type::formlist);
      inline constexpr const auto WeaponAnimType     = parameter_typeinfo("Weapon Animation Type", _enums::WeaponAnimType);
      inline constexpr const auto ProjectileType     = parameter_typeinfo("Projectile Type", _enums::ProjectileType);
      inline constexpr const auto DeliveryType       = parameter_typeinfo("Delivery Type", _enums::DeliveryType);
      inline constexpr const auto CastingType        = parameter_typeinfo("Casting Type", _enums::CastingType);
      
      inline constexpr const parameter_typeinfo* _decider(enumeration_parameter_value decider_dword_value) {
         switch (decider_dword_value) {
            case 0:
               return &Weapon;
            case 1:
               return &WeaponList;
            case 2:
               return &Target;
            case 3:
               return &TargetList;
            case 4: // Target Distance; has no second argument
               return &None;
            case 5:
               return &TargetPart;
            case 6:
               return &VATSAction;
            case 7: // Is Success; has no second argument
            case 8: // Is Critical; has no second argument
               return &None;
            case 9:
               return &CriticalEffect;
            case 10:
               return &CriticalEffectList;
            case 11: // Is Fatal; has no second argument
               return &None;
            case 12: // Explode Part; has no second argument
            case 13: // Dismember Part; has no second argument
            case 14: // Cripple Part; has no second argument
               return &None;
            case 15:
               return &WeaponAnimType;
            case 16: // Is Stranger; has no second argument
            case 17: // Is Paralyzing Palm; has no second argument
               return &None;
            case 18:
               return &ProjectileType;
            case 19:
               return &DeliveryType;
            case 20:
               return &CastingType;
         }
         return &None;
      }
   }
   inline constexpr const auto VATSValue = parameter_typeinfo::make_union("VATSValue", VATSValueFunction, _VATSValueTypes::_decider);
}