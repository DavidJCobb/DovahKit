#include "conditions.h"
#include "../esp/TESPlugin.h"
#include "../helpers/strings.h"

bool Condition::read(TESPluginRecord& record) {
   auto& subrecord = record.get_current_subrecord();
   assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
   if (!subrecord.is_in_bounds(0x14))
      return false;
   subrecord.unchecked_read(this->type);
   subrecord.skip_bytes(3);
   if (this->get_flags() & ConditionTypeFlags::compare_to_global)
      subrecord.unchecked_read(this->compareToGlobalID);
   else
      subrecord.unchecked_read(this->compareToConstant);
   subrecord.unchecked_read(this->function);
   subrecord.skip_bytes(2);
   subrecord.unchecked_read(this->parameter1);
   subrecord.unchecked_read(this->parameter2);
   if (false) { // TODO: for GetEventData only
      if (!subrecord.is_in_bounds(8))
         return false;
      subrecord.unchecked_read(this->eventFunction);
      subrecord.unchecked_read(this->eventMember);
      subrecord.unchecked_read(this->eventFormID);
   } else {
      if (!subrecord.is_in_bounds(12))
         return false;
      subrecord.unchecked_read(this->runOn);
      subrecord.unchecked_read(this->reference);
      subrecord.unchecked_read(this->parameter3);
   }
   auto next = record.peek_next_subrecord_type();
   if (next != 'CIS1' && next != 'CIS2')
      return true;
   if (next == 'CIS1') {
      auto& sub = record.next_subrecord();
      sub.to_string(this->stringParam1);
      //
      next = record.peek_next_subrecord_type();
   }
   if (next == 'CIS2') {
      auto& sub = record.next_subrecord();
      sub.to_string(this->stringParam2);
   }
   return true;
}

void Condition::to_string(std::string& out) const {
   out.clear();
   //
   auto function = getConditionFunction(this->function);
   if (!function) {
      cobb::sprintf(out, "<BAD FUNCTION ID %04X>", this->function);
      return;
   }
   switch (this->runOn) {
      case ConditionRunOn::subject:
         out += "Subject";
         break;
      case ConditionRunOn::target:
         out += "Target";
         break;
      case ConditionRunOn::reference:
         cobb::sprintf(out, "%08X", this->reference);
         break;
      case ConditionRunOn::combat_target:
         out += "CombatTarget";
         break;
      case ConditionRunOn::event_data:
         out += "EventData";
         break;
      case ConditionRunOn::quest_alias:
         out += "QuestAlias";
         break;
      case ConditionRunOn::package_data:
         out += "PackageData";
         break;
      default:
         out += "?????";
   }
   out += '.';
   out += function->name;
   out += '(';
   for (uint32_t i = 0; i < 3; i++) {
      auto& arg = function->paramTypes[i];
      if (arg != ConditionParamType::None) {
         if (i > 0)
            out += ", ";
         out += "<TODO: FORMAT ARGS>";
      }
   }
   out += ") ";
   switch (this->get_operator()) {
      case ConditionOperator::equal:
         out += "==";
         break;
      case ConditionOperator::not_equal:
         out += "!=";
         break;
      case ConditionOperator::greater:
         out += "> ";
         break;
      case ConditionOperator::greater_or_equal:
         out += ">=";
         break;
      case ConditionOperator::less:
         out += "< ";
         break;
      case ConditionOperator::less_or_equal:
         out += "<=";
         break;
      default:
         out += "??";
   }
   out += ' ';
   if (this->get_flags() & ConditionTypeFlags::compare_to_global) {
      std::string glob;
      cobb::sprintf(glob, "[GLOB:%08X]", this->compareToGlobalID);
      out += glob;
   } else {
      out += std::to_string(this->compareToConstant);
   }
}

