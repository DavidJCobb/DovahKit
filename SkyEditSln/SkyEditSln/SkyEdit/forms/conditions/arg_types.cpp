#include "arg_types.h"
#include "../../esp/LoadOrder.h"
#include "../../esp/TESPlugin.h"
#include "../../output.h"
#include "../../helpers/strings.h"

bool ConditionArgType::loadValue(ConditionArgValue& out, TESPluginSubrecord& subrecord) const noexcept {
   switch (this->underlying) {
      case ConditionArgUnderlyingType::formID:
         return subrecord.read(out.formID);
      default:
         return subrecord.read(out.dword);
   }
}
void ConditionArgType::toString(const ConditionArgValue& value, std::string& out) const noexcept {
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
      case ConditionArgUnderlyingType::formID:
         {
            FormStub* stub = LoadOrder::get().getForm(value.formID);
            if (stub) {
               char sig[5];
               FMT_SIGNATURE(formTypeFor(stub->formType).signature, sig);
               cobb::sprintf(out, "[%s:%08X]%s", sig, stub->formID, stub->get_editor_id());
            } else {
               cobb::sprintf(out, "[????:%08X]", value.formID);
            }
         }
         return;
      case ConditionArgUnderlyingType::float32:
         cobb::sprintf(out, "%f", value.float32);
         return;
      case ConditionArgUnderlyingType::character:
         out = (unsigned char)value.byte;
         return;
      case ConditionArgUnderlyingType::int_signed:
         cobb::sprintf(out, "%d", value.dword);
         return;
      case ConditionArgUnderlyingType::int_unsigned:
         cobb::sprintf(out, "%u", value.dword);
         return;
   }
   cobb::sprintf(out, "DWORD 0x%08X", value.dword); // should only happen if there's something we haven't finished yet
}
bool ConditionArgType::isValidForEnum(const ConditionArgValue& value) const noexcept {
   if (this->isEnum)
      for (auto& ev : this->enumValues)
         if (ev.value == value.dword)
            return true;
   return false;
}
void ConditionArgType::getEnumValues(std::vector<ConditionArgValue>& out) const noexcept {
   if (!this->isEnum) {
      out.clear();
      return;
   }
   auto count = this->enumValues.size();
   out.resize(count);
   for (uint32_t i = 0; i < count; i++)
      out[i].dword = this->enumValues[i].value;
}

