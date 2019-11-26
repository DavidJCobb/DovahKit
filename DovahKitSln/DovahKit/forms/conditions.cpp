#include "conditions.h"
#include "../esp/TESPlugin.h"
#include "../helpers/strings.h"


ConditionArgType* Condition::getArgumentType(uint8_t index) const noexcept {
   if (index >= 2)
      return nullptr;
   auto func = getConditionFunction(this->function);
   if (!func)
      return nullptr;
   auto a = func->argTypes[index];
   if (a->isUnion) {
      assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
      return a->resolveUnion(func->argTypes[index - 1], &this->parameters[index - 1]);
   }
   return a;
}
ConditionArgUnderlyingType Condition::getArgumentUnderlyingType(uint8_t index) const noexcept {
   auto a = this->getArgumentType(index);
   if (a)
      return a->underlying;
   return ConditionArgUnderlyingType::none;
}

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
   {
      auto func = getConditionFunction(this->function);
      for (int i = 0; i < 2; i++) {
         if (func && this->getArgumentUnderlyingType(i) == ConditionArgUnderlyingType::formID)
            subrecord.unchecked_read(this->parameters[i].formID);
         else
            subrecord.unchecked_read(this->parameters[i].dword);
      }
      if (func && func->usesEventData) {
         if (!subrecord.is_in_bounds(8))
            return false;
         subrecord.unchecked_read(this->eventFunction);
         subrecord.unchecked_read(this->eventMember);
         subrecord.unchecked_read(this->eventFormID);
      } else {
         if (!subrecord.is_in_bounds(12))
            return false;
         subrecord.unchecked_read(this->runOn);
         subrecord.unchecked_read(this->runOnRef);
         subrecord.unchecked_read(this->runOnIndex);
      }
   }
   auto next = record.peek_next_subrecord_type();
   if (next != 'CIS1' && next != 'CIS2')
      return true;
   if (next == 'CIS1') {
      auto& sub = record.next_subrecord();
      sub.to_string(this->parameters[0].string);
      //
      next = record.peek_next_subrecord_type();
   }
   if (next == 'CIS2') {
      auto& sub = record.next_subrecord();
      sub.to_string(this->parameters[1].string);
   }
   return true;
}
/*static*/ void Condition::generateUseInfo(TESPluginRecord& record, FormStub* stub) {
   auto& subrecord = record.get_current_subrecord();
   assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
   if (!subrecord.is_in_bounds(0x14))
      return;
   uint8_t   type;
   uint16_t  function;
   form_id_t formID;
   subrecord.unchecked_read(type);
   subrecord.skip_bytes(3);
   if (type & ConditionTypeFlags::compare_to_global) {
      subrecord.unchecked_read(formID);
      stub->add_outbound_reference(formID);
   } else
      subrecord.skip_bytes(4);
   subrecord.unchecked_read(function);
   subrecord.skip_bytes(2);
   {
      auto func = getConditionFunction(function);
      auto arg0 = func->argTypes[0];
      auto arg1 = func->argTypes[1];
      uint32_t firstValue; // needed for when the second arg is a union
      if (arg0 && arg0->underlying == ConditionArgUnderlyingType::formID) {
         subrecord.unchecked_read(formID);
         stub->add_outbound_reference(formID);
      } else
         subrecord.unchecked_read(firstValue);
      if (arg1 && arg1->isUnion) { // resolve the union
         ConditionArgValue value;
         value.dword = firstValue;
         arg1 = arg1->resolveUnion(arg0, &value);
      }
      if (arg1 && arg1->underlying == ConditionArgUnderlyingType::formID) {
         subrecord.unchecked_read(formID);
         stub->add_outbound_reference(formID);
      } else
         subrecord.skip_bytes(4);
      if (func && func->usesEventData) {
         if (!subrecord.is_in_bounds(8))
            return;
         subrecord.skip_bytes(4);
         subrecord.unchecked_read(formID);
         stub->add_outbound_reference(formID);
      } else {
         if (!subrecord.is_in_bounds(12))
            return;
         subrecord.skip_bytes(4);
         subrecord.unchecked_read(formID);
         stub->add_outbound_reference(formID);
         subrecord.skip_bytes(4);
      }
   }
   auto next = record.peek_next_subrecord_type();
   if (next != 'CIS1' && next != 'CIS2')
      return;
   if (next == 'CIS1') {
      record.next_subrecord();
      next = record.peek_next_subrecord_type();
   }
   if (next == 'CIS2')
      record.next_subrecord();
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
         cobb::sprintf(out, "[REFR:%08X]", this->runOnRef);
         break;
      case ConditionRunOn::combat_target:
         out += "CombatTarget";
         break;
      case ConditionRunOn::event_data:
         out += "EventData"; // TODO: how does this work?
         break;
      case ConditionRunOn::quest_alias:
         cobb::sprintf(out, "QuestAlias[%d]", this->runOnIndex); // can't show more meaningful information without access to the condition's containing form
         break;
      case ConditionRunOn::package_data:
         cobb::sprintf(out, "PackageData[%d]", this->runOnIndex); // can't show more meaningful information without access to the condition's containing form
         break;
      default:
         out += "?????";
   }
   out += '.';
   out += function->name;
   out += '(';
   for (uint32_t i = 0; i < 3; i++) {
      auto type = this->getArgumentType(i);
      if (type && !type->isNone()) {
         if (i > 0)
            out += ", ";
         auto& param = this->parameters[i];
         std::string temp;
         type->toString(param, temp);
         out += temp;
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
   ConditionFunction(0, "GetWantBlocking",   ""),
   ConditionFunction(1, "GetDistance",       "Returns the distance between this reference and another reference.", ConditionArgTypes::ObjectReference),
   ConditionFunction(2, ConditionFunction::dummy), // actually AddItem
   ConditionFunction(3, ConditionFunction::dummy), // actually SetEssential
   ConditionFunction(4, ConditionFunction::dummy), // actually Rotate
   ConditionFunction(5, "GetLocked",         "Returns 1 if this reference is a locked door or container, or 0 otherwise."),
   ConditionFunction(6, "GetPos",            "Returns this reference's position along the given axis.", ConditionArgTypes::Axis),
   ConditionFunction(7, ConditionFunction::dummy), // actually SetPos
   ConditionFunction(8, "GetAngle",          "Returns this reference's rotation along the given axis.", ConditionArgTypes::Axis),
   ConditionFunction(9, ConditionFunction::dummy), // actually SetANgle
   ConditionFunction(10, "GetStartingPos",    "Returns this reference's starting position along the given axis.", ConditionArgTypes::Axis),
   ConditionFunction(11, "GetStartingAngle",  "Returns this reference's starting position along the given axis.", ConditionArgTypes::Axis),
   ConditionFunction(12, "GetSecondsPassed",  ""),
   ConditionFunction(13, ConditionFunction::dummy), // actually Activate
   ConditionFunction(14, "GetActorValue",     "Returns the current value of the specified ActorValue.", ConditionArgTypes::ActorValue),
   ConditionFunction(15, ConditionFunction::dummy), // actually SetActorValue
   ConditionFunction(16, ConditionFunction::dummy), // actually ModActorValue
   ConditionFunction(17, ConditionFunction::dummy), // actually SetAtStart
   ConditionFunction(18, "GetCurrentTime",    ""),
   ConditionFunction(19, ConditionFunction::dummy), // actually PlayGroup
   ConditionFunction(20, ConditionFunction::dummy), // actually LoopGroup
   ConditionFunction(21, ConditionFunction::dummy), // actually SkipAnim
   ConditionFunction(22, ConditionFunction::dummy), // actually StartCombat
   ConditionFunction(23, ConditionFunction::dummy), // actually StopCombat
   ConditionFunction(24, "GetScale",          "Returns this reference's scale."),
   ConditionFunction(25, "IsMoving",          ""),
   ConditionFunction(26, "IsTurning",         "Returns 1 if this actor is turning to the left, 2 if they are turning to the right, or 0 otherwise."),
   ConditionFunction(27, "GetLineOfSight",    "", ConditionArgTypes::ObjectReference),
   ConditionFunction(28, ConditionFunction::dummy), // actually AddSpell
   ConditionFunction(29, ConditionFunction::dummy), // actually RemoveSpell
   ConditionFunction(30, ConditionFunction::dummy), // actualy Cast
   ConditionFunction(31, ConditionFunction::dummy), // actually GetButtonPressed
   ConditionFunction(32, "GetInSameCell",     "Returns 1 if this reference is in the same cell as the specified reference, or 0 otherwise.", ConditionArgTypes::ObjectReference),
   ConditionFunction(33, ConditionFunction::dummy), // actually Enable
   ConditionFunction(34, ConditionFunction::dummy), // actually Disable
   ConditionFunction(35, "GetDisabled",       "Returns 1 if this reference is disabled, or 0 otherwise."),
   ConditionFunction(36, "MenuMode",          "", ConditionArgTypes::Integer),
   ConditionFunction(37, ConditionFunction::dummy), // actually PlaceAtMe
   ConditionFunction(38, ConditionFunction::dummy), // actually PlaySound
   ConditionFunction(39, "GetDisease",        "Returns 1 if any of this actor's active magic effects came from a spell whose type was set to \"Disease,\" or 0 otherwise."),
   ConditionFunction(40, ConditionFunction::dummy), // actually FailAllObjectives
   ConditionFunction(41, "GetClothingValue",  ""),
   ConditionFunction(42, "SameFaction",       "Returns 1 if this reference is in the same faction as the specified actor, or 0 otherwise.", ConditionArgTypes::Actor),
   ConditionFunction(43, "SameRace",          "Returns 1 if this reference is of the same race as the specified actor, or 0 otherwise.", ConditionArgTypes::Actor),
   ConditionFunction(44, "SameSex",           "Returns 1 if this reference is of the same sex as the specified actor, or 0 otherwise.", ConditionArgTypes::Actor),
   ConditionFunction(45, "GetDetected",       "", ConditionArgTypes::Actor),
   ConditionFunction(46, "GetDead",           ""),
   ConditionFunction(47, "GetItemCount",      "Returns how many of the specified form this reference has in its inventory.", ConditionArgTypes::InventoryItem),
   ConditionFunction(48, "GetGold",           "Returns how much gold this reference is carrying."),
   ConditionFunction(49, "GetSleeping",       ""),
   ConditionFunction(50, "GetTalkedToPC",     ""),
   ConditionFunction(51, ConditionFunction::dummy), // actually Say
   ConditionFunction(52, ConditionFunction::dummy), // actually SayTo
   ConditionFunction(53, "GetScriptVariable",  "", ConditionArgTypes::ObjectReference, ConditionArgTypes::String),
   ConditionFunction(54, ConditionFunction::dummy), // actually StartQuest
   ConditionFunction(55, ConditionFunction::dummy), // actually StopQuest
   ConditionFunction(56, "GetQuestRunning",    "Returns 1 if the specified quest is running, or 0 otherwise.", ConditionArgTypes::Quest),
   ConditionFunction(57, ConditionFunction::dummy), // actually SetStage
   ConditionFunction(58, "GetStage",           "Returns the specified quest's current stage number.", ConditionArgTypes::Quest),
   ConditionFunction(59, "GetStageDone",       "Returns 1 if the specified quest stage is complete, or 0 otherwise.", ConditionArgTypes::Quest, ConditionArgTypes::QuestStage),
   ConditionFunction(60, "GetFactionRankDifference", "", ConditionArgTypes::Faction, ConditionArgTypes::Actor),
   ConditionFunction(61, "GetAlarmed",         ""),
   ConditionFunction(62, "IsRaining",          "Returns 1 if the current weather is rainy, or 0 otherwise."),
   ConditionFunction(63, "GetAttacked",        ""),
   ConditionFunction(64, "GetIsCreature",      ""),
   ConditionFunction(65, "GetLockLevel",       ""),
   ConditionFunction(66, "GetShouldAttack",    "", ConditionArgTypes::Actor),
   ConditionFunction(67, "GetInCell",          "Returns 1 if this reference is in the specified cell, or 0 otherwise.", ConditionArgTypes::Cell),
   ConditionFunction(68, "GetIsClass",         "Returns 1 if this actor is of the specified class, or 0 otherwise.", ConditionArgTypes::Class),
   ConditionFunction(69, "GetIsRace",          "Returns 1 if this actor is of the specified race, or 0 otherwise.",  ConditionArgTypes::Race),
   ConditionFunction(70, "GetIsSex",           "Returns 1 if this actor is of the specified sex, or 0 otherwise.",   ConditionArgTypes::Sex),
   ConditionFunction(71, "GetInFaction",       "", ConditionArgTypes::Faction),
   ConditionFunction(72, "GetIsID",            "", ConditionArgTypes::BaseForm),
   ConditionFunction(73, "GetFactionRank",     "", ConditionArgTypes::Faction),
   ConditionFunction(74, "GetGlobalValue",     "Returns the value of the specified Global.", ConditionArgTypes::Global),
   ConditionFunction(75, "IsSnowing",          "Returns 1 if the current weather is snowy, or 0 otherwise."),
   ConditionFunction(76, ConditionFunction::dummy), // actually FastTravel
   ConditionFunction(77, "GetRandomPercent",   "Returns a random number between 0 and 100, inclusive."),
   ConditionFunction(78, ConditionFunction::dummy), // actually RemoveMusic
   ConditionFunction(79, "GetQuestVariable",   "", ConditionArgTypes::Quest, ConditionArgTypes::String),
   ConditionFunction(80, "GetLevel",           "Returns this actor's level."),
   ConditionFunction(81, "IsRotating",         ""),
   ConditionFunction(82, ConditionFunction::dummy), // actually RemoveItem
   ConditionFunction(83, ConditionFunction::dummy), // actually GetLeveledEncounterValue
   ConditionFunction(84, "GetDeadCount",       "Returns the number of times this character has died.", ConditionArgTypes::ActorBase),
   ConditionFunction(85, ConditionFunction::dummy), // actually AddToMap
   ConditionFunction(86, ConditionFunction::dummy), // actually StartConversation
   ConditionFunction(87, ConditionFunction::dummy), // actually Drop
   ConditionFunction(88, ConditionFunction::dummy), // actually AddTopic
   ConditionFunction(89, ConditionFunction::dummy), // actually ShowMessage
   ConditionFunction(90, ConditionFunction::dummy), // actually SetAlert
   ConditionFunction(91, "GetIsAlerted", ""),
   ConditionFunction(92, ConditionFunction::dummy), // actually Look
   ConditionFunction(93, ConditionFunction::dummy), // actually StopLook
   ConditionFunction(94, ConditionFunction::dummy), // actually EvaluatePackage
   ConditionFunction(95, ConditionFunction::dummy), // actually SendAssaultAlarm
   ConditionFunction(96, ConditionFunction::dummy), // actually EnablePlayerControls
   ConditionFunction(97, ConditionFunction::dummy), // actually DisablePlayerControls
   ConditionFunction(98, "GetPlayerControlsDisabled", "", ConditionArgTypes::Integer, ConditionArgTypes::Integer),
   ConditionFunction(99, "GetHeadingAngle", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(100, ConditionFunction::dummy), // actually PickIdle
   ConditionFunction(101, "IsWeaponMagicOut", ""),
   ConditionFunction(102, "IsTorchOut", ""),
   ConditionFunction(103, "IsShieldOut", ""),
   ConditionFunction(104, ConditionFunction::dummy), // actually CreateDetectionEvent
   ConditionFunction(105, ConditionFunction::dummy), // actually IsActionRef
   ConditionFunction(106, "IsFacingUp", ""),
   ConditionFunction(107, "GetKnockedState", ""),
   ConditionFunction(108, "GetWeaponAnimType", ""),
   ConditionFunction(109, "IsWeaponSkillType", "", ConditionArgTypes::ActorValue),
   ConditionFunction(110, "GetCurrentAIPackage", ""),
   ConditionFunction(111, "IsWaiting", ""),
   ConditionFunction(112, "IsIdlePlaying", ""),
   ConditionFunction(113, ConditionFunction::dummy), // actually CompleteQuest
   ConditionFunction(114, ConditionFunction::dummy), // actually Lock
   ConditionFunction(115, ConditionFunction::dummy), // actually Unlock
   ConditionFunction(116, "IsIntimidatedByPlayer", ""),
   ConditionFunction(117, "IsPlayerInRegion", "Returns 1 if this reference's parent cell belongs to or overlaps the specified region, or 0 otherwise."),
   ConditionFunction(118, "GetActorAggroRadiusViolated", ""),
   ConditionFunction(119, ConditionFunction::dummy),
   ConditionFunction(120, ConditionFunction::dummy),
   ConditionFunction(121, ConditionFunction::dummy),
   ConditionFunction(122, "GetCrime", "", ConditionArgTypes::Actor, ConditionArgTypes::CrimeType),
   ConditionFunction(123, "IsGreetingPlayer", ""),
   ConditionFunction(124, ConditionFunction::dummy),
   ConditionFunction(125, "IsGuard", ""),
   ConditionFunction(126, ConditionFunction::dummy),
   ConditionFunction(127, "HasBeenEaten", "Returns 1 if this actor has been fed on by a cannibal or werewolf, or 0 otherwise."),
   ConditionFunction(128, "GetStaminaPercentage", ""),
   ConditionFunction(129, "GetPCIsClass", "Returns 1 if the player-character is of the specified class, or 0 otherwise.", ConditionArgTypes::Class),
   ConditionFunction(130, "GetPCIsRace", "Returns 1 if the player-character is of the specified race, or 0 otherwise.", ConditionArgTypes::Race),
   ConditionFunction(131, "GetPCIsSex", "Returns 1 if the player-character is of the specified sex, or 0 otherwise.", ConditionArgTypes::Sex),
   ConditionFunction(132, "GetPCInFaction", "", ConditionArgTypes::Faction),
   ConditionFunction(133, "SameFactionAsPC", ""),
   ConditionFunction(134, "SameRaceAsPC", "Returns 1 if this actor is of the same race as the player-character, or 0 otherwise."),
   ConditionFunction(135, "SameSexAsPC", "Returns 1 if this actor is of the same sex as the player-character, or 0 otherwise."),
   ConditionFunction(136, "GetIsReference", "Returns 1 if this reference is the specified reference, or 0 otherwise.", ConditionArgTypes::ObjectReference),
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
   ConditionFunction(149, "GetIsCurrentWeather", "Returns 1 if the specified weather is the current weather, or 0 otherwise.", ConditionArgTypes::Weather),
   ConditionFunction(150, "IsContinuingPackagePCNear", ""),
   ConditionFunction(151, ConditionFunction::dummy),
   ConditionFunction(152, "GetIsCrimeFaction", "", ConditionArgTypes::Faction),
   ConditionFunction(153, "CanHaveFlames", ""),
   ConditionFunction(154, "HasFlames", ""),
   ConditionFunction(155, ConditionFunction::dummy),
   ConditionFunction(156, ConditionFunction::dummy),
   ConditionFunction(157, "GetOpenState", ""),
   ConditionFunction(158, ConditionFunction::dummy),
   ConditionFunction(159, "GetSitting", ""),
   ConditionFunction(160, ConditionFunction::dummy),
   ConditionFunction(161, "GetIsCurrentPackage", "", ConditionArgTypes::Package),
   ConditionFunction(162, "IsCurrentFurnitureRef", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(163, "IsCurrentFurnitureObj", "", ConditionArgTypes::Furniture),
   ConditionFunction(164, ConditionFunction::dummy),
   ConditionFunction(165, ConditionFunction::dummy),
   ConditionFunction(166, ConditionFunction::dummy),
   ConditionFunction(167, ConditionFunction::dummy),
   ConditionFunction(168, ConditionFunction::dummy),
   ConditionFunction(169, ConditionFunction::dummy),
   ConditionFunction(170, "GetDayOfWeek", ""),
   ConditionFunction(171, ConditionFunction::dummy),
   ConditionFunction(172, "GetTalkedToPCParam", "", ConditionArgTypes::Actor),
   ConditionFunction(173, ConditionFunction::dummy),
   ConditionFunction(174, ConditionFunction::dummy),
   ConditionFunction(175, "IsPCSleeping", ""),
   ConditionFunction(176, "IsPCAMurderer", ""),
   ConditionFunction(177, ConditionFunction::dummy),
   ConditionFunction(178, ConditionFunction::dummy),
   ConditionFunction(179, ConditionFunction::dummy),
   ConditionFunction(180, "HasSameEditorLocAsRef", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::Keyword),
   ConditionFunction(181, "HasSameEditorLocAsRefAlias", "", ConditionArgTypes::Alias, ConditionArgTypes::Keyword),
   ConditionFunction(182, "GetEquipped", "", ConditionArgTypes::InventoryItem),
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
   ConditionFunction(193, "GetPCExpelled", "", ConditionArgTypes::Faction),
   ConditionFunction(194, ConditionFunction::dummy),
   ConditionFunction(195, "GetPCFactionMurder", "", ConditionArgTypes::Faction),
   ConditionFunction(196, ConditionFunction::dummy),
   ConditionFunction(197, "GetPCEnemyofFaction", "", ConditionArgTypes::Faction),
   ConditionFunction(198, ConditionFunction::dummy),
   ConditionFunction(199, "GetPCFactionAttack", "", ConditionArgTypes::Faction),
   ConditionFunction(200, ConditionFunction::dummy),
   ConditionFunction(201, ConditionFunction::dummy),
   ConditionFunction(202, ConditionFunction::dummy),
   ConditionFunction(203, "GetDestroyed", "Returns 1 if this reference is destroyed, or 0 otherwise."),
   ConditionFunction(204, ConditionFunction::dummy),
   ConditionFunction(205, ConditionFunction::dummy),
   ConditionFunction(206, ConditionFunction::dummy),
   ConditionFunction(207, ConditionFunction::dummy),
   ConditionFunction(208, ConditionFunction::dummy),
   ConditionFunction(209, ConditionFunction::dummy),
   ConditionFunction(210, ConditionFunction::dummy),
   ConditionFunction(211, ConditionFunction::dummy),
   ConditionFunction(212, ConditionFunction::dummy),
   ConditionFunction(213, ConditionFunction::dummy),
   ConditionFunction(214, "HasMagicEffect", "", ConditionArgTypes::MagicEffect),
   ConditionFunction(215, "GetDefaultOpen", ""),
   ConditionFunction(216, ConditionFunction::dummy),
   ConditionFunction(217, ConditionFunction::dummy),
   ConditionFunction(218, ConditionFunction::dummy),
   ConditionFunction(219, "GetAnimAction", ""),
   ConditionFunction(220, ConditionFunction::dummy),
   ConditionFunction(221, ConditionFunction::dummy),
   ConditionFunction(222, ConditionFunction::dummy),
   ConditionFunction(223, "IsSpellTarget", "", ConditionArgTypes::Spell),
   ConditionFunction(224, "GetVATSMode", ""),
   ConditionFunction(225, "GetPersuasionNumber", ""),
   ConditionFunction(226, "GetVampireFeed", "Returns 1 if this actor is a vampire currently feeding on another actor, or 0 otherwise."),
   ConditionFunction(227, "GetCannibal", "Returns 1 if this actor is currently a cannibal feeding on another actor, or 0 otherwise."),
   ConditionFunction(228, "GetIsClassDefault", "", ConditionArgTypes::Class),
   ConditionFunction(229, "GetClassDefaultMatch", ""),
   ConditionFunction(230, "GetInCellParam", "Returns 1 if the specified reference is inside of the specified cell, or 0 otherwise.", ConditionArgTypes::Cell, ConditionArgTypes::ObjectReference),
   ConditionFunction(231, ConditionFunction::dummy),
   ConditionFunction(232, ConditionFunction::dummy),
   ConditionFunction(233, ConditionFunction::dummy),
   ConditionFunction(234, ConditionFunction::dummy),
   ConditionFunction(235, "GetVatsTargetHeight", ""),
   ConditionFunction(236, ConditionFunction::dummy),
   ConditionFunction(237, "GetIsGhost", ""),
   ConditionFunction(238, ConditionFunction::dummy),
   ConditionFunction(239, ConditionFunction::dummy),
   ConditionFunction(240, ConditionFunction::dummy),
   ConditionFunction(241, ConditionFunction::dummy),
   ConditionFunction(242, "GetUnconscious", "Returns 1 if this actor is unconscious, or 0 otherwise."),
   ConditionFunction(243, ConditionFunction::dummy),
   ConditionFunction(244, "GetRestrained", "Returns 1 if this actor is restrained, or 0 otherwise."),
   ConditionFunction(245, ConditionFunction::dummy),
   ConditionFunction(246, "GetIsUsedItem", "", ConditionArgTypes::BaseForm),
   ConditionFunction(247, "GetIsUsedItemType", "", ConditionArgTypes::FormType),
   ConditionFunction(248, "IsScenePlaying", "", ConditionArgTypes::Scene),
   ConditionFunction(249, "IsInDialogueWithPlayer", ""),
   ConditionFunction(250, "GetLocationCleared", "", ConditionArgTypes::Location),
   ConditionFunction(251, ConditionFunction::dummy),
   ConditionFunction(252, ConditionFunction::dummy),
   ConditionFunction(253, ConditionFunction::dummy),
   ConditionFunction(254, "GetIsPlayableRace", ""),
   ConditionFunction(255, "GetOffersServicesNow", "Returns 1 if this actor is currently available to barter with, or 0 otherwise."),
   ConditionFunction(256, ConditionFunction::dummy),
   ConditionFunction(257, ConditionFunction::dummy),
   ConditionFunction(258, "HasAssociationType", "", ConditionArgTypes::Actor, ConditionArgTypes::AssociationType),
   ConditionFunction(259, "HasFamilyRelationship", "", ConditionArgTypes::Actor),
   ConditionFunction(260, ConditionFunction::dummy),
   ConditionFunction(261, "HasParentRelationship", "", ConditionArgTypes::Actor),
   ConditionFunction(262, "IsWarningAbout", "", ConditionArgTypes::FormList),
   ConditionFunction(263, "IsWeaponOut", ""),
   ConditionFunction(264, "HasSpell", "", ConditionArgTypes::Spell),
   ConditionFunction(265, "IsTimePassing", ""),
   ConditionFunction(266, "IsPleasant", "Returns 1 if the current weather is pleasant, or 0 otherwise."),
   ConditionFunction(267, "IsCloudy", "Returns 1 if the current weather is cloudy, or 0 otherwise."),
   ConditionFunction(268, ConditionFunction::dummy),
   ConditionFunction(269, ConditionFunction::dummy),
   ConditionFunction(270, ConditionFunction::dummy),
   ConditionFunction(271, ConditionFunction::dummy),
   ConditionFunction(272, ConditionFunction::dummy),
   ConditionFunction(273, ConditionFunction::dummy),
   ConditionFunction(274, "IsSmallBump", ""),
   ConditionFunction(275, ConditionFunction::dummy),
   ConditionFunction(276, ConditionFunction::dummy),
   ConditionFunction(277, "GetBaseActorValue", "Returns the base value of the specified ActorValue.", ConditionArgTypes::ActorValue),
   ConditionFunction(278, "IsOwner", "", ConditionArgTypes::OwnerForm),
   ConditionFunction(279, ConditionFunction::dummy),
   ConditionFunction(280, "IsCellOwner", "Returns 1 if the specified actor or faction owns the specified cell, or 0 otherwise.", ConditionArgTypes::Cell, ConditionArgTypes::OwnerForm),
   ConditionFunction(281, ConditionFunction::dummy),
   ConditionFunction(282, "IsHorseStolen", ""),
   ConditionFunction(283, ConditionFunction::dummy),
   ConditionFunction(284, ConditionFunction::dummy),
   ConditionFunction(285, "IsLeftUp", ""),
   ConditionFunction(286, "IsSneaking", "Returns 1 if this actor is in sneak mode, or 0 otherwise."),
   ConditionFunction(287, "IsRunning", ""),
   ConditionFunction(288, "GetFriendHit", ""),
   ConditionFunction(289, "IsInCombat", "", ConditionArgTypes::Integer),
   ConditionFunction(290, ConditionFunction::dummy),
   ConditionFunction(291, ConditionFunction::dummy),
   ConditionFunction(292, ConditionFunction::dummy),
   ConditionFunction(293, ConditionFunction::dummy),
   ConditionFunction(294, ConditionFunction::dummy),
   ConditionFunction(295, ConditionFunction::dummy),
   ConditionFunction(296, ConditionFunction::dummy),
   ConditionFunction(297, ConditionFunction::dummy),
   ConditionFunction(298, ConditionFunction::dummy),
   ConditionFunction(299, ConditionFunction::dummy),
   ConditionFunction(300, "IsInInterior", "Returns 1 if this reference is in an interior cell, or 0 otherwise."),
   ConditionFunction(301, ConditionFunction::dummy),
   ConditionFunction(302, ConditionFunction::dummy),
   ConditionFunction(303, ConditionFunction::dummy),
   ConditionFunction(304, "IsWaterObject", ""),
   ConditionFunction(305, "GetPlayerAction", ""),
   ConditionFunction(306, "IsActorUsingATorch", ""),
   ConditionFunction(307, ConditionFunction::dummy),
   ConditionFunction(308, ConditionFunction::dummy),
   ConditionFunction(309, "IsXBox", ""),
   ConditionFunction(310, "GetInWorldspace", "", ConditionArgTypes::Worldspace),
   ConditionFunction(311, ConditionFunction::dummy),
   ConditionFunction(312, "GetPCMiscStat", "Returns the value of the specified misc stat.", ConditionArgTypes::MiscStat),
   ConditionFunction(313, "GetPairedAnimation", ""),
   ConditionFunction(314, "IsActorAVictim", ""),
   ConditionFunction(315, "GetTotalPersuasionNumber", ""),
   ConditionFunction(316, ConditionFunction::dummy),
   ConditionFunction(317, ConditionFunction::dummy),
   ConditionFunction(318, "GetIdleDoneOnce", ""),
   ConditionFunction(319, ConditionFunction::dummy),
   ConditionFunction(320, "GetNoRumors", ""),
   ConditionFunction(321, ConditionFunction::dummy),
   ConditionFunction(322, ConditionFunction::dummy),
   ConditionFunction(323, "GetCombatState", ""),
   ConditionFunction(324, ConditionFunction::dummy),
   ConditionFunction(325, "GetWithinPackageLocation", "", ConditionArgTypes::PackageData),
   ConditionFunction(326, ConditionFunction::dummy),
   ConditionFunction(327, "IsRidingMount", ""),
   ConditionFunction(328, ConditionFunction::dummy),
   ConditionFunction(329, "IsFleeing", ""),
   ConditionFunction(330, ConditionFunction::dummy),
   ConditionFunction(331, ConditionFunction::dummy),
   ConditionFunction(332, "IsInDangerousWater", "Returns 1 if this actor is standing or swimming in a body of water that has been flagged as dangerous, or 0 otherwise."),
   ConditionFunction(333, ConditionFunction::dummy),
   ConditionFunction(334, ConditionFunction::dummy),
   ConditionFunction(335, ConditionFunction::dummy),
   ConditionFunction(336, ConditionFunction::dummy),
   ConditionFunction(337, ConditionFunction::dummy),
   ConditionFunction(338, "GetIgnoreFriendlyHits", ""),
   ConditionFunction(339, "IsPlayersLastRiddenMount", ""),
   ConditionFunction(340, ConditionFunction::dummy),
   ConditionFunction(341, ConditionFunction::dummy),
   ConditionFunction(342, ConditionFunction::dummy),
   ConditionFunction(343, ConditionFunction::dummy),
   ConditionFunction(344, ConditionFunction::dummy),
   ConditionFunction(345, ConditionFunction::dummy),
   ConditionFunction(346, ConditionFunction::dummy),
   ConditionFunction(347, ConditionFunction::dummy),
   ConditionFunction(348, ConditionFunction::dummy),
   ConditionFunction(349, ConditionFunction::dummy),
   ConditionFunction(350, ConditionFunction::dummy),
   ConditionFunction(351, ConditionFunction::dummy),
   ConditionFunction(352, ConditionFunction::dummy),
   ConditionFunction(353, "IsActor", "Returns 1 if this reference is an actor, or 0 otherwise."),
   ConditionFunction(354, "IsEssential", "Returns 1 if this actor is flagged as essential, or 0 otherwise."),
   ConditionFunction(355, ConditionFunction::dummy),
   ConditionFunction(356, ConditionFunction::dummy),
   ConditionFunction(357, ConditionFunction::dummy),
   ConditionFunction(358, "IsPlayerMovingIntoNewSpace", ""),
   ConditionFunction(359, "GetInCurrentLoc", "", ConditionArgTypes::Location),
   ConditionFunction(360, "GetInCurrentLocAlias", "", ConditionArgTypes::Alias),
   ConditionFunction(361, "GetTimeDead", ""),
   ConditionFunction(362, "HasLinkedRef", "", ConditionArgTypes::Keyword),
   ConditionFunction(363, ConditionFunction::dummy),
   ConditionFunction(364, ConditionFunction::dummy),
   ConditionFunction(365, "IsChild", ""),
   ConditionFunction(366, "GetStolenItemValueNoCrime", "", ConditionArgTypes::Faction),
   ConditionFunction(367, "GetLastPlayerAction", ""),
   ConditionFunction(368, "IsPlayerActionActive", "", ConditionArgTypes::Integer), // arg type needs verification
   ConditionFunction(369, ConditionFunction::dummy),
   ConditionFunction(370, "IsTalkingActivatorActor", "", ConditionArgTypes::Actor),
   ConditionFunction(371, ConditionFunction::dummy),
   ConditionFunction(372, "IsInList", "", ConditionArgTypes::FormList),
   ConditionFunction(373, "GetStolenItemValue", "", ConditionArgTypes::Faction),
   ConditionFunction(374, ConditionFunction::dummy),
   ConditionFunction(375, "GetCrimeGoldViolent", ""),
   ConditionFunction(376, "GetCrimeGoldNonViolent", ""),
   ConditionFunction(377, ConditionFunction::dummy),
   ConditionFunction(378, "HasShout", "", ConditionArgTypes::Shout),
   ConditionFunction(379, ConditionFunction::dummy),
   ConditionFunction(380, ConditionFunction::dummy),
   ConditionFunction(381, "GetHasNote", "", ConditionArgTypes::Integer), // arg type needs verification
   ConditionFunction(382, ConditionFunction::dummy),
   ConditionFunction(383, ConditionFunction::dummy),
   ConditionFunction(384, ConditionFunction::dummy),
   ConditionFunction(385, ConditionFunction::dummy),
   ConditionFunction(386, ConditionFunction::dummy),
   ConditionFunction(387, ConditionFunction::dummy),
   ConditionFunction(388, ConditionFunction::dummy),
   ConditionFunction(389, ConditionFunction::dummy),
   ConditionFunction(390, "GetHitLocation", ""),
   ConditionFunction(391, "IsPC1stPerson", ""),
   ConditionFunction(392, ConditionFunction::dummy),
   ConditionFunction(393, ConditionFunction::dummy),
   ConditionFunction(394, ConditionFunction::dummy),
   ConditionFunction(395, ConditionFunction::dummy),
   ConditionFunction(396, "GetCauseofDeath", ""),
   ConditionFunction(397, "IsLimbGone", "", ConditionArgTypes::Integer), // CK actually does expose this as an int
   ConditionFunction(398, "IsWeaponInList", "", ConditionArgTypes::FormList),
   ConditionFunction(399, ConditionFunction::dummy),
   ConditionFunction(400, ConditionFunction::dummy),
   ConditionFunction(401, ConditionFunction::dummy),
   ConditionFunction(402, "IsBribedByPlayer", ""),
   ConditionFunction(403, "GetRelationshipRank", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(404, ConditionFunction::dummy),
   ConditionFunction(405, ConditionFunction::dummy),
   ConditionFunction(406, ConditionFunction::dummy),
   ConditionFunction(407, "GetVATSValue", "", ConditionArgTypes::VATSValueFunction, ConditionArgTypes::VATSValue),
   ConditionFunction(408, "IsKiller", "", ConditionArgTypes::Actor),
   ConditionFunction(409, "IsKillerObject", "", ConditionArgTypes::FormList),
   ConditionFunction(410, "GetFactionCombatReaction", "", ConditionArgTypes::Faction, ConditionArgTypes::Faction),
   ConditionFunction(411, ConditionFunction::dummy),
   ConditionFunction(412, ConditionFunction::dummy),
   ConditionFunction(413, ConditionFunction::dummy),
   ConditionFunction(414, "Exists", "Returns 1 if this reference is the specified reference and if the specified reference exists, or 0 otherwise.", ConditionArgTypes::ObjectReference),
   ConditionFunction(415, "GetGroupMemberCount", ""),
   ConditionFunction(416, "GetGroupTargetCount", ""),
   ConditionFunction(417, ConditionFunction::dummy),
   ConditionFunction(418, ConditionFunction::dummy),
   ConditionFunction(419, ConditionFunction::dummy),
   ConditionFunction(420, ConditionFunction::dummy),
   ConditionFunction(421, ConditionFunction::dummy),
   ConditionFunction(422, ConditionFunction::dummy),
   ConditionFunction(423, ConditionFunction::dummy),
   ConditionFunction(424, ConditionFunction::dummy),
   ConditionFunction(425, ConditionFunction::dummy),
   ConditionFunction(426, "GetIsVoiceType", "Returns 1 if this actor uses the specified voicetype, or 0 otherwise.", ConditionArgTypes::Voicetype),
   ConditionFunction(427, "GetPlantedExplosive", ""),
   ConditionFunction(428, ConditionFunction::dummy),
   ConditionFunction(429, "IsScenePackageRunning", ""),
   ConditionFunction(430, "GetHealthPercentage", ""),
   ConditionFunction(431, ConditionFunction::dummy),
   ConditionFunction(432, "GetIsObjectType", "", ConditionArgTypes::FormList),
   ConditionFunction(433, ConditionFunction::dummy),
   ConditionFunction(434, "GetDialogueEmotion", ""),
   ConditionFunction(435, "GetDialogueEmotionValue", ""),
   ConditionFunction(436, ConditionFunction::dummy),
   ConditionFunction(437, "GetIsCreatureType", "", ConditionArgTypes::Integer), // double-check this
   ConditionFunction(438, ConditionFunction::dummy),
   ConditionFunction(439, ConditionFunction::dummy),
   ConditionFunction(440, ConditionFunction::dummy),
   ConditionFunction(441, ConditionFunction::dummy),
   ConditionFunction(442, ConditionFunction::dummy),
   ConditionFunction(443, ConditionFunction::dummy),
   ConditionFunction(444, "GetInCurrentLocFormList", "", ConditionArgTypes::FormList),
   ConditionFunction(445, "GetInZone", "", ConditionArgTypes::EncounterZone),
   ConditionFunction(446, "GetVelocity", "", ConditionArgTypes::Axis),
   ConditionFunction(447, "GetGraphVariableFloat", "", ConditionArgTypes::String),
   ConditionFunction(448, "HasPerk", "", ConditionArgTypes::Perk, ConditionArgTypes::Integer), // second arg type is not known
   ConditionFunction(449, "GetFactionRelation", "", ConditionArgTypes::Actor),
   ConditionFunction(450, "IsLastIdlePlayed", "", ConditionArgTypes::Idle),
   ConditionFunction(451, ConditionFunction::dummy),
   ConditionFunction(452, ConditionFunction::dummy),
   ConditionFunction(453, "GetPlayerTeammate", ""),
   ConditionFunction(454, "GetPlayerTeammateCount", ""),
   ConditionFunction(455, ConditionFunction::dummy),
   ConditionFunction(456, ConditionFunction::dummy),
   ConditionFunction(457, ConditionFunction::dummy),
   ConditionFunction(458, "GetActorCrimePlayerEnemy", ""),
   ConditionFunction(459, "GetCrimeGold", ""),
   ConditionFunction(460, ConditionFunction::dummy),
   ConditionFunction(461, ConditionFunction::dummy),
   ConditionFunction(462, ConditionFunction::dummy),
   ConditionFunction(463, "IsPlayerGrabbedRef", "Returns 1 if the player is Z-keying the specified object, or 0 otherwise.", ConditionArgTypes::ObjectReference),
   ConditionFunction(464, ConditionFunction::dummy),
   ConditionFunction(465, "GetKeywordItemCount", "", ConditionArgTypes::Keyword),
   ConditionFunction(466, ConditionFunction::dummy),
   ConditionFunction(467, ConditionFunction::dummy),
   ConditionFunction(468, ConditionFunction::dummy),
   ConditionFunction(469, ConditionFunction::dummy),
   ConditionFunction(470, "GetDestructionStage", ""),
   ConditionFunction(471, ConditionFunction::dummy),
   ConditionFunction(472, ConditionFunction::dummy),
   ConditionFunction(473, "GetIsAlignment", "A Fallout leftover. Returns 1 if this actor has the specified karma level, or 0 otherwise.", ConditionArgTypes::Alignment),
   ConditionFunction(474, ConditionFunction::dummy),
   ConditionFunction(475, ConditionFunction::dummy),
   ConditionFunction(476, "IsProtected", "Returns 1 if this actor is flagged as protected, or 0 otherwise."),
   ConditionFunction(477, "GetThreatRatio", "", ConditionArgTypes::Actor),
   ConditionFunction(478, ConditionFunction::dummy),
   ConditionFunction(479, "GetIsUsedItemEquipType", "", ConditionArgTypes::EquipType),
   ConditionFunction(480, ConditionFunction::dummy),
   ConditionFunction(481, ConditionFunction::dummy),
   ConditionFunction(482, ConditionFunction::dummy),
   ConditionFunction(483, ConditionFunction::dummy),
   ConditionFunction(484, ConditionFunction::dummy),
   ConditionFunction(485, ConditionFunction::dummy),
   ConditionFunction(486, ConditionFunction::dummy),
   ConditionFunction(487, "IsCarryable", ""),
   ConditionFunction(488, "GetConcussed", ""),
   ConditionFunction(489, ConditionFunction::dummy),
   ConditionFunction(490, ConditionFunction::dummy),
   ConditionFunction(491, "GetMapMarkerVisible", ""),
   ConditionFunction(492, ConditionFunction::dummy),
   ConditionFunction(493, "PlayerKnows", "", ConditionArgTypes::KnowableForm),
   ConditionFunction(494, "GetPermanentActorValue", "Returns the \"permanent modifier\" value of the specified ActorValue.", ConditionArgTypes::ActorValue),
   ConditionFunction(495, "GetKillingBlowLimb", ""),
   ConditionFunction(496, ConditionFunction::dummy),
   ConditionFunction(497, "CanPayCrimeGold", ""),
   ConditionFunction(498, ConditionFunction::dummy),
   ConditionFunction(499, "GetDaysInJail", ""),
   ConditionFunction(500, "EPAlchemyGetMakingPoison", ""),
   ConditionFunction(501, "EPAlchemyEffectHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(502, ConditionFunction::dummy),
   ConditionFunction(503, "GetAllowWorldInteractions", ""),
   ConditionFunction(504, ConditionFunction::dummy),
   ConditionFunction(505, ConditionFunction::dummy),
   ConditionFunction(506, ConditionFunction::dummy),
   ConditionFunction(507, ConditionFunction::dummy),
   ConditionFunction(508, "GetLastHitCritical", ""),
   ConditionFunction(509, ConditionFunction::dummy),
   ConditionFunction(510, ConditionFunction::dummy),
   ConditionFunction(511, ConditionFunction::dummy),
   ConditionFunction(512, ConditionFunction::dummy),
   ConditionFunction(513, "IsCombatTarget", "", ConditionArgTypes::Actor),
   ConditionFunction(514, ConditionFunction::dummy),
   ConditionFunction(515, "GetVATSRightAreaFree", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(516, "GetVATSLeftAreaFree", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(517, "GetVATSBackAreaFree", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(518, "GetVATSFrontAreaFree", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(519, "GetLockIsBroken", ""),
   ConditionFunction(520, "IsPS3", ""),
   ConditionFunction(521, "IsWin32", ""),
   ConditionFunction(522, "GetVATSRightTargetVisible", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(523, "GetVATSLeftTargetVisible", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(524, "GetVATSBackTargetVisible", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(525, "GetVATSFrontTargetVisible", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(526, ConditionFunction::dummy),
   ConditionFunction(527, ConditionFunction::dummy),
   ConditionFunction(528, "IsInCriticalStage", "", ConditionArgTypes::CriticalStage),
   ConditionFunction(529, ConditionFunction::dummy),
   ConditionFunction(530, "GetXPForNextLevel", ""),
   ConditionFunction(531, ConditionFunction::dummy),
   ConditionFunction(532, ConditionFunction::dummy),
   ConditionFunction(533, "GetInfamy", ""),
   ConditionFunction(534, "GetInfamyViolent", ""),
   ConditionFunction(535, "GetInfamyNonViolent", ""),
   ConditionFunction(536, ConditionFunction::dummy),
   ConditionFunction(537, ConditionFunction::dummy),
   ConditionFunction(538, ConditionFunction::dummy),
   ConditionFunction(539, ConditionFunction::dummy),
   ConditionFunction(540, ConditionFunction::dummy),
   ConditionFunction(541, ConditionFunction::dummy),
   ConditionFunction(542, ConditionFunction::dummy),
   ConditionFunction(543, "GetQuestCompleted", "", ConditionArgTypes::Quest),
   ConditionFunction(544, ConditionFunction::dummy),
   ConditionFunction(545, ConditionFunction::dummy),
   ConditionFunction(546, ConditionFunction::dummy),
   ConditionFunction(547, "IsGoreDisabled", ""),
   ConditionFunction(548, ConditionFunction::dummy),
   ConditionFunction(549, ConditionFunction::dummy),
   ConditionFunction(550, "IsSceneActionComplete", "", ConditionArgTypes::Scene, ConditionArgTypes::Integer), // TODO: the int is probably an action index
   ConditionFunction(551, ConditionFunction::dummy),
   ConditionFunction(552, "GetSpellUsageNum", "", ConditionArgTypes::Spell),
   ConditionFunction(553, ConditionFunction::dummy),
   ConditionFunction(554, "GetActorsInHigh", "Returns the number of actors currently in \"high\" AI processing."),
   ConditionFunction(555, "HasLoaded3D", "Returns 1 if this reference has any 3D loaded, or 0 otherwise."),
   ConditionFunction(556, ConditionFunction::dummy),
   ConditionFunction(557, ConditionFunction::dummy),
   ConditionFunction(558, ConditionFunction::dummy),
   ConditionFunction(559, ConditionFunction::dummy),
   ConditionFunction(560, "HasKeyword", "Returns 1 if this reference's base form has the specified keyword, or 0 otherwise.", ConditionArgTypes::Keyword),
   ConditionFunction(561, "HasRefType", "", ConditionArgTypes::LocRefType),
   ConditionFunction(562, "LocationHasKeyword", "Returns 1 if this location has the specified keyword, or 0 otherwise.", ConditionArgTypes::Keyword),
   ConditionFunction(563, "LocationHasRefType", "", ConditionArgTypes::LocRefType),
   ConditionFunction(564, ConditionFunction::dummy),
   ConditionFunction(565, "GetIsEditorLocation", "", ConditionArgTypes::Location),
   ConditionFunction(566, "GetIsAliasRef", "", ConditionArgTypes::Alias),
   ConditionFunction(567, "GetIsEditorLocAlias", "", ConditionArgTypes::Alias),
   ConditionFunction(568, "IsSprinting", "Returns 1 if this actor is sprinting, or 0 otherwise."),
   ConditionFunction(569, "IsBlocking", "Returns 1 if this actor is blocking, or 0 otherwise."),
   ConditionFunction(570, "HasEquippedSpell", "", ConditionArgTypes::CastingSource),
   ConditionFunction(571, "GetCurrentCastingType", "", ConditionArgTypes::CastingSource),
   ConditionFunction(572, "GetCurrentDeliveryType", "", ConditionArgTypes::CastingSource),
   ConditionFunction(573, ConditionFunction::dummy),
   ConditionFunction(574, "GetAttackState", ""),
   ConditionFunction(575, ConditionFunction::dummy),
   ConditionFunction(576, "GetEventData", "", ConditionFunction::uses_event_data),
   ConditionFunction(577, "IsCloserToAThanB", "Returns 1 if this reference is closer to the first argument than it is to the second argument, or 0 otherwise.", ConditionArgTypes::ObjectReference, ConditionArgTypes::ObjectReference),
   ConditionFunction(578, ConditionFunction::dummy),
   ConditionFunction(579, "GetEquippedShout", "", ConditionArgTypes::Shout),
   ConditionFunction(580, "IsBleedingOut", ""),
   ConditionFunction(581, ConditionFunction::dummy),
   ConditionFunction(582, ConditionFunction::dummy),
   ConditionFunction(583, ConditionFunction::dummy),
   ConditionFunction(584, "GetRelativeAngle", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::Axis),
   ConditionFunction(585, ConditionFunction::dummy),
   ConditionFunction(586, ConditionFunction::dummy),
   ConditionFunction(587, ConditionFunction::dummy),
   ConditionFunction(588, ConditionFunction::dummy),
   ConditionFunction(589, "GetMovementDirection", ""),
   ConditionFunction(590, "IsInScene", ""),
   ConditionFunction(591, "GetRefTypeDeadCount", "", ConditionArgTypes::Location, ConditionArgTypes::LocRefType),
   ConditionFunction(592, "GetRefTypeAliveCount", "", ConditionArgTypes::Location, ConditionArgTypes::LocRefType),
   ConditionFunction(593, ConditionFunction::dummy),
   ConditionFunction(594, "GetIsFlying", ""),
   ConditionFunction(595, "IsCurrentSpell", "", ConditionArgTypes::Spell, ConditionArgTypes::CastingSource),
   ConditionFunction(596, "SpellHasKeyword", "", ConditionArgTypes::CastingSource, ConditionArgTypes::Keyword),
   ConditionFunction(597, "GetEquippedItemType", "", ConditionArgTypes::CastingSource),
   ConditionFunction(598, "GetLocationAliasCleared", "", ConditionArgTypes::Alias),
   ConditionFunction(599, ConditionFunction::dummy),
   ConditionFunction(600, "GetLocAliasRefTypeDeadCount", "", ConditionArgTypes::Alias, ConditionArgTypes::LocRefType),
   ConditionFunction(601, "GetLocAliasRefTypeAliveCount", "", ConditionArgTypes::Alias, ConditionArgTypes::LocRefType),
   ConditionFunction(602, "IsWardState", "", ConditionArgTypes::WardState),
   ConditionFunction(603, "IsInSameCurrentLocAsRef", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::Keyword),
   ConditionFunction(604, "IsInSameCurrentLocAsRefAlias", "", ConditionArgTypes::Alias, ConditionArgTypes::Keyword),
   ConditionFunction(605, "LocAliasIsLocation", "", ConditionArgTypes::Alias, ConditionArgTypes::Location),
   ConditionFunction(606, "GetKeywordDataForLocation", "", ConditionArgTypes::Location, ConditionArgTypes::Keyword),
   ConditionFunction(607, ConditionFunction::dummy),
   ConditionFunction(608, "GetKeywordDataForAloas", "", ConditionArgTypes::Alias, ConditionArgTypes::Keyword),
   ConditionFunction(609, ConditionFunction::dummy),
   ConditionFunction(610, "LocAliasHasKeyword", "", ConditionArgTypes::Alias, ConditionArgTypes::Keyword),
   ConditionFunction(611, "IsNullPackageData", "", ConditionArgTypes::PackageData),
   ConditionFunction(612, "GetNumericPackageData", "", ConditionArgTypes::Integer), // TODO: verify
   ConditionFunction(613, "IsFurnitureAnimType", "", ConditionArgTypes::FurnitureAnim),
   ConditionFunction(614, "IsFurnitureEntryType", "", ConditionArgTypes::FurnitureEntry),
   ConditionFunction(615, "GetHighestRelationshipRank", ""),
   ConditionFunction(616, "GetLowestRelationshipRank", ""),
   ConditionFunction(617, "HasAssociationTypeAny", "", ConditionArgTypes::AssociationType),
   ConditionFunction(618, "HasFamilyRelationshipAny", ""),
   ConditionFunction(619, "GetPathingTargetOffset", "", ConditionArgTypes::Axis),
   ConditionFunction(620, "GetPathingTargetAngleOffset", "", ConditionArgTypes::Axis),
   ConditionFunction(621, "GetPathingTargetSpeed", ""),
   ConditionFunction(622, "GetPathingTargetSpeedAngle", "", ConditionArgTypes::Axis),
   ConditionFunction(623, "GetMovementSpeed", ""),
   ConditionFunction(624, "GetInContainer", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(625, "IsLocationLoaded", "", ConditionArgTypes::Location),
   ConditionFunction(626, "IsLocAliasLoaded", "", ConditionArgTypes::Alias),
   ConditionFunction(627, "IsDualCasting", ""),
   ConditionFunction(628, ConditionFunction::dummy),
   ConditionFunction(629, "GetVMQuestVariable", "", ConditionArgTypes::Quest, ConditionArgTypes::String),
   ConditionFunction(630, "GetVMScriptVariable", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::String),
   ConditionFunction(631, "IsEnteringInteractionQuick", ""),
   ConditionFunction(632, "IsCasting", ""),
   ConditionFunction(633, "GetFlyingState", ""),
   ConditionFunction(634, ConditionFunction::dummy),
   ConditionFunction(635, "IsInFavorState", ""),
   ConditionFunction(636, "HasTwoHandedWeaponEquipped", ""),
   ConditionFunction(637, "IsExitingInstant", ""),
   ConditionFunction(638, "IsInFriendStateWithPlayer", ""),
   ConditionFunction(639, "GetWithinDistance", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::Float),
   ConditionFunction(640, "GetActorValuePercent", "", ConditionArgTypes::ActorValue),
   ConditionFunction(641, "IsUnique", ""),
   ConditionFunction(642, "GetLastBumpDirection", ""),
   ConditionFunction(643, ConditionFunction::dummy),
   ConditionFunction(644, "IsInFurnitureState", "", ConditionArgTypes::FurnitureAnim),
   ConditionFunction(645, "GetIsInjured", ""),
   ConditionFunction(646, "GetIsCrashLandRequest", ""),
   ConditionFunction(647, "GetIsHastyLandRequest", ""),
   ConditionFunction(648, ConditionFunction::dummy),
   ConditionFunction(649, ConditionFunction::dummy),
   ConditionFunction(650, "IsLinkedTo", "", ConditionArgTypes::ObjectReference, ConditionArgTypes::Keyword),
   ConditionFunction(651, "GetKeywordDataForCurrentLocation", "", ConditionArgTypes::Keyword),
   ConditionFunction(652, "GetInSharedCrimeFaction", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(653, ConditionFunction::dummy),
   ConditionFunction(654, "GetBribeSuccess", ""),
   ConditionFunction(655, "GetIntimidateSuccess", ""),
   ConditionFunction(656, "GetArrestedState", ""),
   ConditionFunction(657, "GetArrestingActor", ""), // TODO: verify
   ConditionFunction(658, ConditionFunction::dummy),
   ConditionFunction(659, "EPTemperingItemIsEnchanted", ""),
   ConditionFunction(660, "EPTemperingItemHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(661, ConditionFunction::dummy),
   ConditionFunction(662, ConditionFunction::dummy),
   ConditionFunction(663, ConditionFunction::dummy),
   ConditionFunction(664, "GetReplacedItemType", "", ConditionArgTypes::CastingSource),
   ConditionFunction(665, ConditionFunction::dummy),
   ConditionFunction(666, ConditionFunction::dummy),
   ConditionFunction(667, ConditionFunction::dummy),
   ConditionFunction(668, ConditionFunction::dummy),
   ConditionFunction(669, ConditionFunction::dummy),
   ConditionFunction(670, ConditionFunction::dummy),
   ConditionFunction(671, ConditionFunction::dummy),
   ConditionFunction(672, "IsAttacking", ""),
   ConditionFunction(673, "IsPowerAttacking", ""),
   ConditionFunction(674, "IsLastHostileActor", ""),
   ConditionFunction(675, "GetGraphVariableInt", "", ConditionArgTypes::String),
   ConditionFunction(676, "GetCurrentShoutVariation", ""),
   ConditionFunction(677, ConditionFunction::dummy),
   ConditionFunction(678, "ShouldAttackKill", "", ConditionArgTypes::Actor),
   ConditionFunction(679, ConditionFunction::dummy),
   ConditionFunction(680, ConditionFunction::dummy),
   ConditionFunction(681, "EPMagic_IsAdvanceSkill", "", ConditionArgTypes::ActorValue),
   ConditionFunction(682, "WornHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(683, "GetPathingCurrentSpeed", ""),
   ConditionFunction(684, "GetPathingCurrentSpeedAngle", "", ConditionArgTypes::Axis),
   ConditionFunction(685, ConditionFunction::dummy),
   ConditionFunction(686, ConditionFunction::dummy),
   ConditionFunction(687, ConditionFunction::dummy),
   ConditionFunction(688, ConditionFunction::dummy),
   ConditionFunction(689, ConditionFunction::dummy),
   ConditionFunction(690, ConditionFunction::dummy),
   ConditionFunction(691, "EPModSkillUsage_AdvanceObjectHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(692, "EPModSkillUsage_IsAdvanceAction", "", ConditionArgTypes::AdvanceAction),
   ConditionFunction(693, "EPMagic_SpellHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(694, "GetNoBleedoutRecovery", ""),
   ConditionFunction(695, ConditionFunction::dummy),
   ConditionFunction(696, "EPMagic_SpellHasSkill", "", ConditionArgTypes::ActorValue),
   ConditionFunction(697, "IsAttackType", "", ConditionArgTypes::Keyword),
   ConditionFunction(698, "IsAllowedToFly", ""),
   ConditionFunction(699, "HasMagicEffectKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(700, "IsCommandedActor", ""),
   ConditionFunction(701, "IsStaggered", ""),
   ConditionFunction(702, "IsRecoiling", ""),
   ConditionFunction(703, "IsExitingInteractionQuick", ""),
   ConditionFunction(704, "IsPathing", ""),
   ConditionFunction(705, "GetShouldHelp", "", ConditionArgTypes::Actor),
   ConditionFunction(706, "HasBoundWeaponEquipped", "", ConditionArgTypes::CastingSource),
   ConditionFunction(707, "GetCombatTargetHasKeyword", "", ConditionArgTypes::Keyword),
   ConditionFunction(708, ConditionFunction::dummy),
   ConditionFunction(709, "GetCombatGroupMemberCount", ""),
   ConditionFunction(710, "IsIgnoringCombat", ""),
   ConditionFunction(711, "GetLightLevel", ""),
   ConditionFunction(712, ConditionFunction::dummy),
   ConditionFunction(713, "SpellHasCastingPerk", "", ConditionArgTypes::Perk),
   ConditionFunction(714, "IsBeingRidden", ""),
   ConditionFunction(715, "IsUndead", ""),
   ConditionFunction(716, "GetRealHoursPassed", ""),
   ConditionFunction(717, ConditionFunction::dummy),
   ConditionFunction(718, "IsUnlockedDoor", ""),
   ConditionFunction(719, "IsHostileToActor", "", ConditionArgTypes::Actor),
   ConditionFunction(720, "GetTargetHeight", "", ConditionArgTypes::ObjectReference),
   ConditionFunction(721, "IsPoison", ""),
   ConditionFunction(722, "WornApparelHasKeywordCount", "", ConditionArgTypes::Keyword),
   ConditionFunction(723, "GetItemHealthPercent", ""),
   ConditionFunction(724, "EffectWasDualCast", ""),
   ConditionFunction(725, "GetKnockedStateEnum", ""),
   ConditionFunction(726, "DoesNotExist", ""),
   ConditionFunction(727, ConditionFunction::dummy),
   ConditionFunction(728, ConditionFunction::dummy),
   ConditionFunction(729, ConditionFunction::dummy),
   ConditionFunction(730, "IsOnFlyingMount", ""),
   ConditionFunction(731, "CanFlyHere", ""),
   ConditionFunction(732, "IsFlyingMountPatrolQueud", ""),
   ConditionFunction(733, "IsFlyingMountFastTravelling", ""),
   ConditionFunction(734, "IsOverencumbered", ""), // TODO: is this SSE?
   ConditionFunction(735, "GetActorWarmth", ""), // TODO: SSE only
};
ConditionFunction extendedConditionFunctions[] = {
   ConditionFunction(1024, "GetSKSEVersion", ""),
   ConditionFunction(1025, "GetSKSEVersionMinor", ""),
   ConditionFunction(1026, "GetSKSEVersionBeta", ""),
   ConditionFunction(1027, "GetSKSERelease", ""),
   ConditionFunction(1028, "ClearInvalidRegistrations", ""),
};

const ConditionFunction* getConditionFunction(uint16_t id) {
   if (id < std::extent<decltype(conditionFunctions)>::value)
      return &conditionFunctions[id];
   for (uint16_t i = 0; i < std::extent<decltype(extendedConditionFunctions)>::value; i++) {
      auto& f = extendedConditionFunctions[i];
      if (f.id == id)
         return &f;
   }
   return nullptr;
}