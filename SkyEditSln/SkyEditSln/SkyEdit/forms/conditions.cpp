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
   ConditionFunction(  2, ConditionFunction::dummy),
   ConditionFunction(  3, ConditionFunction::dummy),
   ConditionFunction(  4, ConditionFunction::dummy),
   ConditionFunction(  5, "GetLocked",         "Returns 1 if this reference is a locked door or container, or 0 otherwise."),
   ConditionFunction(  6, "GetPos",            "Returns this reference's position along the given axis.", ConditionParamType::Axis),
   ConditionFunction(  7, ConditionFunction::dummy),
   ConditionFunction(  8, "GetAngle",          "Returns this reference's rotation along the given axis.", ConditionParamType::Axis),
   ConditionFunction(  9, ConditionFunction::dummy),
   ConditionFunction( 10, "GetStartingPos",    "Returns this reference's starting position along the given axis.", ConditionParamType::Axis),
   ConditionFunction( 11, "GetStartingAngle",  "Returns this reference's starting position along the given axis.", ConditionParamType::Axis),
   ConditionFunction( 12, "GetSecondsPassed",  ""),
   ConditionFunction( 13, ConditionFunction::dummy),
   ConditionFunction( 14, "GetActorValue",     "Returns the current value of the specified ActorValue.", ConditionParamType::ActorValue),
   ConditionFunction( 15, ConditionFunction::dummy),
   ConditionFunction( 16, ConditionFunction::dummy),
   ConditionFunction( 17, ConditionFunction::dummy),
   ConditionFunction( 18, "GetCurrentTime",    ""),
   ConditionFunction( 19, ConditionFunction::dummy),
   ConditionFunction( 20, ConditionFunction::dummy),
   ConditionFunction( 21, ConditionFunction::dummy),
   ConditionFunction( 22, ConditionFunction::dummy),
   ConditionFunction( 23, ConditionFunction::dummy),
   ConditionFunction( 24, "GetScale",          "Returns this reference's scale."),
   ConditionFunction( 25, "IsMoving",          ""),
   ConditionFunction( 26, "IsTurning",         ""),
   ConditionFunction( 27, "GetLineOfSight",    "", ConditionParamType::ObjectReference),
   ConditionFunction( 28, ConditionFunction::dummy),
   ConditionFunction( 29, ConditionFunction::dummy),
   ConditionFunction( 30, ConditionFunction::dummy),
   ConditionFunction( 31, ConditionFunction::dummy),
   ConditionFunction( 32, "GetInSameCell",     "Returns 1 if this reference is in the same cell as the specified reference, or 0 otherwise.", ConditionParamType::ObjectReference),
   ConditionFunction( 33, ConditionFunction::dummy),
   ConditionFunction( 34, ConditionFunction::dummy),
   ConditionFunction( 35, "GetDisabled",       "Returns 1 if this reference is disabled, or 0 otherwise."),
   ConditionFunction( 36, "MenuMode",          "", ConditionParamType::Integer),
   ConditionFunction( 37, ConditionFunction::dummy),
   ConditionFunction( 38, ConditionFunction::dummy),
   ConditionFunction( 39, "GetDisease",        ""),
   ConditionFunction( 40, ConditionFunction::dummy),
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
   ConditionFunction( 51, ConditionFunction::dummy),
   ConditionFunction( 52, ConditionFunction::dummy),
   ConditionFunction( 53, "GetScriptVariable",  "", ConditionParamType::ObjectReference, ConditionParamType::ScriptVariableIndex),
   ConditionFunction( 54, ConditionFunction::dummy),
   ConditionFunction( 55, ConditionFunction::dummy),
   ConditionFunction( 56, "GetQuestRunning",    "Returns 1 if the specified quest is running, or 0 otherwise.", ConditionParamType::Quest),
   ConditionFunction( 57, ConditionFunction::dummy),
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
   ConditionFunction( 76, ConditionFunction::dummy),
   ConditionFunction( 77, "GetRandomPercent",   "Returns a random number between 0 and 100, inclusive."),
   ConditionFunction( 78, ConditionFunction::dummy),
   ConditionFunction( 79, "GetQuestVariable",   "", ConditionParamType::Quest, ConditionParamType::ScriptVariableIndex),
   ConditionFunction( 80, "GetLevel",           "Returns this actor's level."),
   ConditionFunction( 81, "IsRotating",         ""),
   ConditionFunction( 82, ConditionFunction::dummy),
   ConditionFunction( 83, ConditionFunction::dummy),
   ConditionFunction( 84, "GetDeadCount",       "Returns the number of times this character has died.", ConditionParamType::ActorBase),
   ConditionFunction( 85, ConditionFunction::dummy),
   ConditionFunction( 86, ConditionFunction::dummy),
   ConditionFunction( 87, ConditionFunction::dummy),
   ConditionFunction( 88, ConditionFunction::dummy),
   ConditionFunction( 89, ConditionFunction::dummy),
   ConditionFunction( 90, ConditionFunction::dummy),
   ConditionFunction( 91, "GetIsAlerted", ""),
   ConditionFunction( 92, ConditionFunction::dummy),
   ConditionFunction( 93, ConditionFunction::dummy),
   ConditionFunction( 94, ConditionFunction::dummy),
   ConditionFunction( 95, ConditionFunction::dummy),
   ConditionFunction( 96, ConditionFunction::dummy),
   ConditionFunction( 97, ConditionFunction::dummy),
   ConditionFunction( 98, "GetPlayerControlsDisabled", "", ConditionParamType::Integer, ConditionParamType::Integer),
   ConditionFunction( 99, "GetHeadingAngle",    "", ConditionParamType::ObjectReference),
   ConditionFunction(100, ConditionFunction::dummy),
   ConditionFunction(101, "IsWeaponMagicOut",   ""),
   ConditionFunction(102, "IsTorchOut",         ""),
   ConditionFunction(103, "IsShieldOut",        ""),
   ConditionFunction(104, ConditionFunction::dummy),
   ConditionFunction(105, ConditionFunction::dummy),
   ConditionFunction(106, "IsFacingUp",         ""),
   ConditionFunction(107, "GetKnockedState",    ""),
   ConditionFunction(108, "GetWeaponAnimType",  ""),
   //
   // ...FINISH ME!!!
   //
};

const ConditionFunction* getConditionFunction(uint16_t id) {
   if (id >= std::extent<decltype(conditionFunctions)>::value)
      return nullptr;
   return &conditionFunctions[id];
}