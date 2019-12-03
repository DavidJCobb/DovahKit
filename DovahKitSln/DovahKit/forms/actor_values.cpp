#include "actor_values.h"
#include <cassert>
#include <string>

namespace {
   struct _range {
      uint8_t min; // AV index (first to fall in this range)
      uint8_t max; // AV index (last to fall in this range)
   };
   _range _ranges[] = {
      { 24, 37 }, // attributes; attribute heal rates; gameplay-critical stats e.g. speed and carry capacity
      {  6, 23 }, // skills
      {  0,  5 }, // AI values
      { 39, 45 }, // damage resistances
      { 46, 52 }, // limb condition
      { 53, 159 }, // all other actor values
   };
}
ActorValueInfo::ActorValueInfo(uint32_t index, const char* n) : index(index), name(n) {
   //
   // The form ID can be computed from the actor value index. The game uses the same ranges 
   // that we do here, and the same formula once it knows that the AV falls into a range; 
   // it just uses hardcoded bounds checks instead of a loop. See the Skyrim Classic sub-
   // routine at 0x005AD160, though its helper function at 0x005ACFD0 may be easier to read.
   //
   // 0x005AD160: uint32_t ActorValueIndexToFormID(uint32_t index);
   //
   //              - Identifies the range index (i) that the AV falls into.
   //              - Calls _ActorValueOffsetWithinRange to get the offset (o).
   //              - Returns (i + 10) * 100 + o;
   //
   // 0x005ACFD0: uint32_t _ActorValueOffsetWithinRange(uint32_t range, uint32_t index);
   //
   //              - Subtracts from index the minimum end of the range. So for example, 
   //                range 0's minimum is 24, so if the (range) argument is 0, then this 
   //                returns (index - 24).
   //
   //              - This function is easier to read than the other one in my opinion, 
   //                so you can quickly read it to identify all of the ranges and their 
   //                minimums.
   //
   int range = std::extent<decltype(_ranges)>::value;
   int i     = 0;
   for(; i < std::extent<decltype(_ranges)>::value; i++)
      if (_ranges[i].min >= index && _ranges[i].max <= index) {
         range = i;
         break;
      }
   assert(range < std::extent<decltype(_ranges)>::value && "Unable to convert AV index to hardcoded form ID.");
   this->formID = index - _ranges[i].min + (10 + i) * 100;
};

