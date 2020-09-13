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
namespace dovah::data {
   actor_value_info::actor_value_info(uint32_t index, const char* n) : index(index), name(n) {
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

   actor_Value_info_list::actor_Value_info_list() {
      actor_value_info data[] = {
         // Group 0: FormID = (index - 24) + 1000
         actor_value_info( 24, 0x3E8, "Health"),
         actor_value_info( 25, 0x3E9, "Magicka"),
         actor_value_info( 26, 0x3EA, "Stamina"),
         actor_value_info( 27, 0x3EB, "HealRate"),
         actor_value_info( 28, 0x3EC, "MagickaRate"),
         actor_value_info( 29, 0x3ED, "StaminaRate"),
         actor_value_info( 30, 0x3EE, "SpeedMult"),
         actor_value_info( 31, 0x3EF, "InventoryWeight"),
         actor_value_info( 32, 0x3F0, "CarryWeight"),
         actor_value_info( 33, 0x3F1, "CritChance"),
         actor_value_info( 34, 0x3F2, "MeleeDamage"),
         actor_value_info( 35, 0x3F3, "UnarmedDamage"),
         actor_value_info( 36, 0x3F4, "Mass"),
         actor_value_info( 37, 0x3F5, "VoicePoints"),
         actor_value_info( 38, 0x3F6, "VoiceRate"),
         // Group 1: FormID = (index - 6) + 1100
         actor_value_info(  6, 0x44C, "OneHanded"),
         actor_value_info(  7, 0x44D, "TwoHanded"),
         actor_value_info(  8, 0x44E, "Marksman"),
         actor_value_info(  9, 0x44F, "Block"),
         actor_value_info( 10, 0x450, "Smithing"),
         actor_value_info( 11, 0x451, "HeavyArmor"),
         actor_value_info( 12, 0x452, "LightArmor"),
         actor_value_info( 13, 0x453, "Pickpocket"),
         actor_value_info( 14, 0x454, "Lockpicking"),
         actor_value_info( 15, 0x455, "Sneak"),
         actor_value_info( 16, 0x456, "Alchemy"),
         actor_value_info( 17, 0x457, "Speechcraft"),
         actor_value_info( 18, 0x458, "Alteration"),
         actor_value_info( 19, 0x459, "Conjuration"),
         actor_value_info( 20, 0x45A, "Destruction"),
         actor_value_info( 21, 0x45B, "Illusion"),
         actor_value_info( 22, 0x45C, "Restoration"),
         actor_value_info( 23, 0x45D, "Enchanting"),
         actor_value_info(  0, 0x4B0, "Aggression"),
         actor_value_info(  1, 0x4B1, "Confidence"),
         actor_value_info(  2, 0x4B2, "Energy"),
         actor_value_info(  3, 0x4B3, "Morality"),
         actor_value_info(  4, 0x4B4, "Mood"),
         actor_value_info(  5, 0x4B5, "Assistance"),
         actor_value_info( 39, 0x5CE, "DamageResist"),
         actor_value_info( 40, 0x5CF, "PoisonResist"),
         actor_value_info( 41, 0x5D0, "FireResist"),
         actor_value_info( 42, 0x5D1, "ElectricResist"),
         actor_value_info( 43, 0x5D2, "FrostResist"),
         actor_value_info( 44, 0x5D3, "MagicResist"),
         actor_value_info( 45, 0x5D4, "DiseaseResist"),
         actor_value_info( 46, 0x5D5, "PerceptionCondition"),
         actor_value_info( 47, 0x5D6, "EnduranceCondition"),
         actor_value_info( 48, 0x5D7, "LeftAttackCondition"),
         actor_value_info( 49, 0x5D8, "RightAttackCondition"),
         actor_value_info( 50, 0x5D9, "LeftMobilityCondition"),
         actor_value_info( 51, 0x5DA, "RightMobilityCondition"),
         actor_value_info( 52, 0x5DB, "BrainCondition"),
         actor_value_info( 53, 0x5DC, "Paralysis"),
         actor_value_info( 54, 0x5DD, "Invisibility"),
         actor_value_info( 55, 0x5DE, "NightEye"),
         actor_value_info( 56, 0x5DF, "DetectLifeRange"),
         actor_value_info( 57, 0x5E0, "WaterBreathing"),
         actor_value_info( 58, 0x5E1, "WaterWalking"),
         actor_value_info( 59, 0x5E2, "IgnoreCrippledLimbs"),
         actor_value_info( 60, 0x5E3, "Fame"),
         actor_value_info( 61, 0x5E4, "Infamy"),
         actor_value_info( 62, 0x5E5, "JumpingBonus"),
         actor_value_info( 63, 0x5E6, "WardPower"),
         actor_value_info( 64, 0x5E7, "RightItemCharge"),
         actor_value_info( 65, 0x5E8, "ArmorPerks"),
         actor_value_info( 66, 0x5E9, "ShieldPerks"),
         actor_value_info( 67, 0x5EA, "WardDeflection"),
         actor_value_info( 68, 0x5EB, "Variable01"),
         actor_value_info( 69, 0x5EC, "Variable02"),
         actor_value_info( 70, 0x5ED, "Variable03"),
         actor_value_info( 71, 0x5EE, "Variable04"),
         actor_value_info( 72, 0x5EF, "Variable05"),
         actor_value_info( 73, 0x5F0, "Variable06"),
         actor_value_info( 74, 0x5F1, "Variable07"),
         actor_value_info( 75, 0x5F2, "Variable08"),
         actor_value_info( 76, 0x5F3, "Variable09"),
         actor_value_info( 77, 0x5F4, "Variable10"),
         actor_value_info( 78, 0x5F5, "BowSpeedBonus"),
         actor_value_info( 79, 0x5F6, "FavorActive"),
         actor_value_info( 80, 0x5F7, "FavorsPerDay"),
         actor_value_info( 81, 0x5F8, "FavorsPerDayTimer"),
         actor_value_info( 82, 0x5F9, "LeftItemCharge"),
         actor_value_info( 83, 0x5FA, "AbsorbChance"),
         actor_value_info( 84, 0x5FB, "Blindness"),
         actor_value_info( 85, 0x5FC, "WeaponSpeedMult"),
         actor_value_info( 86, 0x5FD, "ShoutRecoveryMult"),
         actor_value_info( 87, 0x5FE, "BowStaggerBonus"),
         actor_value_info( 88, 0x5FF, "Telekinesis"),
         actor_value_info( 89, 0x600, "FavorPointsBonus"),
         actor_value_info( 90, 0x601, "LastBribedIntimidated"),
         actor_value_info( 91, 0x602, "LastFlattered"),
         actor_value_info( 92, 0x603, "MovementNoiseMult"),
         actor_value_info( 93, 0x604, "BypassVendorStolenCheck"),
         actor_value_info( 94, 0x605, "BypassVendorKeywordCheck"),
         actor_value_info( 95, 0x606, "WaitingForPlayer"),
         actor_value_info( 96, 0x607, "OneHandedMod"),
         actor_value_info( 97, 0x608, "TwoHandedMod"),
         actor_value_info( 98, 0x609, "MarksmanMod"),
         actor_value_info( 99, 0x60A, "BlockMod"),
         actor_value_info(100, 0x60B, "SmithingMod"),
         actor_value_info(101, 0x60C, "HeavyArmorMod"),
         actor_value_info(102, 0x60D, "LightArmorMod"),
         actor_value_info(103, 0x60E, "PickPocketMod"),
         actor_value_info(104, 0x60F, "LockpickingMod"),
         actor_value_info(105, 0x610, "SneakMod"),
         actor_value_info(106, 0x611, "AlchemyMod"),
         actor_value_info(107, 0x612, "SpeechcraftMod"),
         actor_value_info(108, 0x613, "AlterationMod"),
         actor_value_info(109, 0x614, "ConjurationMod"),
         actor_value_info(110, 0x615, "DestructionMod"),
         actor_value_info(111, 0x616, "IllusionMod"),
         actor_value_info(112, 0x617, "RestorationMod"),
         actor_value_info(113, 0x618, "EnchantingMod"),
         actor_value_info(114, 0x619, "OneHandedSkillAdvance"),
         actor_value_info(115, 0x61A, "TwoHandedSkillAdvance"),
         actor_value_info(116, 0x61B, "MarksmanSkillAdvance"),
         actor_value_info(117, 0x61C, "BlockSkillAdvance"),
         actor_value_info(118, 0x61D, "SmithingSkillAdvance"),
         actor_value_info(119, 0x61E, "HeavyArmorSkillAdvance"),
         actor_value_info(120, 0x61F, "LightArmorSkillAdvance"),
         actor_value_info(121, 0x620, "PickPocketSkillAdvance"),
         actor_value_info(122, 0x621, "LockpickingSkillAdvance"),
         actor_value_info(123, 0x622, "SneakSkillAdvance"),
         actor_value_info(124, 0x623, "AlchemySkillAdvance"),
         actor_value_info(125, 0x624, "SpeechcraftSkillAdvance"),
         actor_value_info(126, 0x625, "AlterationSkillAdvance"),
         actor_value_info(127, 0x626, "ConjurationSkillAdvance"),
         actor_value_info(128, 0x627, "DestructionSkillAdvance"),
         actor_value_info(129, 0x628, "IllusionSkillAdvance"),
         actor_value_info(130, 0x629, "RestorationSkillAdvance"),
         actor_value_info(131, 0x62A, "EnchantingSkillAdvance"),
         actor_value_info(132, 0x62B, "LeftWeaponSpeedMult"),
         actor_value_info(133, 0x62C, "DragonSouls"),
         actor_value_info(134, 0x62D, "CombatHealthRegenMult"),
         actor_value_info(135, 0x62E, "OneHandedPowerMod"),
         actor_value_info(136, 0x62F, "TwoHandedPowerMod"),
         actor_value_info(137, 0x630, "MarksmanPowerMod"),
         actor_value_info(138, 0x631, "BlockPowerMod"),
         actor_value_info(139, 0x632, "SmithingPowerMod"),
         actor_value_info(140, 0x633, "HeavyArmorPowerMod"),
         actor_value_info(141, 0x634, "LightArmorPowerMod"),
         actor_value_info(142, 0x635, "PickPocketPowerMod"),
         actor_value_info(143, 0x636, "LockpickingPowerMod"),
         actor_value_info(144, 0x637, "SneakPowerMod"),
         actor_value_info(145, 0x638, "AlchemyPowerMod"),
         actor_value_info(146, 0x639, "SpeechcraftPowerMod"),
         actor_value_info(147, 0x63A, "AlterationPowerMod"),
         actor_value_info(148, 0x63B, "ConjurationPowerMod"),
         actor_value_info(149, 0x63C, "DestructionPowerMod"),
         actor_value_info(150, 0x63D, "IllusionPowerMod"),
         actor_value_info(151, 0x63E, "RestorationPowerMod"),
         actor_value_info(152, 0x63F, "EnchantingPowerMod"),
         actor_value_info(153, 0x640, "DragonRend"),
         actor_value_info(154, 0x641, "AttackDamageMult"),
         actor_value_info(155, 0x642, "HealRateMult"),
         actor_value_info(156, 0x643, "MagickaRateMult"),
         actor_value_info(157, 0x644, "StaminaRateMult"),
         actor_value_info(158, 0x645, "WerewolfPerks"),
         actor_value_info(160, 0x647, "GrabActorOffset"),
         actor_value_info(161, 0x648, "Grabbed"),
         actor_value_info(162, 0x649, "DEPRECATED05"),
         actor_value_info(163, 0x64A, "ReflectDamage"),
         actor_value_info(159, 0x646, "VampirePerks"),
      };
      this->list  = (actor_value_info*) malloc(sizeof(data));
      memcpy(this->list, data, sizeof(data));
      this->count = std::extent<decltype(data)>::value;
   }
   actor_Value_info_list::~actor_Value_info_list() {
      if (this->list)
         free(this->list);
   }
}