namespace ConditionArgTypes {
   ConditionArgType     None            = ConditionArgType("None", ConditionArgUnderlyingType::none);
   ConditionArgFormType Actor           = ConditionArgFormType("Actor", { FormType::Actor });
   ConditionArgFormType ActorBase       = ConditionArgFormType("ActorBase", { FormType::ActorBase });
   ConditionArgEnumType ActorValue      = ConditionArgEnumType("ActorValue", 0, {
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
   ConditionArgType     AdvanceAction   = ConditionArgType("Advance Action", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "Normal Usage"),
      ConditionEnumValue(1, "Power Attack"),
      ConditionEnumValue(2, "Bash"),
      ConditionEnumValue(3, "Lockpick Success"),
      ConditionEnumValue(4, "Lockpick Broken"),
   });
   ConditionArgType     Alias           = ConditionArgType("Alias", ConditionArgUnderlyingType::aliasID);
   ConditionArgType     Alignment       = ConditionArgType("Alignment", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "Good"),
      ConditionEnumValue(1, "Neutral"),
      ConditionEnumValue(2, "Evil"),
      ConditionEnumValue(3, "Very Good"),
      ConditionEnumValue(4, "Very Evil"),
   });
   ConditionArgFormType AssociationType = ConditionArgFormType("Association Type", { FormType::AssociationType });
   ConditionArgType     Axis            = ConditionArgType("Axis", ConditionArgUnderlyingType::character, {
      ConditionEnumValue('X', "X"),
      ConditionEnumValue('Y', "Y"),
      ConditionEnumValue('Z', "Z"),
   });
   ConditionArgFormType BaseForm        = ConditionArgFormType("Base Form", {
      FormType::AcousticSpace, // Confirmed in CK. Strange, since these aren't placeable.
      FormType::Activator,
      FormType::ActorBase,
      FormType::Container,
      FormType::Door,
      FormType::Flora,
      FormType::Furniture,
      FormType::Grass,
      FormType::Hazard,
      FormType::IdleMarker,
      FormType::Light,
      FormType::MovableStatic,
      FormType::Projectile,
      FormType::Sound,
      FormType::Static,
      FormType::TalkingActivator,
      FormType::Tree,
      // Items:
      FormType::Ammo,
      FormType::Armor,
      FormType::ArmorAddon,
      FormType::Book,
      FormType::Key,
      FormType::LeveledItem,
      FormType::MiscItem,
      FormType::Potion,
      FormType::Scroll,
      FormType::SoulGem,
      FormType::Weapon,
      // Magic:
      FormType::Enchantment,
      FormType::LeveledSpell,
      FormType::Shout,
      FormType::Spell,
      // Other:
      FormType::FormList,
   });
   ConditionArgType     CastingSource   = ConditionArgType("Casting Source", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "Left"),
      ConditionEnumValue(1, "Right"),
      ConditionEnumValue(2, "Voice"),
      ConditionEnumValue(3, "Instant"),
   });
   ConditionArgFormType Cell            = ConditionArgFormType("Cell", { FormType::Cell });
   ConditionArgFormType Class           = ConditionArgFormType("Class", { FormType::Class });
   ConditionArgType     CrimeType       = ConditionArgType("Crime Type", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(-1, "None"),
      ConditionEnumValue( 0, "Steal"),
      ConditionEnumValue( 1, "Pickpocket"),
      ConditionEnumValue( 2, "Trespass"),
      ConditionEnumValue( 3, "Attack"),
      ConditionEnumValue( 4, "Murder"),
      ConditionEnumValue( 5, "Escape Jail"),
      ConditionEnumValue( 6, "Werewolf Transformation"),
   });
   ConditionArgType     CriticalStage   = ConditionArgType("Critical Stage", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "None"),
      ConditionEnumValue(1, "Goo Start"),
      ConditionEnumValue(2, "Goo End"),
      ConditionEnumValue(3, "Disintegrate Start"),
      ConditionEnumValue(4, "Disintegrate End"),
   });
   ConditionArgFormType EffectItem      = ConditionArgFormType("Effect Item", { FormType::Spell, FormType::Potion, FormType::Enchantment, FormType::Ingredient, FormType::Scroll });
   ConditionArgFormType EncounterZone   = ConditionArgFormType("Encounter Zone", { FormType::EncounterZone });
   ConditionArgType     EquipType       = ConditionArgType("Equip Type (Deprecated/Broken)", ConditionArgUnderlyingType::int_unsigned);
   ConditionArgFormType Faction         = ConditionArgFormType("Faction", { FormType::Faction });
   ConditionArgType     Float           = ConditionArgType("Float", ConditionArgUnderlyingType::float32);
   ConditionArgFormType FormList        = ConditionArgFormType("Form List", { FormType::FormList });
   ConditionArgFormType Furniture       = ConditionArgFormType("Furniture", { FormType::Furniture });
   ConditionArgType     FurnitureAnim   = ConditionArgType("Furniture Anim", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(1, "Sit"),
      ConditionEnumValue(2, "Sleep"),
      ConditionEnumValue(4, "Lean"),
   });
   ConditionArgType     FurnitureEntry  = ConditionArgType("Furniture Entry", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0x01, "Front"),
      ConditionEnumValue(0x02, "Back"),
      ConditionEnumValue(0x04, "Left"),
      ConditionEnumValue(0x08, "Right"),
      ConditionEnumValue(0x10, "Up"),
   });
   ConditionArgFormType Global          = ConditionArgFormType("Global Variable", { FormType::Global });
   ConditionArgFormType Idle            = ConditionArgFormType("Idle", { FormType::Idle });
   ConditionArgType     Integer         = ConditionArgType("Integer", ConditionArgUnderlyingType::int_signed);
   ConditionArgFormType InventoryItem   = ConditionArgFormType("Inventory Item", {
      FormType::Ammo,
      FormType::Armor,
      FormType::Book,
      FormType::ConstructibleObject,
      FormType::FormList,
      FormType::Ingredient,
      FormType::Key,
      FormType::LeveledItem,
      FormType::Light, // light forms can be flagged as "carryable;" this is how torches work
      FormType::MiscItem,
      FormType::Potion,
      FormType::Scroll,
      FormType::SoulGem,
      FormType::Weapon,
   });
   ConditionArgFormType Keyword         = ConditionArgFormType("Keyword", { FormType::Keyword });
   ConditionArgFormType KnowableForm    = ConditionArgFormType("Knowable Form", { FormType::MagicEffect, FormType::WordOfPower });
   ConditionArgFormType Location        = ConditionArgFormType("Location", { FormType::Location });
   ConditionArgFormType LocRefType      = ConditionArgFormType("Location Ref Type", { FormType::LocationRefType });
   ConditionArgFormType MagicEffect     = ConditionArgFormType("Magic Effect", { FormType::MagicEffect });
   ConditionArgType     MiscStat        = ConditionArgType("Misc Stat", ConditionArgUnderlyingType::int_unsigned, {
      ConditionEnumValue(0xFCDD5011, "Animals Killed"),
      ConditionEnumValue(0x366D84CF, "Armor Improved"),
      ConditionEnumValue(0x023497E6, "Armor Made"),
      ConditionEnumValue(0x8E20D7C9, "Assaults"),
      ConditionEnumValue(0x579FFA75, "Automations Killed"),
      ConditionEnumValue(0xB9B50725, "Backstabs"),
      ConditionEnumValue(0xED6A0EF2, "Barters"),
      ConditionEnumValue(0xCCB952CE, "Books Read"),
      ConditionEnumValue(0x317E8B4C, "Brawls Won"),
      ConditionEnumValue(0x1D79006B, "Bribes"),
      ConditionEnumValue(0x3602DE8F, "Bunnies Slaughtered"),
      ConditionEnumValue(0x53D9E9B5, "Chests Looted"),
      ConditionEnumValue(0x683C1980, "Civil War Quests Completed"),
      ConditionEnumValue(0x66CCC50A, "College of Winterhold Quests Completed"),
      ConditionEnumValue(0x40B11EFE, "Creatures Killed"),
      ConditionEnumValue(0x22D5BA38, "Critical Strikes"),
      ConditionEnumValue(0xA930980F, "Daedra Killed"),
      ConditionEnumValue(0x3558374B, "Daedric Quests Completed"),
      ConditionEnumValue(0x37A76425, "Dawnguard Quests Completed"),
      ConditionEnumValue(0x2BDAC36F, "Days as a Vampire"),
      ConditionEnumValue(0x6E684590, "Days as a Werewolf"),
      ConditionEnumValue(0xB6F118DB, "Days Jailed"),
      ConditionEnumValue(0x3C626A90, "Days Passed"),
      ConditionEnumValue(0x8556AD88, "Diseases Contracted"),
      ConditionEnumValue(0x46D6FBBC, "Dragon Souls Collected"),
      ConditionEnumValue(0x8D115F78, "Dragonborn Quests Completed"),
      ConditionEnumValue(0xAA444695, "Dungeons Cleared"),
      ConditionEnumValue(0x1A37F336, "Eastmarch Bounty"),
      ConditionEnumValue(0x5AC3A8ED, "Falkreath Bounty"),
      ConditionEnumValue(0x87B12ECC, "Favorite School"),
      ConditionEnumValue(0x518BBC4E, "Favorite Shout"),
      ConditionEnumValue(0x41DD77A6, "Favorite Spell"),
      ConditionEnumValue(0x171C5391, "Favorite Weapon"),
      ConditionEnumValue(0x4F041AA2, "Fines Paid"),
      ConditionEnumValue(0x9311B22B, "Food Eaten"),
      ConditionEnumValue(0x57C089F7, "Gold Found"),
      ConditionEnumValue(0xD20EDA4F, "Haafingar Bounty"),
      ConditionEnumValue(0x516C486D, "Hjaalmarch Bounty"),
      ConditionEnumValue(0xB0A1E32E, "Horses Owned"),
      ConditionEnumValue(0xEBAE35E8, "Horses Stolen"),
      ConditionEnumValue(0xFA024018, "Hours Slept"),
      ConditionEnumValue(0xCAD2ECA1, "Hours Waiting"),
      ConditionEnumValue(0x527DF857, "Houses Owned"),
      ConditionEnumValue(0x47B4A015, "Ingredients Eaten"),
      ConditionEnumValue(0xCE842356, "Ingredients Harvested"),
      ConditionEnumValue(0x7D2E57C0, "Intimidations"),
      ConditionEnumValue(0xC21702B5, "Items Pickpocketed"),
      ConditionEnumValue(0x82F190C2, "Items Stolen"),
      ConditionEnumValue(0x6627464B, "Jail Escapes"),
      ConditionEnumValue(0x3520E710, "Largest Bounty"),
      ConditionEnumValue(0x8A24FDE2, "Locations Discovered"),
      ConditionEnumValue(0x5829CC2E, "Locks Picked"),
      ConditionEnumValue(0x88089979, "Magic Items Made"),
      ConditionEnumValue(0x7EA26C2D, "Main Quests Completed"),
      ConditionEnumValue(0x7187A208, "Mauls"),
      ConditionEnumValue(0x98EE55DC, "Misc Objectives Completed"),
      ConditionEnumValue(0xFA06230B, "Most Gold Carried"),
      ConditionEnumValue(0xD37C6909, "Murders"),
      ConditionEnumValue(0x22C2CBD0, "Necks Bitten"),
      ConditionEnumValue(0xBEEBCC87, "Nirnroots Found"),
      ConditionEnumValue(0x56CCFC54, "NumVampirePerks"),
      ConditionEnumValue(0x76A1A5C0, "NumWerewolfPerks"),
      ConditionEnumValue(0xF22A8133, "People Killed"),
      ConditionEnumValue(0x47A78467, "Persuasions"),
      ConditionEnumValue(0xF2BAC234, "Pockets Picked"),
      ConditionEnumValue(0x17C64668, "Poisons Mixed"),
      ConditionEnumValue(0x7D8F2EA6, "Poisons Used"),
      ConditionEnumValue(0x4228DE85, "Potions Mixed"),
      ConditionEnumValue(0x9631EC11, "Potions Used"),
      ConditionEnumValue(0xDE6C73FE, "Questlines Completed"),
      ConditionEnumValue(0x0D7B8B16, "Quests Completed"),
      ConditionEnumValue(0xBB39399E, "Shouts Learned"),
      ConditionEnumValue(0x731B5333, "Shouts Mastered"),
      ConditionEnumValue(0xF921D8BA, "Shouts Unlocked"),
      ConditionEnumValue(0xB1AE4792, "Side Quests Completed"),
      ConditionEnumValue(0xACE470D7, "Skill Books Read"),
      ConditionEnumValue(0xF33130CE, "Skill Increases"),
      ConditionEnumValue(0xB556CC52, "Sneak Attacks"),
      ConditionEnumValue(0x9E8F2530, "Solstheim Locations Discovered"),
      ConditionEnumValue(0xA74CBE83, "Soul Gems Used"),
      ConditionEnumValue(0xC2C9E233, "Souls Trapped"),
      ConditionEnumValue(0x5EC89F1A, "Spells Learned"),
      ConditionEnumValue(0x593E44F8, "StalhrimItemsCrafted"),
      ConditionEnumValue(0xB251A346, "Standing Stones Found"),
      ConditionEnumValue(0x05D45702, "Stores Invested In"),
      ConditionEnumValue(0xD0FE7031, "The Companions Quests Completed"),
      ConditionEnumValue(0x52BA68CB, "The Dark Brotherhood Quests Completed"),
      ConditionEnumValue(0x3E267D77, "The Pale Bounty"),
      ConditionEnumValue(0x69B48177, "The Reach Bounty"),
      ConditionEnumValue(0x50A23F69, "The Rift Bounty"),
      ConditionEnumValue(0x62B2E95D, "Thieves' Guild Quests Completed"),
      ConditionEnumValue(0x944CEA93, "Times Jailed"),
      ConditionEnumValue(0x50AAB633, "Times Shouted"),
      ConditionEnumValue(0x99BB86D8, "Total Lifetime Bounty"),
      ConditionEnumValue(0x4C252391, "Training Sessions"),
      ConditionEnumValue(0x7AEA9C2B, "Trespasses"),
      ConditionEnumValue(0xA67626F4, "Tribal Orcs Bounty"),
      ConditionEnumValue(0x41D4BC0F, "Undead Killed"),
      ConditionEnumValue(0xF39260A1, "Vampirism Cures"),
      ConditionEnumValue(0x61A5C5A9, "Weapons Disarmed"),
      ConditionEnumValue(0x1D3BA844, "Weapons Improved"),
      ConditionEnumValue(0x25F1EA25, "Weapons Made"),
      ConditionEnumValue(0x38A2DD66, "Werewolf Transformations"),
      ConditionEnumValue(0x4231FA4F, "Whiterun Bounty"),
      ConditionEnumValue(0x92565767, "Wings Plucked"),
      ConditionEnumValue(0xC7FC518D, "Winterhold Bounty"),
      ConditionEnumValue(0x949FA7BC, "Words of Power Learned"),
      ConditionEnumValue(0x2C6E3FC0, "Words of Power Unlocked"),
   });
   ConditionArgFormType ObjectReference = ConditionArgFormType("ObjectReference", {
      FormType::Character,
      FormType::Reference,
      FormType::PlacedArrowProjectile,
      FormType::PlacedBarrierProjectile,
      FormType::PlacedBeamProjectile,
      FormType::PlacedConeProjectile,
      FormType::PlacedFlameProjectile,
      FormType::PlacedGrenadeProjectile,
      FormType::PlacedMissileProjectile,
      FormType::PlacedHazard,
   });
   ConditionArgFormType OwnerForm       = ConditionArgFormType("Owner", { FormType::ActorBase, FormType::Faction });
   ConditionArgFormType Package         = ConditionArgFormType("Package", { FormType::Package });
   ConditionArgFormType Perk            = ConditionArgFormType("Perk", { FormType::Perk });
   ConditionArgFormType Quest           = ConditionArgFormType("Quest", { FormType::Quest });
   ConditionArgFormType Race            = ConditionArgFormType("Race", { FormType::Race });
   ConditionArgFormType Region          = ConditionArgFormType("Region", { FormType::Region });
   ConditionArgFormType Scene           = ConditionArgFormType("Scene", { FormType::Scene });
   ConditionArgType     Sex             = ConditionArgType("Axis", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "Male"),
      ConditionEnumValue(1, "Female"),
   });
   ConditionArgFormType Shout           = ConditionArgFormType("Shout", { FormType::Shout });
   ConditionArgFormType Spell           = ConditionArgFormType("Spell", { FormType::Spell });
   ConditionArgType     VATSValueFunction = ConditionArgType("VATS Value Function", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue( 0, "Weapon Is"),
      ConditionEnumValue( 1, "Weapon In List"),
      ConditionEnumValue( 2, "Target Is"),
      ConditionEnumValue( 3, "Target In List"),
      ConditionEnumValue( 4, "Target Distance"),
      ConditionEnumValue( 5, "Target Part"),
      ConditionEnumValue( 6, "VATS Action"),
      ConditionEnumValue( 7, "Is Success"),
      ConditionEnumValue( 8, "Is Critical"),
      ConditionEnumValue( 9, "Critical Effect Is"),
      ConditionEnumValue(10, "Critical Effect In List"),
      ConditionEnumValue(11, "Is Fatal"),
      ConditionEnumValue(12, "Explode Part"),
      ConditionEnumValue(13, "Dismember Part"),
      ConditionEnumValue(14, "Cripple Part"),
      ConditionEnumValue(15, "Weapon Type Is"),
      ConditionEnumValue(16, "Is Stranger"),
      ConditionEnumValue(17, "Is Paralyzing Palm"),
      ConditionEnumValue(18, "Projectile Type Is"),
      ConditionEnumValue(19, "Delivery Type Is"),
      ConditionEnumValue(20, "Casting Type Is"),
   });
   ConditionArgFormType Voicetype       = ConditionArgFormType("Voicetype", { FormType::Voicetype });
   ConditionArgType     WardState       = ConditionArgType("Ward State", ConditionArgUnderlyingType::int_signed, {
      ConditionEnumValue(0, "None"),
      ConditionEnumValue(1, "Absorb"),
      ConditionEnumValue(2, "Break"),
   });
   ConditionArgFormType Weather         = ConditionArgFormType("Weather", { FormType::Weather });
   ConditionArgFormType Worldspace      = ConditionArgFormType("Worldspace", { FormType::Worldspace });
}