ActorValueInfoList::ActorValueInfoList() {
   ActorValueInfo data[] = {
      // Group 0: FormID = (index - 24) + 1000
      ActorValueInfo( 24, 0x3E8, "Health"),
      ActorValueInfo( 25, 0x3E9, "Magicka"),
      ActorValueInfo( 26, 0x3EA, "Stamina"),
      ActorValueInfo( 27, 0x3EB, "HealRate"),
      ActorValueInfo( 28, 0x3EC, "MagickaRate"),
      ActorValueInfo( 29, 0x3ED, "StaminaRate"),
      ActorValueInfo( 30, 0x3EE, "SpeedMult"),
      ActorValueInfo( 31, 0x3EF, "InventoryWeight"),
      ActorValueInfo( 32, 0x3F0, "CarryWeight"),
      ActorValueInfo( 33, 0x3F1, "CritChance"),
      ActorValueInfo( 34, 0x3F2, "MeleeDamage"),
      ActorValueInfo( 35, 0x3F3, "UnarmedDamage"),
      ActorValueInfo( 36, 0x3F4, "Mass"),
      ActorValueInfo( 37, 0x3F5, "VoicePoints"),
      ActorValueInfo( 38, 0x3F6, "VoiceRate"),
      // Group 1: FormID = (index - 6) + 1100
      ActorValueInfo(  6, 0x44C, "OneHanded"),
      ActorValueInfo(  7, 0x44D, "TwoHanded"),
      ActorValueInfo(  8, 0x44E, "Marksman"),
      ActorValueInfo(  9, 0x44F, "Block"),
      ActorValueInfo( 10, 0x450, "Smithing"),
      ActorValueInfo( 11, 0x451, "HeavyArmor"),
      ActorValueInfo( 12, 0x452, "LightArmor"),
      ActorValueInfo( 13, 0x453, "Pickpocket"),
      ActorValueInfo( 14, 0x454, "Lockpicking"),
      ActorValueInfo( 15, 0x455, "Sneak"),
      ActorValueInfo( 16, 0x456, "Alchemy"),
      ActorValueInfo( 17, 0x457, "Speechcraft"),
      ActorValueInfo( 18, 0x458, "Alteration"),
      ActorValueInfo( 19, 0x459, "Conjuration"),
      ActorValueInfo( 20, 0x45A, "Destruction"),
      ActorValueInfo( 21, 0x45B, "Illusion"),
      ActorValueInfo( 22, 0x45C, "Restoration"),
      ActorValueInfo( 23, 0x45D, "Enchanting"),
      ActorValueInfo(  0, 0x4B0, "Aggression"),
      ActorValueInfo(  1, 0x4B1, "Confidence"),
      ActorValueInfo(  2, 0x4B2, "Energy"),
      ActorValueInfo(  3, 0x4B3, "Morality"),
      ActorValueInfo(  4, 0x4B4, "Mood"),
      ActorValueInfo(  5, 0x4B5, "Assistance"),
      ActorValueInfo( 39, 0x5CE, "DamageResist"),
      ActorValueInfo( 40, 0x5CF, "PoisonResist"),
      ActorValueInfo( 41, 0x5D0, "FireResist"),
      ActorValueInfo( 42, 0x5D1, "ElectricResist"),
      ActorValueInfo( 43, 0x5D2, "FrostResist"),
      ActorValueInfo( 44, 0x5D3, "MagicResist"),
      ActorValueInfo( 45, 0x5D4, "DiseaseResist"),
      ActorValueInfo( 46, 0x5D5, "PerceptionCondition"),
      ActorValueInfo( 47, 0x5D6, "EnduranceCondition"),
      ActorValueInfo( 48, 0x5D7, "LeftAttackCondition"),
      ActorValueInfo( 49, 0x5D8, "RightAttackCondition"),
      ActorValueInfo( 50, 0x5D9, "LeftMobilityCondition"),
      ActorValueInfo( 51, 0x5DA, "RightMobilityCondition"),
      ActorValueInfo( 52, 0x5DB, "BrainCondition"),
      ActorValueInfo( 53, 0x5DC, "Paralysis"),
      ActorValueInfo( 54, 0x5DD, "Invisibility"),
      ActorValueInfo( 55, 0x5DE, "NightEye"),
      ActorValueInfo( 56, 0x5DF, "DetectLifeRange"),
      ActorValueInfo( 57, 0x5E0, "WaterBreathing"),
      ActorValueInfo( 58, 0x5E1, "WaterWalking"),
      ActorValueInfo( 59, 0x5E2, "IgnoreCrippledLimbs"),
      ActorValueInfo( 60, 0x5E3, "Fame"),
      ActorValueInfo( 61, 0x5E4, "Infamy"),
      ActorValueInfo( 62, 0x5E5, "JumpingBonus"),
      ActorValueInfo( 63, 0x5E6, "WardPower"),
      ActorValueInfo( 64, 0x5E7, "RightItemCharge"),
      ActorValueInfo( 65, 0x5E8, "ArmorPerks"),
      ActorValueInfo( 66, 0x5E9, "ShieldPerks"),
      ActorValueInfo( 67, 0x5EA, "WardDeflection"),
      ActorValueInfo( 68, 0x5EB, "Variable01"),
      ActorValueInfo( 69, 0x5EC, "Variable02"),
      ActorValueInfo( 70, 0x5ED, "Variable03"),
      ActorValueInfo( 71, 0x5EE, "Variable04"),
      ActorValueInfo( 72, 0x5EF, "Variable05"),
      ActorValueInfo( 73, 0x5F0, "Variable06"),
      ActorValueInfo( 74, 0x5F1, "Variable07"),
      ActorValueInfo( 75, 0x5F2, "Variable08"),
      ActorValueInfo( 76, 0x5F3, "Variable09"),
      ActorValueInfo( 77, 0x5F4, "Variable10"),
      ActorValueInfo( 78, 0x5F5, "BowSpeedBonus"),
      ActorValueInfo( 79, 0x5F6, "FavorActive"),
      ActorValueInfo( 80, 0x5F7, "FavorsPerDay"),
      ActorValueInfo( 81, 0x5F8, "FavorsPerDayTimer"),
      ActorValueInfo( 82, 0x5F9, "LeftItemCharge"),
      ActorValueInfo( 83, 0x5FA, "AbsorbChance"),
      ActorValueInfo( 84, 0x5FB, "Blindness"),
      ActorValueInfo( 85, 0x5FC, "WeaponSpeedMult"),
      ActorValueInfo( 86, 0x5FD, "ShoutRecoveryMult"),
      ActorValueInfo( 87, 0x5FE, "BowStaggerBonus"),
      ActorValueInfo( 88, 0x5FF, "Telekinesis"),
      ActorValueInfo( 89, 0x600, "FavorPointsBonus"),
      ActorValueInfo( 90, 0x601, "LastBribedIntimidated"),
      ActorValueInfo( 91, 0x602, "LastFlattered"),
      ActorValueInfo( 92, 0x603, "MovementNoiseMult"),
      ActorValueInfo( 93, 0x604, "BypassVendorStolenCheck"),
      ActorValueInfo( 94, 0x605, "BypassVendorKeywordCheck"),
      ActorValueInfo( 95, 0x606, "WaitingForPlayer"),
      ActorValueInfo( 96, 0x607, "OneHandedMod"),
      ActorValueInfo( 97, 0x608, "TwoHandedMod"),
      ActorValueInfo( 98, 0x609, "MarksmanMod"),
      ActorValueInfo( 99, 0x60A, "BlockMod"),
      ActorValueInfo(100, 0x60B, "SmithingMod"),
      ActorValueInfo(101, 0x60C, "HeavyArmorMod"),
      ActorValueInfo(102, 0x60D, "LightArmorMod"),
      ActorValueInfo(103, 0x60E, "PickPocketMod"),
      ActorValueInfo(104, 0x60F, "LockpickingMod"),
      ActorValueInfo(105, 0x610, "SneakMod"),
      ActorValueInfo(106, 0x611, "AlchemyMod"),
      ActorValueInfo(107, 0x612, "SpeechcraftMod"),
      ActorValueInfo(108, 0x613, "AlterationMod"),
      ActorValueInfo(109, 0x614, "ConjurationMod"),
      ActorValueInfo(110, 0x615, "DestructionMod"),
      ActorValueInfo(111, 0x616, "IllusionMod"),
      ActorValueInfo(112, 0x617, "RestorationMod"),
      ActorValueInfo(113, 0x618, "EnchantingMod"),
      ActorValueInfo(114, 0x619, "OneHandedSkillAdvance"),
      ActorValueInfo(115, 0x61A, "TwoHandedSkillAdvance"),
      ActorValueInfo(116, 0x61B, "MarksmanSkillAdvance"),
      ActorValueInfo(117, 0x61C, "BlockSkillAdvance"),
      ActorValueInfo(118, 0x61D, "SmithingSkillAdvance"),
      ActorValueInfo(119, 0x61E, "HeavyArmorSkillAdvance"),
      ActorValueInfo(120, 0x61F, "LightArmorSkillAdvance"),
      ActorValueInfo(121, 0x620, "PickPocketSkillAdvance"),
      ActorValueInfo(122, 0x621, "LockpickingSkillAdvance"),
      ActorValueInfo(123, 0x622, "SneakSkillAdvance"),
      ActorValueInfo(124, 0x623, "AlchemySkillAdvance"),
      ActorValueInfo(125, 0x624, "SpeechcraftSkillAdvance"),
      ActorValueInfo(126, 0x625, "AlterationSkillAdvance"),
      ActorValueInfo(127, 0x626, "ConjurationSkillAdvance"),
      ActorValueInfo(128, 0x627, "DestructionSkillAdvance"),
      ActorValueInfo(129, 0x628, "IllusionSkillAdvance"),
      ActorValueInfo(130, 0x629, "RestorationSkillAdvance"),
      ActorValueInfo(131, 0x62A, "EnchantingSkillAdvance"),
      ActorValueInfo(132, 0x62B, "LeftWeaponSpeedMult"),
      ActorValueInfo(133, 0x62C, "DragonSouls"),
      ActorValueInfo(134, 0x62D, "CombatHealthRegenMult"),
      ActorValueInfo(135, 0x62E, "OneHandedPowerMod"),
      ActorValueInfo(136, 0x62F, "TwoHandedPowerMod"),
      ActorValueInfo(137, 0x630, "MarksmanPowerMod"),
      ActorValueInfo(138, 0x631, "BlockPowerMod"),
      ActorValueInfo(139, 0x632, "SmithingPowerMod"),
      ActorValueInfo(140, 0x633, "HeavyArmorPowerMod"),
      ActorValueInfo(141, 0x634, "LightArmorPowerMod"),
      ActorValueInfo(142, 0x635, "PickPocketPowerMod"),
      ActorValueInfo(143, 0x636, "LockpickingPowerMod"),
      ActorValueInfo(144, 0x637, "SneakPowerMod"),
      ActorValueInfo(145, 0x638, "AlchemyPowerMod"),
      ActorValueInfo(146, 0x639, "SpeechcraftPowerMod"),
      ActorValueInfo(147, 0x63A, "AlterationPowerMod"),
      ActorValueInfo(148, 0x63B, "ConjurationPowerMod"),
      ActorValueInfo(149, 0x63C, "DestructionPowerMod"),
      ActorValueInfo(150, 0x63D, "IllusionPowerMod"),
      ActorValueInfo(151, 0x63E, "RestorationPowerMod"),
      ActorValueInfo(152, 0x63F, "EnchantingPowerMod"),
      ActorValueInfo(153, 0x640, "DragonRend"),
      ActorValueInfo(154, 0x641, "AttackDamageMult"),
      ActorValueInfo(155, 0x642, "HealRateMult"),
      ActorValueInfo(156, 0x643, "MagickaRateMult"),
      ActorValueInfo(157, 0x644, "StaminaRateMult"),
      ActorValueInfo(158, 0x645, "WerewolfPerks"),
      ActorValueInfo(160, 0x647, "GrabActorOffset"),
      ActorValueInfo(161, 0x648, "Grabbed"),
      ActorValueInfo(162, 0x649, "DEPRECATED05"),
      ActorValueInfo(163, 0x64A, "ReflectDamage"),
      ActorValueInfo(159, 0x646, "VampirePerks"),
   };
   this->list  = (ActorValueInfo*) malloc(sizeof(data));
   memcpy(this->list, data, sizeof(data));
   this->count = std::extent<decltype(data)>::value;
}
ActorValueInfoList::~ActorValueInfoList() {
   if (this->list)
      free(this->list);
}