ConditionFunction conditionFunctions[] = {
   ConditionFunction(  0, "GetWantBlocking",   ""),
   ConditionFunction(  1, "GetDistance",       "Returns the distance between this reference and another reference.", ConditionParamType::ObjectReference),
   ConditionFunction(  2, ConditionFunction::dummy), // actually AddItem
   ConditionFunction(  3, ConditionFunction::dummy), // actually SetEssential
   ConditionFunction(  4, ConditionFunction::dummy), // actually Rotate
   ConditionFunction(  5, "GetLocked",         "Returns 1 if this reference is a locked door or container, or 0 otherwise."),
   ConditionFunction(  6, "GetPos",            "Returns this reference's position along the given axis.", ConditionParamType::Axis),
   ConditionFunction(  7, ConditionFunction::dummy), // actually SetPos
   ConditionFunction(  8, "GetAngle",          "Returns this reference's rotation along the given axis.", ConditionParamType::Axis),
   ConditionFunction(  9, ConditionFunction::dummy), // actually SetANgle
   ConditionFunction( 10, "GetStartingPos",    "Returns this reference's starting position along the given axis.", ConditionParamType::Axis),
   ConditionFunction( 11, "GetStartingAngle",  "Returns this reference's starting position along the given axis.", ConditionParamType::Axis),
   ConditionFunction( 12, "GetSecondsPassed",  ""),
   ConditionFunction( 13, ConditionFunction::dummy), // actually Activate
   ConditionFunction( 14, "GetActorValue",     "Returns the current value of the specified ActorValue.", ConditionParamType::ActorValue),
   ConditionFunction( 15, ConditionFunction::dummy), // actually SetActorValue
   ConditionFunction( 16, ConditionFunction::dummy), // actually ModActorValue
   ConditionFunction( 17, ConditionFunction::dummy), // actually SetAtStart
   ConditionFunction( 18, "GetCurrentTime",    ""),
   ConditionFunction( 19, ConditionFunction::dummy), // actually PlayGroup
   ConditionFunction( 20, ConditionFunction::dummy), // actually LoopGroup
   ConditionFunction( 21, ConditionFunction::dummy), // actually SkipAnim
   ConditionFunction( 22, ConditionFunction::dummy), // actually StartCombat
   ConditionFunction( 23, ConditionFunction::dummy), // actually StopCombat
   ConditionFunction( 24, "GetScale",          "Returns this reference's scale."),
   ConditionFunction( 25, "IsMoving",          ""),
   ConditionFunction( 26, "IsTurning",         ""),
   ConditionFunction( 27, "GetLineOfSight",    "", ConditionParamType::ObjectReference),
   ConditionFunction( 28, ConditionFunction::dummy), // actually AddSpell
   ConditionFunction( 29, ConditionFunction::dummy), // actually RemoveSpell
   ConditionFunction( 30, ConditionFunction::dummy), // actualy Cast
   ConditionFunction( 31, ConditionFunction::dummy), // actually GetButtonPressed
   ConditionFunction( 32, "GetInSameCell",     "Returns 1 if this reference is in the same cell as the specified reference, or 0 otherwise.", ConditionParamType::ObjectReference),
   ConditionFunction( 33, ConditionFunction::dummy), // actually Enable
   ConditionFunction( 34, ConditionFunction::dummy), // actually Disable
   ConditionFunction( 35, "GetDisabled",       "Returns 1 if this reference is disabled, or 0 otherwise."),
   ConditionFunction( 36, "MenuMode",          "", ConditionParamType::Integer),
   ConditionFunction( 37, ConditionFunction::dummy), // actually PlaceAtMe
   ConditionFunction( 38, ConditionFunction::dummy), // actually PlaySound
   ConditionFunction( 39, "GetDisease",        ""),
   ConditionFunction( 40, ConditionFunction::dummy), // actually FailAllObjectives
   ConditionFunction( 41, "GetClothingValue",  ""),
   ConditionFunction( 42, "SameFaction",       "Returns 1 if this reference is in the same faction as the specified actor, or 0 otherwise.", ConditionParamType::Actor),
   ConditionFunction( 43, "SameRace",          "Returns 1 if this reference is of the same race as the specified actor, or 0 otherwise.", ConditionParamType::Actor),
   ConditionFunction( 44, "SameSex",           "Returns 1 if this reference is of the same sex as the specified actor, or 0 otherwise.", ConditionParamType::Actor),
   ConditionFunction( 45, "GetDetected",       "", ConditionParamType::Actor),
   ConditionFunction( 46, "GetDead",           ""),
   ConditionFunction( 47, "GetItemCount",      "Returns how many of the specified form this reference has in its inventory.", ConditionParamType::InventoryItem),
   ConditionFunction( 48, "GetGold",           "Returns how much gold this reference is carrying."),
   ConditionFunction( 49, "GetSleeping",       ""),
   ConditionFunction( 50, "GetTalkedToPC",     ""),
   ConditionFunction( 51, ConditionFunction::dummy), // actually Say
   ConditionFunction( 52, ConditionFunction::dummy), // actually SayTo
   ConditionFunction( 53, "GetScriptVariable",  "", ConditionParamType::ObjectReference, ConditionParamType::ScriptVariableIndex),
   ConditionFunction( 54, ConditionFunction::dummy), // actually StartQuest
   ConditionFunction( 55, ConditionFunction::dummy), // actually StopQuest
   ConditionFunction( 56, "GetQuestRunning",    "Returns 1 if the specified quest is running, or 0 otherwise.", ConditionParamType::Quest),
   ConditionFunction( 57, ConditionFunction::dummy), // actually SetStage
   ConditionFunction( 58, "GetStage",           "Returns the specified quest's current stage number.", ConditionParamType::Quest),
   ConditionFunction( 59, "GetStageDone",       "Returns 1 if the specified quest stage is complete, or 0 otherwise.", ConditionParamType::Quest, ConditionParamType::QuestStage),
   ConditionFunction( 60, "GetFactionRankDifference", "", ConditionParamType::Faction, ConditionParamType::Actor),
   ConditionFunction( 61, "GetAlarmed",         ""),
   ConditionFunction( 62, "IsRaining",          "Returns 1 if the current weather is rainy, or 0 otherwise."),
   ConditionFunction( 63, "GetAttacked",        ""),
   ConditionFunction( 64, "GetIsCreature",      ""),
   ConditionFunction( 65, "GetLockLevel",       ""),
   ConditionFunction( 66, "GetShouldAttack",    "", ConditionParamType::Actor),
   ConditionFunction( 67, "GetInCell",          "Returns 1 if this reference is in the specified cell, or 0 otherwise.", ConditionParamType::Cell),
   ConditionFunction( 68, "GetIsClass",         "Returns 1 if this actor is of the specified class, or 0 otherwise.", ConditionParamType::Class),
   ConditionFunction( 69, "GetIsRace",          "Returns 1 if this actor is of the specified race, or 0 otherwise.",  ConditionParamType::Race),
   ConditionFunction( 70, "GetIsSex",           "Returns 1 if this actor is of the specified sex, or 0 otherwise.",   ConditionParamType::Sex),
   ConditionFunction( 71, "GetInFaction",       "", ConditionParamType::Faction),
   ConditionFunction( 72, "GetIsID",            "", ConditionParamType::BaseForm),
   ConditionFunction( 73, "GetFactionRank",     "", ConditionParamType::Faction),
   ConditionFunction( 74, "GetGlobalValue",     "Returns the value of the specified Global.", ConditionParamType::Global),
   ConditionFunction( 75, "IsSnowing",          "Returns 1 if the current weather is snowy, or 0 otherwise."),
   ConditionFunction( 76, ConditionFunction::dummy), // actually FastTravel
   ConditionFunction( 77, "GetRandomPercent",   "Returns a random number between 0 and 100, inclusive."),
   ConditionFunction( 78, ConditionFunction::dummy), // actually RemoveMusic
   ConditionFunction( 79, "GetQuestVariable",   "", ConditionParamType::Quest, ConditionParamType::ScriptVariableIndex),
   ConditionFunction( 80, "GetLevel",           "Returns this actor's level."),
   ConditionFunction( 81, "IsRotating",         ""),
   ConditionFunction( 82, ConditionFunction::dummy), // actually RemoveItem
   ConditionFunction( 83, ConditionFunction::dummy), // actually GetLeveledEncounterValue
   ConditionFunction( 84, "GetDeadCount",       "Returns the number of times this character has died.", ConditionParamType::ActorBase),
   ConditionFunction( 85, ConditionFunction::dummy), // actually AddToMap
   ConditionFunction( 86, ConditionFunction::dummy), // actually StartConversation
   ConditionFunction( 87, ConditionFunction::dummy), // actually Drop
   ConditionFunction( 88, ConditionFunction::dummy), // actually AddTopic
   ConditionFunction( 89, ConditionFunction::dummy), // actually ShowMessage
   ConditionFunction( 90, ConditionFunction::dummy), // actually SetAlert
   ConditionFunction( 91, "GetIsAlerted", ""),
   ConditionFunction( 92, ConditionFunction::dummy), // actually Look
   ConditionFunction( 93, ConditionFunction::dummy), // actually StopLook
   ConditionFunction( 94, ConditionFunction::dummy), // actually EvaluatePackage
   ConditionFunction( 95, ConditionFunction::dummy), // actually SendAssaultAlarm
   ConditionFunction( 96, ConditionFunction::dummy), // actually EnablePlayerControls
   ConditionFunction( 97, ConditionFunction::dummy), // actually DisablePlayerControls
   ConditionFunction( 98, "GetPlayerControlsDisabled", "", ConditionParamType::Integer, ConditionParamType::Integer),
   ConditionFunction( 99, "GetHeadingAngle", "", ConditionParamType::ObjectReference),
   ConditionFunction(100, ConditionFunction::dummy), // actually PickIdle
   ConditionFunction(101, "IsWeaponMagicOut", ""),
   ConditionFunction(102, "IsTorchOut", ""),
   ConditionFunction(103, "IsShieldOut", ""),
   ConditionFunction(104, ConditionFunction::dummy), // actually CreateDetectionEvent
   ConditionFunction(105, ConditionFunction::dummy), // actually IsActionRef
   ConditionFunction(106, "IsFacingUp", ""),
   ConditionFunction(107, "GetKnockedState", ""),
   ConditionFunction(108, "GetWeaponAnimType", ""),
   ConditionFunction(109, "IsWeaponSkillType", "", ConditionParamType::ActorValue),
   ConditionFunction(110, "GetCurrentAIPackage", ""),
   ConditionFunction(111, "IsWaiting", ""),
   ConditionFunction(112, "IsIdlePlaying", ""),
   ConditionFunction(113, ConditionFunction::dummy), // actually CompleteQuest
   ConditionFunction(114, ConditionFunction::dummy), // actually Lock
   ConditionFunction(115, ConditionFunction::dummy), // actually Unlock
   ConditionFunction(116, "IsIntimidatedByPlayer", ""),
   ConditionFunction(117, "IsPlayerInRegion", ""),
   ConditionFunction(118, "GetActorAggroRadiusViolated", ""),
   ConditionFunction(119, ConditionFunction::dummy),
   ConditionFunction(120, ConditionFunction::dummy),
   ConditionFunction(121, ConditionFunction::dummy),
   ConditionFunction(122, "GetCrime", "", ConditionParamType::Actor, ConditionParamType::CrimeType),
   ConditionFunction(123, "IsGreetingPlayer", ""),
   ConditionFunction(124, ConditionFunction::dummy),
   ConditionFunction(125, "IsGuard", ""),
   ConditionFunction(126, ConditionFunction::dummy),
   ConditionFunction(127, "HasBeenEaten", "Returns 1 if this actor has been fed on by a cannibal or werewolf, or 0 otherwise."),
   ConditionFunction(128, "GetStaminaPercentage", ""),
   ConditionFunction(129, "GetPCIsClass", "Returns 1 if the player-character is of the specified class, or 0 otherwise.", ConditionParamType::Class),
   ConditionFunction(130, "GetPCIsRace", "Returns 1 if the player-character is of the specified race, or 0 otherwise.", ConditionParamType::Race),
   ConditionFunction(131, "GetPCIsSex", "Returns 1 if the player-character is of the specified sex, or 0 otherwise.", ConditionParamType::Sex),
   ConditionFunction(132, "GetPCInFaction", "", ConditionParamType::Faction),
   ConditionFunction(133, "SameFactionAsPC", ""),
   ConditionFunction(134, "SameRaceAsPC", "Returns 1 if this actor is of the same race as the player-character, or 0 otherwise."),
   ConditionFunction(135, "SameSexAsPC", "Returns 1 if this actor is of the same sex as the player-character, or 0 otherwise."),
   ConditionFunction(136, "GetIsReference", "Returns 1 if this reference is the specified reference, or 0 otherwise.", ConditionParamType::ObjectReference),
   ConditionFunction(137, ConditionFunction::dummy),
   ConditionFunction(138, ConditionFunction::dummy),
   ConditionFunction(139, ConditionFunction::dummy),
   ConditionFunction(140, ConditionFunction::dummy),
   ConditionFunction(141, "IsTalking", ""),
   ConditionFunction(142, "GetWalkSpeed", ""),
   ConditionFunction(143, "GetCurrentAIProcedure", ""),
   ConditionFunction(144, "GetTrespassWarningLevel", ""),
   ConditionFunction(145, "IsTrespassing", ""),
   ConditionFunction(146, "IsInMyOwnedCell", ""),
   ConditionFunction(147, "GetWindSpeed", ""),
   ConditionFunction(148, "GetCurrentWeatherPercent", ""),
   ConditionFunction(149, "GetIsCurrentWeather", "Returns 1 if the specified weather is the current weather, or 0 otherwise.", ConditionParamType::Weather),
   ConditionFunction(150, "IsContinuingPackagePCNear", ""),
   ConditionFunction(151, ConditionFunction::dummy),
   ConditionFunction(152, "GetIsCrimeFaction", "", ConditionParamType::Faction),
   ConditionFunction(153, "CanHaveFlames", ""),
   ConditionFunction(154, "HasFlames", ""),
   ConditionFunction(155, ConditionFunction::dummy),
   ConditionFunction(156, ConditionFunction::dummy),
   ConditionFunction(157, "GetOpenState", ""),
   ConditionFunction(158, ConditionFunction::dummy),
   ConditionFunction(159, "GetSitting", ""),
   ConditionFunction(160, ConditionFunction::dummy),
   ConditionFunction(161, "GetIsCurrentPackage", "", ConditionParamType::Package),
   ConditionFunction(162, "IsCurrentFurnitureRef", "", ConditionParamType::ObjectReference),
   ConditionFunction(163, "IsCurrentFurnitureObj", "", ConditionParamType::Furniture),
   ConditionFunction(164, ConditionFunction::dummy),
   ConditionFunction(165, ConditionFunction::dummy),
   ConditionFunction(166, ConditionFunction::dummy),
   ConditionFunction(167, ConditionFunction::dummy),
   ConditionFunction(168, ConditionFunction::dummy),
   ConditionFunction(169, ConditionFunction::dummy),
   ConditionFunction(170, "GetDayOfWeek", ""),
   ConditionFunction(171, ConditionFunction::dummy),
   ConditionFunction(172, "GetTalkedToPCParam", "", ConditionParamType::Actor),
   ConditionFunction(173, ConditionFunction::dummy),
   ConditionFunction(174, ConditionFunction::dummy),
   ConditionFunction(175, "IsPCSleeping", ""),
   ConditionFunction(176, "IsPCAMurderer", ""),
   ConditionFunction(177, ConditionFunction::dummy),
   ConditionFunction(178, ConditionFunction::dummy),
   ConditionFunction(179, ConditionFunction::dummy),
   ConditionFunction(180, "HasSameEditorLocAsRef", "", ConditionParamType::ObjectReference, ConditionParamType::Keyword),
   ConditionFunction(181, "HasSameEditorLocAsRefAlias", "", ConditionParamType::Alias, ConditionParamType::Keyword),
   ConditionFunction(182, "GetEquipped", "", ConditionParamType::InventoryItem),
   ConditionFunction(183, ConditionFunction::dummy),
   ConditionFunction(184, ConditionFunction::dummy),
   ConditionFunction(185, "IsSwimming", ""),
   ConditionFunction(186, ConditionFunction::dummy),
   ConditionFunction(187, ConditionFunction::dummy),
   ConditionFunction(188, ConditionFunction::dummy),
   ConditionFunction(189, ConditionFunction::dummy),
   ConditionFunction(190, "GetAmountGoldStolen", ""),
   ConditionFunction(191, ConditionFunction::dummy),
   ConditionFunction(192, "GetIgnoreCrime", ""),
   ConditionFunction(193, "GetPCExpelled", "", ConditionParamType::Faction),
   ConditionFunction(194, ConditionFunction::dummy),
   ConditionFunction(195, "GetPCFactionMurder", "", ConditionParamType::Faction),
   ConditionFunction(196, ConditionFunction::dummy),
   ConditionFunction(197, "GetPCEnemyofFaction", "", ConditionParamType::Faction),
   ConditionFunction(198, ConditionFunction::dummy),
   ConditionFunction(199, "GetPCFactionAttack", "", ConditionParamType::Faction),
   ConditionFunction(200, ConditionFunction::dummy),
   //
   // ...FINISH ME!!!
   //
};

const ConditionFunction* getConditionFunction(uint16_t id) {
   if (id >= std::extent<decltype(conditionFunctions)>::value)
      return nullptr;
   return &conditionFunctions[id];
}