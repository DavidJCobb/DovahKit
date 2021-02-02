#include "conditions.h"
#include "../../../helpers/strings.h"
#include "../_common_cpp.h"

#include "../Quest.h"

namespace dovah::loaded_forms::components {
   namespace condition_info {
      /*static*/ const function* function::lookup_by_id(uint16_t id) noexcept {
         if (id < function_list.size())
            return &function_list[id];
         for (auto& f : extended_function_list)
            if (f.id == id)
               return &f;
         return nullptr;
      }

      #pragma region Function definitions
      std::array<function, 736> function_list = {{
         function(0, "GetWantBlocking",   ""),
         function(1, "GetDistance",       "Returns the distance between this reference and another reference.", arg_types::ObjectReference),
         function(2, function::dummy), // actually AddItem
         function(3, function::dummy), // actually SetEssential
         function(4, function::dummy), // actually Rotate
         function(5, "GetLocked",         "Returns 1 if this reference is a locked door or container, or 0 otherwise."),
         function(6, "GetPos",            "Returns this reference's position along the given axis.", arg_types::Axis),
         function(7, function::dummy), // actually SetPos
         function(8, "GetAngle",          "Returns this reference's rotation along the given axis.", arg_types::Axis),
         function(9, function::dummy), // actually SetANgle
         function(10, "GetStartingPos",    "Returns this reference's starting position along the given axis.", arg_types::Axis),
         function(11, "GetStartingAngle",  "Returns this reference's starting position along the given axis.", arg_types::Axis),
         function(12, "GetSecondsPassed",  ""),
         function(13, function::dummy), // actually Activate
         function(14, "GetActorValue",     "Returns the current value of the specified ActorValue.", arg_types::ActorValue),
         function(15, function::dummy), // actually SetActorValue
         function(16, function::dummy), // actually ModActorValue
         function(17, function::dummy), // actually SetAtStart
         function(18, "GetCurrentTime",    ""),
         function(19, function::dummy), // actually PlayGroup
         function(20, function::dummy), // actually LoopGroup
         function(21, function::dummy), // actually SkipAnim
         function(22, function::dummy), // actually StartCombat
         function(23, function::dummy), // actually StopCombat
         function(24, "GetScale",          "Returns this reference's scale."),
         function(25, "IsMoving",          ""),
         function(26, "IsTurning",         "Returns 1 if this actor is turning to the left, 2 if they are turning to the right, or 0 otherwise."),
         function(27, "GetLineOfSight",    "", arg_types::ObjectReference),
         function(28, function::dummy), // actually AddSpell
         function(29, function::dummy), // actually RemoveSpell
         function(30, function::dummy), // actualy Cast
         function(31, function::dummy), // actually GetButtonPressed
         function(32, "GetInSameCell",     "Returns 1 if this reference is in the same cell as the specified reference, or 0 otherwise.", arg_types::ObjectReference),
         function(33, function::dummy), // actually Enable
         function(34, function::dummy), // actually Disable
         function(35, "GetDisabled",       "Returns 1 if this reference is disabled, or 0 otherwise."),
         function(36, "MenuMode",          "", arg_types::Integer),
         function(37, function::dummy), // actually PlaceAtMe
         function(38, function::dummy), // actually PlaySound
         function(39, "GetDisease",        "Returns 1 if any of this actor's active magic effects came from a spell whose type was set to \"Disease,\" or 0 otherwise."),
         function(40, function::dummy), // actually FailAllObjectives
         function(41, "GetClothingValue",  ""),
         function(42, "SameFaction",       "Returns 1 if this reference is in the same faction as the specified actor, or 0 otherwise.", arg_types::Actor),
         function(43, "SameRace",          "Returns 1 if this reference is of the same race as the specified actor, or 0 otherwise.", arg_types::Actor),
         function(44, "SameSex",           "Returns 1 if this reference is of the same sex as the specified actor, or 0 otherwise.", arg_types::Actor),
         function(45, "GetDetected",       "", arg_types::Actor),
         function(46, "GetDead",           ""),
         function(47, "GetItemCount",      "Returns how many of the specified form this reference has in its inventory.", arg_types::InventoryItem),
         function(48, "GetGold",           "Returns how much gold this reference is carrying."),
         function(49, "GetSleeping",       ""),
         function(50, "GetTalkedToPC",     ""),
         function(51, function::dummy), // actually Say
         function(52, function::dummy), // actually SayTo
         function(53, "GetScriptVariable",  "", arg_types::ObjectReference, arg_types::String),
         function(54, function::dummy), // actually StartQuest
         function(55, function::dummy), // actually StopQuest
         function(56, "GetQuestRunning",    "Returns 1 if the specified quest is running, or 0 otherwise.", arg_types::Quest),
         function(57, function::dummy), // actually SetStage
         function(58, "GetStage",           "Returns the specified quest's current stage number.", arg_types::Quest),
         function(59, "GetStageDone",       "Returns 1 if the specified quest stage is complete, or 0 otherwise.", arg_types::Quest, arg_types::QuestStage),
         function(60, "GetFactionRankDifference", "", arg_types::Faction, arg_types::Actor),
         function(61, "GetAlarmed",         ""),
         function(62, "IsRaining",          "Returns 1 if the current weather is rainy, or 0 otherwise."),
         function(63, "GetAttacked",        ""),
         function(64, "GetIsCreature",      ""),
         function(65, "GetLockLevel",       ""),
         function(66, "GetShouldAttack",    "", arg_types::Actor),
         function(67, "GetInCell",          "Returns 1 if this reference is in the specified cell, or 0 otherwise.", arg_types::Cell),
         function(68, "GetIsClass",         "Returns 1 if this actor is of the specified class, or 0 otherwise.", arg_types::Class),
         function(69, "GetIsRace",          "Returns 1 if this actor is of the specified race, or 0 otherwise.",  arg_types::Race),
         function(70, "GetIsSex",           "Returns 1 if this actor is of the specified sex, or 0 otherwise.",   arg_types::Sex),
         function(71, "GetInFaction",       "", arg_types::Faction),
         function(72, "GetIsID",            "", arg_types::BaseForm),
         function(73, "GetFactionRank",     "", arg_types::Faction),
         function(74, "GetGlobalValue",     "Returns the value of the specified Global.", arg_types::Global),
         function(75, "IsSnowing",          "Returns 1 if the current weather is snowy, or 0 otherwise."),
         function(76, function::dummy), // actually FastTravel
         function(77, "GetRandomPercent",   "Returns a random number between 0 and 100, inclusive."),
         function(78, function::dummy), // actually RemoveMusic
         function(79, "GetQuestVariable",   "", arg_types::Quest, arg_types::String),
         function(80, "GetLevel",           "Returns this actor's level."),
         function(81, "IsRotating",         ""),
         function(82, function::dummy), // actually RemoveItem
         function(83, function::dummy), // actually GetLeveledEncounterValue
         function(84, "GetDeadCount",       "Returns the number of times this character has died.", arg_types::ActorBase),
         function(85, function::dummy), // actually AddToMap
         function(86, function::dummy), // actually StartConversation
         function(87, function::dummy), // actually Drop
         function(88, function::dummy), // actually AddTopic
         function(89, function::dummy), // actually ShowMessage
         function(90, function::dummy), // actually SetAlert
         function(91, "GetIsAlerted", ""),
         function(92, function::dummy), // actually Look
         function(93, function::dummy), // actually StopLook
         function(94, function::dummy), // actually EvaluatePackage
         function(95, function::dummy), // actually SendAssaultAlarm
         function(96, function::dummy), // actually EnablePlayerControls
         function(97, function::dummy), // actually DisablePlayerControls
         function(98, "GetPlayerControlsDisabled", "", arg_types::Integer, arg_types::Integer),
         function(99, "GetHeadingAngle", "", arg_types::ObjectReference),
         function(100, function::dummy), // actually PickIdle
         function(101, "IsWeaponMagicOut", ""),
         function(102, "IsTorchOut", ""),
         function(103, "IsShieldOut", ""),
         function(104, function::dummy), // actually CreateDetectionEvent
         function(105, function::dummy), // actually IsActionRef
         function(106, "IsFacingUp", ""),
         function(107, "GetKnockedState", ""),
         function(108, "GetWeaponAnimType", ""),
         function(109, "IsWeaponSkillType", "", arg_types::ActorValue),
         function(110, "GetCurrentAIPackage", ""),
         function(111, "IsWaiting", ""),
         function(112, "IsIdlePlaying", ""),
         function(113, function::dummy), // actually CompleteQuest
         function(114, function::dummy), // actually Lock
         function(115, function::dummy), // actually Unlock
         function(116, "IsIntimidatedByPlayer", ""),
         function(117, "IsPlayerInRegion", "Returns 1 if this reference's parent cell belongs to or overlaps the specified region, or 0 otherwise."),
         function(118, "GetActorAggroRadiusViolated", ""),
         function(119, function::dummy),
         function(120, function::dummy),
         function(121, function::dummy),
         function(122, "GetCrime", "", arg_types::Actor, arg_types::CrimeType),
         function(123, "IsGreetingPlayer", ""),
         function(124, function::dummy),
         function(125, "IsGuard", ""),
         function(126, function::dummy),
         function(127, "HasBeenEaten", "Returns 1 if this actor has been fed on by a cannibal or werewolf, or 0 otherwise."),
         function(128, "GetStaminaPercentage", ""),
         function(129, "GetPCIsClass", "Returns 1 if the player-character is of the specified class, or 0 otherwise.", arg_types::Class),
         function(130, "GetPCIsRace", "Returns 1 if the player-character is of the specified race, or 0 otherwise.", arg_types::Race),
         function(131, "GetPCIsSex", "Returns 1 if the player-character is of the specified sex, or 0 otherwise.", arg_types::Sex),
         function(132, "GetPCInFaction", "", arg_types::Faction),
         function(133, "SameFactionAsPC", ""),
         function(134, "SameRaceAsPC", "Returns 1 if this actor is of the same race as the player-character, or 0 otherwise."),
         function(135, "SameSexAsPC", "Returns 1 if this actor is of the same sex as the player-character, or 0 otherwise."),
         function(136, "GetIsReference", "Returns 1 if this reference is the specified reference, or 0 otherwise.", arg_types::ObjectReference),
         function(137, function::dummy),
         function(138, function::dummy),
         function(139, function::dummy),
         function(140, function::dummy),
         function(141, "IsTalking", ""),
         function(142, "GetWalkSpeed", ""),
         function(143, "GetCurrentAIProcedure", ""),
         function(144, "GetTrespassWarningLevel", ""),
         function(145, "IsTrespassing", ""),
         function(146, "IsInMyOwnedCell", ""),
         function(147, "GetWindSpeed", ""),
         function(148, "GetCurrentWeatherPercent", ""),
         function(149, "GetIsCurrentWeather", "Returns 1 if the specified weather is the current weather, or 0 otherwise.", arg_types::Weather),
         function(150, "IsContinuingPackagePCNear", ""),
         function(151, function::dummy),
         function(152, "GetIsCrimeFaction", "", arg_types::Faction),
         function(153, "CanHaveFlames", ""),
         function(154, "HasFlames", ""),
         function(155, function::dummy),
         function(156, function::dummy),
         function(157, "GetOpenState", ""),
         function(158, function::dummy),
         function(159, "GetSitting", ""),
         function(160, function::dummy),
         function(161, "GetIsCurrentPackage", "", arg_types::Package),
         function(162, "IsCurrentFurnitureRef", "", arg_types::ObjectReference),
         function(163, "IsCurrentFurnitureObj", "", arg_types::Furniture),
         function(164, function::dummy),
         function(165, function::dummy),
         function(166, function::dummy),
         function(167, function::dummy),
         function(168, function::dummy),
         function(169, function::dummy),
         function(170, "GetDayOfWeek", ""),
         function(171, function::dummy),
         function(172, "GetTalkedToPCParam", "", arg_types::Actor),
         function(173, function::dummy),
         function(174, function::dummy),
         function(175, "IsPCSleeping", ""),
         function(176, "IsPCAMurderer", ""),
         function(177, function::dummy),
         function(178, function::dummy),
         function(179, function::dummy),
         function(180, "HasSameEditorLocAsRef", "", arg_types::ObjectReference, arg_types::Keyword),
         function(181, "HasSameEditorLocAsRefAlias", "", arg_types::Alias, arg_types::Keyword),
         function(182, "GetEquipped", "", arg_types::InventoryItem),
         function(183, function::dummy),
         function(184, function::dummy),
         function(185, "IsSwimming", ""),
         function(186, function::dummy),
         function(187, function::dummy),
         function(188, function::dummy),
         function(189, function::dummy),
         function(190, "GetAmountGoldStolen", ""),
         function(191, function::dummy),
         function(192, "GetIgnoreCrime", ""),
         function(193, "GetPCExpelled", "", arg_types::Faction),
         function(194, function::dummy),
         function(195, "GetPCFactionMurder", "", arg_types::Faction),
         function(196, function::dummy),
         function(197, "GetPCEnemyofFaction", "", arg_types::Faction),
         function(198, function::dummy),
         function(199, "GetPCFactionAttack", "", arg_types::Faction),
         function(200, function::dummy),
         function(201, function::dummy),
         function(202, function::dummy),
         function(203, "GetDestroyed", "Returns 1 if this reference is destroyed, or 0 otherwise."),
         function(204, function::dummy),
         function(205, function::dummy),
         function(206, function::dummy),
         function(207, function::dummy),
         function(208, function::dummy),
         function(209, function::dummy),
         function(210, function::dummy),
         function(211, function::dummy),
         function(212, function::dummy),
         function(213, function::dummy),
         function(214, "HasMagicEffect", "", arg_types::MagicEffect),
         function(215, "GetDefaultOpen", ""),
         function(216, function::dummy),
         function(217, function::dummy),
         function(218, function::dummy),
         function(219, "GetAnimAction", ""),
         function(220, function::dummy),
         function(221, function::dummy),
         function(222, function::dummy),
         function(223, "IsSpellTarget", "", arg_types::Spell),
         function(224, "GetVATSMode", ""),
         function(225, "GetPersuasionNumber", ""),
         function(226, "GetVampireFeed", "Returns 1 if this actor is a vampire currently feeding on another actor, or 0 otherwise."),
         function(227, "GetCannibal", "Returns 1 if this actor is currently a cannibal feeding on another actor, or 0 otherwise."),
         function(228, "GetIsClassDefault", "", arg_types::Class),
         function(229, "GetClassDefaultMatch", ""),
         function(230, "GetInCellParam", "Returns 1 if the specified reference is inside of the specified cell, or 0 otherwise.", arg_types::Cell, arg_types::ObjectReference),
         function(231, function::dummy),
         function(232, function::dummy),
         function(233, function::dummy),
         function(234, function::dummy),
         function(235, "GetVatsTargetHeight", ""),
         function(236, function::dummy),
         function(237, "GetIsGhost", ""),
         function(238, function::dummy),
         function(239, function::dummy),
         function(240, function::dummy),
         function(241, function::dummy),
         function(242, "GetUnconscious", "Returns 1 if this actor is unconscious, or 0 otherwise."),
         function(243, function::dummy),
         function(244, "GetRestrained", "Returns 1 if this actor is restrained, or 0 otherwise."),
         function(245, function::dummy),
         function(246, "GetIsUsedItem", "", arg_types::BaseForm),
         function(247, "GetIsUsedItemType", "", arg_types::FormType),
         function(248, "IsScenePlaying", "", arg_types::Scene),
         function(249, "IsInDialogueWithPlayer", ""),
         function(250, "GetLocationCleared", "", arg_types::Location),
         function(251, function::dummy),
         function(252, function::dummy),
         function(253, function::dummy),
         function(254, "GetIsPlayableRace", ""),
         function(255, "GetOffersServicesNow", "Returns 1 if this actor is currently available to barter with, or 0 otherwise."),
         function(256, function::dummy),
         function(257, function::dummy),
         function(258, "HasAssociationType", "", arg_types::Actor, arg_types::AssociationType),
         function(259, "HasFamilyRelationship", "", arg_types::Actor),
         function(260, function::dummy),
         function(261, "HasParentRelationship", "", arg_types::Actor),
         function(262, "IsWarningAbout", "", arg_types::FormList),
         function(263, "IsWeaponOut", ""),
         function(264, "HasSpell", "", arg_types::Spell),
         function(265, "IsTimePassing", ""),
         function(266, "IsPleasant", "Returns 1 if the current weather is pleasant, or 0 otherwise."),
         function(267, "IsCloudy", "Returns 1 if the current weather is cloudy, or 0 otherwise."),
         function(268, function::dummy),
         function(269, function::dummy),
         function(270, function::dummy),
         function(271, function::dummy),
         function(272, function::dummy),
         function(273, function::dummy),
         function(274, "IsSmallBump", ""),
         function(275, function::dummy),
         function(276, function::dummy),
         function(277, "GetBaseActorValue", "Returns the base value of the specified ActorValue.", arg_types::ActorValue),
         function(278, "IsOwner", "", arg_types::OwnerForm),
         function(279, function::dummy),
         function(280, "IsCellOwner", "Returns 1 if the specified actor or faction owns the specified cell, or 0 otherwise.", arg_types::Cell, arg_types::OwnerForm),
         function(281, function::dummy),
         function(282, "IsHorseStolen", ""),
         function(283, function::dummy),
         function(284, function::dummy),
         function(285, "IsLeftUp", ""),
         function(286, "IsSneaking", "Returns 1 if this actor is in sneak mode, or 0 otherwise."),
         function(287, "IsRunning", ""),
         function(288, "GetFriendHit", ""),
         function(289, "IsInCombat", "", arg_types::Integer),
         function(290, function::dummy),
         function(291, function::dummy),
         function(292, function::dummy),
         function(293, function::dummy),
         function(294, function::dummy),
         function(295, function::dummy),
         function(296, function::dummy),
         function(297, function::dummy),
         function(298, function::dummy),
         function(299, function::dummy),
         function(300, "IsInInterior", "Returns 1 if this reference is in an interior cell, or 0 otherwise."),
         function(301, function::dummy),
         function(302, function::dummy),
         function(303, function::dummy),
         function(304, "IsWaterObject", ""),
         function(305, "GetPlayerAction", ""),
         function(306, "IsActorUsingATorch", ""),
         function(307, function::dummy),
         function(308, function::dummy),
         function(309, "IsXBox", ""),
         function(310, "GetInWorldspace", "", arg_types::Worldspace),
         function(311, function::dummy),
         function(312, "GetPCMiscStat", "Returns the value of the specified misc stat.", arg_types::MiscStat),
         function(313, "GetPairedAnimation", ""),
         function(314, "IsActorAVictim", ""),
         function(315, "GetTotalPersuasionNumber", ""),
         function(316, function::dummy),
         function(317, function::dummy),
         function(318, "GetIdleDoneOnce", ""),
         function(319, function::dummy),
         function(320, "GetNoRumors", ""),
         function(321, function::dummy),
         function(322, function::dummy),
         function(323, "GetCombatState", ""),
         function(324, function::dummy),
         function(325, "GetWithinPackageLocation", "", arg_types::PackageData),
         function(326, function::dummy),
         function(327, "IsRidingMount", ""),
         function(328, function::dummy),
         function(329, "IsFleeing", ""),
         function(330, function::dummy),
         function(331, function::dummy),
         function(332, "IsInDangerousWater", "Returns 1 if this actor is standing or swimming in a body of water that has been flagged as dangerous, or 0 otherwise."),
         function(333, function::dummy),
         function(334, function::dummy),
         function(335, function::dummy),
         function(336, function::dummy),
         function(337, function::dummy),
         function(338, "GetIgnoreFriendlyHits", ""),
         function(339, "IsPlayersLastRiddenMount", ""),
         function(340, function::dummy),
         function(341, function::dummy),
         function(342, function::dummy),
         function(343, function::dummy),
         function(344, function::dummy),
         function(345, function::dummy),
         function(346, function::dummy),
         function(347, function::dummy),
         function(348, function::dummy),
         function(349, function::dummy),
         function(350, function::dummy),
         function(351, function::dummy),
         function(352, function::dummy),
         function(353, "IsActor", "Returns 1 if this reference is an actor, or 0 otherwise."),
         function(354, "IsEssential", "Returns 1 if this actor is flagged as essential, or 0 otherwise."),
         function(355, function::dummy),
         function(356, function::dummy),
         function(357, function::dummy),
         function(358, "IsPlayerMovingIntoNewSpace", ""),
         function(359, "GetInCurrentLoc", "", arg_types::Location),
         function(360, "GetInCurrentLocAlias", "", arg_types::Alias),
         function(361, "GetTimeDead", ""),
         function(362, "HasLinkedRef", "", arg_types::Keyword),
         function(363, function::dummy),
         function(364, function::dummy),
         function(365, "IsChild", ""),
         function(366, "GetStolenItemValueNoCrime", "", arg_types::Faction),
         function(367, "GetLastPlayerAction", ""),
         function(368, "IsPlayerActionActive", "", arg_types::Integer), // arg type needs verification
         function(369, function::dummy),
         function(370, "IsTalkingActivatorActor", "", arg_types::Actor),
         function(371, function::dummy),
         function(372, "IsInList", "", arg_types::FormList),
         function(373, "GetStolenItemValue", "", arg_types::Faction),
         function(374, function::dummy),
         function(375, "GetCrimeGoldViolent", ""),
         function(376, "GetCrimeGoldNonViolent", ""),
         function(377, function::dummy),
         function(378, "HasShout", "", arg_types::Shout),
         function(379, function::dummy),
         function(380, function::dummy),
         function(381, "GetHasNote", "", arg_types::Integer), // arg type needs verification
         function(382, function::dummy),
         function(383, function::dummy),
         function(384, function::dummy),
         function(385, function::dummy),
         function(386, function::dummy),
         function(387, function::dummy),
         function(388, function::dummy),
         function(389, function::dummy),
         function(390, "GetHitLocation", ""),
         function(391, "IsPC1stPerson", ""),
         function(392, function::dummy),
         function(393, function::dummy),
         function(394, function::dummy),
         function(395, function::dummy),
         function(396, "GetCauseofDeath", ""),
         function(397, "IsLimbGone", "", arg_types::Integer), // CK actually does expose this as an int
         function(398, "IsWeaponInList", "", arg_types::FormList),
         function(399, function::dummy),
         function(400, function::dummy),
         function(401, function::dummy),
         function(402, "IsBribedByPlayer", ""),
         function(403, "GetRelationshipRank", "", arg_types::ObjectReference),
         function(404, function::dummy),
         function(405, function::dummy),
         function(406, function::dummy),
         function(407, "GetVATSValue", "", arg_types::VATSValueFunction, arg_types::VATSValue),
         function(408, "IsKiller", "", arg_types::Actor),
         function(409, "IsKillerObject", "", arg_types::FormList),
         function(410, "GetFactionCombatReaction", "", arg_types::Faction, arg_types::Faction),
         function(411, function::dummy),
         function(412, function::dummy),
         function(413, function::dummy),
         function(414, "Exists", "Returns 1 if this reference is the specified reference and if the specified reference exists, or 0 otherwise.", arg_types::ObjectReference),
         function(415, "GetGroupMemberCount", ""),
         function(416, "GetGroupTargetCount", ""),
         function(417, function::dummy),
         function(418, function::dummy),
         function(419, function::dummy),
         function(420, function::dummy),
         function(421, function::dummy),
         function(422, function::dummy),
         function(423, function::dummy),
         function(424, function::dummy),
         function(425, function::dummy),
         function(426, "GetIsVoiceType", "Returns 1 if this actor uses the specified voicetype, or 0 otherwise.", arg_types::Voicetype),
         function(427, "GetPlantedExplosive", ""),
         function(428, function::dummy),
         function(429, "IsScenePackageRunning", ""),
         function(430, "GetHealthPercentage", ""),
         function(431, function::dummy),
         function(432, "GetIsObjectType", "", arg_types::FormType),
         function(433, function::dummy),
         function(434, "GetDialogueEmotion", ""),
         function(435, "GetDialogueEmotionValue", ""),
         function(436, function::dummy),
         function(437, "GetIsCreatureType", "", arg_types::Integer), // double-check this
         function(438, function::dummy),
         function(439, function::dummy),
         function(440, function::dummy),
         function(441, function::dummy),
         function(442, function::dummy),
         function(443, function::dummy),
         function(444, "GetInCurrentLocFormList", "", arg_types::FormList),
         function(445, "GetInZone", "", arg_types::EncounterZone),
         function(446, "GetVelocity", "", arg_types::Axis),
         function(447, "GetGraphVariableFloat", "", arg_types::String),
         function(448, "HasPerk", "", arg_types::Perk, arg_types::Integer), // second arg type is not known
         function(449, "GetFactionRelation", "", arg_types::Actor),
         function(450, "IsLastIdlePlayed", "", arg_types::Idle),
         function(451, function::dummy),
         function(452, function::dummy),
         function(453, "GetPlayerTeammate", ""),
         function(454, "GetPlayerTeammateCount", ""),
         function(455, function::dummy),
         function(456, function::dummy),
         function(457, function::dummy),
         function(458, "GetActorCrimePlayerEnemy", ""),
         function(459, "GetCrimeGold", ""),
         function(460, function::dummy),
         function(461, function::dummy),
         function(462, function::dummy),
         function(463, "IsPlayerGrabbedRef", "Returns 1 if the player is Z-keying the specified object, or 0 otherwise.", arg_types::ObjectReference),
         function(464, function::dummy),
         function(465, "GetKeywordItemCount", "", arg_types::Keyword),
         function(466, function::dummy),
         function(467, function::dummy),
         function(468, function::dummy),
         function(469, function::dummy),
         function(470, "GetDestructionStage", ""),
         function(471, function::dummy),
         function(472, function::dummy),
         function(473, "GetIsAlignment", "A Fallout leftover. Returns 1 if this actor has the specified karma level, or 0 otherwise.", arg_types::Alignment),
         function(474, function::dummy),
         function(475, function::dummy),
         function(476, "IsProtected", "Returns 1 if this actor is flagged as protected, or 0 otherwise."),
         function(477, "GetThreatRatio", "", arg_types::Actor),
         function(478, function::dummy),
         function(479, "GetIsUsedItemEquipType", "", arg_types::EquipType),
         function(480, function::dummy),
         function(481, function::dummy),
         function(482, function::dummy),
         function(483, function::dummy),
         function(484, function::dummy),
         function(485, function::dummy),
         function(486, function::dummy),
         function(487, "IsCarryable", ""),
         function(488, "GetConcussed", ""),
         function(489, function::dummy),
         function(490, function::dummy),
         function(491, "GetMapMarkerVisible", ""),
         function(492, function::dummy),
         function(493, "PlayerKnows", "", arg_types::KnowableForm),
         function(494, "GetPermanentActorValue", "Returns the \"permanent modifier\" value of the specified ActorValue.", arg_types::ActorValue),
         function(495, "GetKillingBlowLimb", ""),
         function(496, function::dummy),
         function(497, "CanPayCrimeGold", ""),
         function(498, function::dummy),
         function(499, "GetDaysInJail", ""),
         function(500, "EPAlchemyGetMakingPoison", ""),
         function(501, "EPAlchemyEffectHasKeyword", "", arg_types::Keyword),
         function(502, function::dummy),
         function(503, "GetAllowWorldInteractions", ""),
         function(504, function::dummy),
         function(505, function::dummy),
         function(506, function::dummy),
         function(507, function::dummy),
         function(508, "GetLastHitCritical", ""),
         function(509, function::dummy),
         function(510, function::dummy),
         function(511, function::dummy),
         function(512, function::dummy),
         function(513, "IsCombatTarget", "", arg_types::Actor),
         function(514, function::dummy),
         function(515, "GetVATSRightAreaFree", "", arg_types::ObjectReference),
         function(516, "GetVATSLeftAreaFree", "", arg_types::ObjectReference),
         function(517, "GetVATSBackAreaFree", "", arg_types::ObjectReference),
         function(518, "GetVATSFrontAreaFree", "", arg_types::ObjectReference),
         function(519, "GetLockIsBroken", ""),
         function(520, "IsPS3", ""),
         function(521, "IsWin32", ""),
         function(522, "GetVATSRightTargetVisible", "", arg_types::ObjectReference),
         function(523, "GetVATSLeftTargetVisible", "", arg_types::ObjectReference),
         function(524, "GetVATSBackTargetVisible", "", arg_types::ObjectReference),
         function(525, "GetVATSFrontTargetVisible", "", arg_types::ObjectReference),
         function(526, function::dummy),
         function(527, function::dummy),
         function(528, "IsInCriticalStage", "", arg_types::CriticalStage),
         function(529, function::dummy),
         function(530, "GetXPForNextLevel", ""),
         function(531, function::dummy),
         function(532, function::dummy),
         function(533, "GetInfamy", ""),
         function(534, "GetInfamyViolent", ""),
         function(535, "GetInfamyNonViolent", ""),
         function(536, function::dummy),
         function(537, function::dummy),
         function(538, function::dummy),
         function(539, function::dummy),
         function(540, function::dummy),
         function(541, function::dummy),
         function(542, function::dummy),
         function(543, "GetQuestCompleted", "", arg_types::Quest),
         function(544, function::dummy),
         function(545, function::dummy),
         function(546, function::dummy),
         function(547, "IsGoreDisabled", ""),
         function(548, function::dummy),
         function(549, function::dummy),
         function(550, "IsSceneActionComplete", "", arg_types::Scene, arg_types::Integer), // TODO: the int is probably an action index
         function(551, function::dummy),
         function(552, "GetSpellUsageNum", "", arg_types::Spell),
         function(553, function::dummy),
         function(554, "GetActorsInHigh", "Returns the number of actors currently in \"high\" AI processing."),
         function(555, "HasLoaded3D", "Returns 1 if this reference has any 3D loaded, or 0 otherwise."),
         function(556, function::dummy),
         function(557, function::dummy),
         function(558, function::dummy),
         function(559, function::dummy),
         function(560, "HasKeyword", "Returns 1 if this reference's base form has the specified keyword, or 0 otherwise.", arg_types::Keyword),
         function(561, "HasRefType", "", arg_types::LocRefType),
         function(562, "LocationHasKeyword", "Returns 1 if this location has the specified keyword, or 0 otherwise.", arg_types::Keyword),
         function(563, "LocationHasRefType", "", arg_types::LocRefType),
         function(564, function::dummy),
         function(565, "GetIsEditorLocation", "", arg_types::Location),
         function(566, "GetIsAliasRef", "", arg_types::Alias),
         function(567, "GetIsEditorLocAlias", "", arg_types::Alias),
         function(568, "IsSprinting", "Returns 1 if this actor is sprinting, or 0 otherwise."),
         function(569, "IsBlocking", "Returns 1 if this actor is blocking, or 0 otherwise."),
         function(570, "HasEquippedSpell", "", arg_types::CastingSource),
         function(571, "GetCurrentCastingType", "", arg_types::CastingSource),
         function(572, "GetCurrentDeliveryType", "", arg_types::CastingSource),
         function(573, function::dummy),
         function(574, "GetAttackState", ""),
         function(575, function::dummy),
         function(576, "GetEventData", "", function::function_uses_event_data),
         function(577, "IsCloserToAThanB", "Returns 1 if this reference is closer to the first argument than it is to the second argument, or 0 otherwise.", arg_types::ObjectReference, arg_types::ObjectReference),
         function(578, function::dummy),
         function(579, "GetEquippedShout", "", arg_types::Shout),
         function(580, "IsBleedingOut", ""),
         function(581, function::dummy),
         function(582, function::dummy),
         function(583, function::dummy),
         function(584, "GetRelativeAngle", "", arg_types::ObjectReference, arg_types::Axis),
         function(585, function::dummy),
         function(586, function::dummy),
         function(587, function::dummy),
         function(588, function::dummy),
         function(589, "GetMovementDirection", ""),
         function(590, "IsInScene", ""),
         function(591, "GetRefTypeDeadCount", "", arg_types::Location, arg_types::LocRefType),
         function(592, "GetRefTypeAliveCount", "", arg_types::Location, arg_types::LocRefType),
         function(593, function::dummy),
         function(594, "GetIsFlying", ""),
         function(595, "IsCurrentSpell", "", arg_types::Spell, arg_types::CastingSource),
         function(596, "SpellHasKeyword", "", arg_types::CastingSource, arg_types::Keyword),
         function(597, "GetEquippedItemType", "", arg_types::CastingSource),
         function(598, "GetLocationAliasCleared", "", arg_types::Alias),
         function(599, function::dummy),
         function(600, "GetLocAliasRefTypeDeadCount", "", arg_types::Alias, arg_types::LocRefType),
         function(601, "GetLocAliasRefTypeAliveCount", "", arg_types::Alias, arg_types::LocRefType),
         function(602, "IsWardState", "", arg_types::WardState),
         function(603, "IsInSameCurrentLocAsRef", "", arg_types::ObjectReference, arg_types::Keyword),
         function(604, "IsInSameCurrentLocAsRefAlias", "", arg_types::Alias, arg_types::Keyword),
         function(605, "LocAliasIsLocation", "", arg_types::Alias, arg_types::Location),
         function(606, "GetKeywordDataForLocation", "", arg_types::Location, arg_types::Keyword),
         function(607, function::dummy),
         function(608, "GetKeywordDataForAloas", "", arg_types::Alias, arg_types::Keyword),
         function(609, function::dummy),
         function(610, "LocAliasHasKeyword", "", arg_types::Alias, arg_types::Keyword),
         function(611, "IsNullPackageData", "", arg_types::PackageData),
         function(612, "GetNumericPackageData", "", arg_types::Integer), // TODO: verify
         function(613, "IsFurnitureAnimType", "", arg_types::FurnitureAnim),
         function(614, "IsFurnitureEntryType", "", arg_types::FurnitureEntry),
         function(615, "GetHighestRelationshipRank", ""),
         function(616, "GetLowestRelationshipRank", ""),
         function(617, "HasAssociationTypeAny", "", arg_types::AssociationType),
         function(618, "HasFamilyRelationshipAny", ""),
         function(619, "GetPathingTargetOffset", "", arg_types::Axis),
         function(620, "GetPathingTargetAngleOffset", "", arg_types::Axis),
         function(621, "GetPathingTargetSpeed", ""),
         function(622, "GetPathingTargetSpeedAngle", "", arg_types::Axis),
         function(623, "GetMovementSpeed", ""),
         function(624, "GetInContainer", "", arg_types::ObjectReference),
         function(625, "IsLocationLoaded", "", arg_types::Location),
         function(626, "IsLocAliasLoaded", "", arg_types::Alias),
         function(627, "IsDualCasting", ""),
         function(628, function::dummy),
         function(629, "GetVMQuestVariable", "", arg_types::Quest, arg_types::String),
         function(630, "GetVMScriptVariable", "", arg_types::ObjectReference, arg_types::String),
         function(631, "IsEnteringInteractionQuick", ""),
         function(632, "IsCasting", ""),
         function(633, "GetFlyingState", ""),
         function(634, function::dummy),
         function(635, "IsInFavorState", ""),
         function(636, "HasTwoHandedWeaponEquipped", ""),
         function(637, "IsExitingInstant", ""),
         function(638, "IsInFriendStateWithPlayer", ""),
         function(639, "GetWithinDistance", "", arg_types::ObjectReference, arg_types::Float),
         function(640, "GetActorValuePercent", "", arg_types::ActorValue),
         function(641, "IsUnique", ""),
         function(642, "GetLastBumpDirection", ""),
         function(643, function::dummy),
         function(644, "IsInFurnitureState", "", arg_types::FurnitureAnim),
         function(645, "GetIsInjured", ""),
         function(646, "GetIsCrashLandRequest", ""),
         function(647, "GetIsHastyLandRequest", ""),
         function(648, function::dummy),
         function(649, function::dummy),
         function(650, "IsLinkedTo", "", arg_types::ObjectReference, arg_types::Keyword),
         function(651, "GetKeywordDataForCurrentLocation", "", arg_types::Keyword),
         function(652, "GetInSharedCrimeFaction", "", arg_types::ObjectReference),
         function(653, function::dummy),
         function(654, "GetBribeSuccess", ""),
         function(655, "GetIntimidateSuccess", ""),
         function(656, "GetArrestedState", ""),
         function(657, "GetArrestingActor", ""),
         function(658, function::dummy),
         function(659, "EPTemperingItemIsEnchanted", ""),
         function(660, "EPTemperingItemHasKeyword", "", arg_types::Keyword),
         function(661, function::dummy),
         function(662, function::dummy),
         function(663, function::dummy),
         function(664, "GetReplacedItemType", "", arg_types::CastingSource),
         function(665, function::dummy),
         function(666, function::dummy),
         function(667, function::dummy),
         function(668, function::dummy),
         function(669, function::dummy),
         function(670, function::dummy),
         function(671, function::dummy),
         function(672, "IsAttacking", ""),
         function(673, "IsPowerAttacking", ""),
         function(674, "IsLastHostileActor", ""),
         function(675, "GetGraphVariableInt", "", arg_types::String),
         function(676, "GetCurrentShoutVariation", ""),
         function(677, function::dummy),
         function(678, "ShouldAttackKill", "", arg_types::Actor),
         function(679, function::dummy),
         function(680, function::dummy),
         function(681, "EPMagic_IsAdvanceSkill", "", arg_types::ActorValue),
         function(682, "WornHasKeyword", "", arg_types::Keyword),
         function(683, "GetPathingCurrentSpeed", ""),
         function(684, "GetPathingCurrentSpeedAngle", "", arg_types::Axis),
         function(685, function::dummy),
         function(686, function::dummy),
         function(687, function::dummy),
         function(688, function::dummy),
         function(689, function::dummy),
         function(690, function::dummy),
         function(691, "EPModSkillUsage_AdvanceObjectHasKeyword", "", arg_types::Keyword),
         function(692, "EPModSkillUsage_IsAdvanceAction", "", arg_types::AdvanceAction),
         function(693, "EPMagic_SpellHasKeyword", "", arg_types::Keyword),
         function(694, "GetNoBleedoutRecovery", ""),
         function(695, function::dummy),
         function(696, "EPMagic_SpellHasSkill", "", arg_types::ActorValue),
         function(697, "IsAttackType", "", arg_types::Keyword),
         function(698, "IsAllowedToFly", ""),
         function(699, "HasMagicEffectKeyword", "", arg_types::Keyword),
         function(700, "IsCommandedActor", ""),
         function(701, "IsStaggered", ""),
         function(702, "IsRecoiling", ""),
         function(703, "IsExitingInteractionQuick", ""),
         function(704, "IsPathing", ""),
         function(705, "GetShouldHelp", "", arg_types::Actor),
         function(706, "HasBoundWeaponEquipped", "", arg_types::CastingSource),
         function(707, "GetCombatTargetHasKeyword", "", arg_types::Keyword),
         function(708, function::dummy),
         function(709, "GetCombatGroupMemberCount", ""),
         function(710, "IsIgnoringCombat", ""),
         function(711, "GetLightLevel", ""),
         function(712, function::dummy),
         function(713, "SpellHasCastingPerk", "", arg_types::Perk),
         function(714, "IsBeingRidden", ""),
         function(715, "IsUndead", ""),
         function(716, "GetRealHoursPassed", ""),
         function(717, function::dummy),
         function(718, "IsUnlockedDoor", ""),
         function(719, "IsHostileToActor", "", arg_types::Actor),
         function(720, "GetTargetHeight", "", arg_types::ObjectReference),
         function(721, "IsPoison", ""),
         function(722, "WornApparelHasKeywordCount", "", arg_types::Keyword),
         function(723, "GetItemHealthPercent", ""),
         function(724, "EffectWasDualCast", ""),
         function(725, "GetKnockedStateEnum", ""),
         function(726, "DoesNotExist", ""),
         function(727, function::dummy),
         function(728, function::dummy),
         function(729, function::dummy),
         function(730, "IsOnFlyingMount", ""),
         function(731, "CanFlyHere", ""),
         function(732, "IsFlyingMountPatrolQueued", ""),
         function(733, "IsFlyingMountFastTravelling", ""),
         function(734, "IsOverencumbered", ""), // TODO: SSE only
         function(735, "GetActorWarmth", ""), // TODO: SSE only
      }};
      std::array<function, 5>   extended_function_list = {{
         function(1024, "GetSKSEVersion", ""),
         function(1025, "GetSKSEVersionMinor", ""),
         function(1026, "GetSKSEVersionBeta", ""),
         function(1027, "GetSKSERelease", ""),
         function(1028, "ClearInvalidRegistrations", ""),
      }};
      #pragma endregion
   }
   
   condition_context::condition_context(Form& owner) : owner(&owner) {
      if (auto* casted = dynamic_cast<Quest*>(&owner)) {
         this->quest = casted;
         return;
      }
      //
      // TODO: cast to Package; set (quest) and (package) if so
      //
      // TODO: cast to Scene; set (quest) if so
      //
      // TODO: cast to Topic; set (quest) if so
      //
      // TODO: cast to TopicInfo; get parent Topic and set (quest) if so
      //
   }

   #pragma region condition
   condition_info::arg_type* condition::get_argument_type(uint8_t index) const noexcept {
      if (index >= 2)
         return nullptr;
      auto func = condition_info::function::lookup_by_id(this->function);
      if (!func)
         return nullptr;
      auto a = func->argument_types[index];
      if (a->isUnion) {
         assert(index != 0 && "No behavior defined for a condition function whose first argument type is a union!");
         return a->resolve_union(func->argument_types[index - 1], &this->parameters[index - 1]);
      }
      return a;
   }
   condition_info::arg_underlying_type condition::get_argument_underlying_type(uint8_t index) const noexcept {
      auto a = this->get_argument_type(index);
      if (a) {
         if (a->can_be_alias) {
            if (this->flags & flag::use_aliases)
               return condition_info::arg_underlying_type::aliasID;
            if (this->flags & flag::use_packdata)
               return condition_info::arg_underlying_type::package_data;
         }
         return a->underlying;
      }
      return condition_info::arg_underlying_type::none;
   }

   void condition::set_function(loaded_forms::Form& my_owner, uint16_t id) {
      using _aut = condition_info::arg_underlying_type;
      //
      if (id == this->function)
         return;
      //
      auto prior = condition_info::function::lookup_by_id(this->function);
      auto after = condition_info::function::lookup_by_id(id);
      assert(prior && "Cannot properly audit or clear this condition's parameters: the existing function value is unrecognized.");
      assert(after && "Invalid function ID.");
      for (int i = 0; i < this->parameters.size(); ++i) {
         auto* to_type  = after->argument_types[i];
         auto  to_under = to_type ? to_type->underlying : _aut::none;
         //
         auto& param = this->parameters[i];
         auto  under = this->get_argument_underlying_type(i);
         if (under == _aut::formID) {
            if (to_under != _aut::formID)
               param.form.set(my_owner, nullptr);
         } else {
            param.dword = 0;
         }
         if (!(under == to_under && under == _aut::string))
            param.string.clear();
      }
      //
      this->function = id;
   }
   void condition::set_uses_aliases(loaded_forms::Form& my_owner, bool f) {
      bool prior = this->flags & flag::use_aliases;
      if (prior == f)
         return;
      if (!prior) {
         //
         // The flag wasn't already set, so find any form-type parameters that would have their underlying 
         // types changed by the flag, and clear their values.
         //
         for (int i = 0; i < this->parameters.size(); ++i) {
            auto* type  = this->get_argument_type(i);
            auto  under = this->get_argument_underlying_type(i);
            if (type->can_be_alias && under == condition_info::arg_underlying_type::formID)
               this->parameters[i].form.set(my_owner, nullptr);
         }
      }
      cobb::edit_bit(this->flags, flag::use_aliases, f);
   }
   void condition::set_uses_package_data(loaded_forms::Form& my_owner, bool f) {
      bool prior = this->flags & flag::use_packdata;
      if (prior == f)
         return;
      if (!prior) {
         //
         // The flag wasn't already set, so find any form-type parameters that would have their underlying 
         // types changed by the flag, and clear their values.
         //
         for (int i = 0; i < this->parameters.size(); ++i) {
            auto* type  = this->get_argument_type(i);
            auto  under = this->get_argument_underlying_type(i);
            if (type->can_be_alias && under == condition_info::arg_underlying_type::formID)
               this->parameters[i].form.set(my_owner, nullptr);
         }
      }
      cobb::edit_bit(this->flags, flag::use_packdata, f);
   }

   bool condition::read(tes_record_reader& record, load_order_interfaces::form_load& intfc) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
      if (!subrecord.is_in_bounds(0x14))
         return false;
      {
         uint8_t type; // flags | (operator << 5)
         subrecord.unchecked_read(type);
         this->comparison.op = (operator_t)((type >> 5) & 7);
         this->flags = type & 0x1F;
      }
      subrecord.skip_bytes(3);
      if (this->flags & flag::compare_to_global) {
         subrecord.unchecked_read(this->comparison.operand.global);
         intfc.log_load_warning(
            detailed_notice::warn_if_wrong_type(subrecord.signature(), form_type::global, intfc.target_stub, this->comparison.operand.global)
         );
      } else {
         subrecord.unchecked_read(this->comparison.operand.constant);
      }
      subrecord.unchecked_read(this->function);
      subrecord.skip_bytes(2);
      {
         auto func = condition_info::function::lookup_by_id(this->function);
         if (func) {
            for (int i = 0; i < 2; i++) {
               if (this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID) {
                  auto* arg_type   = func->argument_types[i];
                  auto& allowed    = arg_type->allowedFormTypes;
                  auto& value_form = this->parameters[i].form;
                  subrecord.unchecked_read(value_form);
                  //
                  if (allowed.size() == 1) {
                     intfc.log_load_warning(
                        detailed_notice::warn_if_wrong_type(subrecord.signature(), arg_type->allowedFormTypes[0], intfc.target_stub, value_form)
                     );
                  } else if (auto* stub = value_form.get_form_stub()) {
                     if (!arg_type->allows_form_type(stub->formType)) {
                        intfc.log_load_warning(
                           detailed_notice::warn_if_wrong_type(subrecord.signature(), {}, intfc.target_stub, value_form)
                        );
                     }
                  }
               } else {
                  subrecord.unchecked_read(this->parameters[i].dword);
               }
            }
         }
         if (func && func->uses_event_data) {
            if (!subrecord.is_in_bounds(8))
               return false;
            subrecord.unchecked_read(this->event_parameters.function);
            subrecord.unchecked_read(this->event_parameters.member);
            subrecord.unchecked_read(this->event_parameters.form);
         } else {
            if (!subrecord.is_in_bounds(12))
               return false;
            subrecord.unchecked_read(this->run_on.type);
            subrecord.unchecked_read(this->run_on.reference);
            if (this->run_on.type == run_on_t::event_data) {
               subrecord.read_signature(this->run_on.index);
            } else {
               subrecord.unchecked_read(this->run_on.index);
            }
            intfc.log_load_warning(
               detailed_notice::warn_if_not_object_reference(subrecord.signature(), intfc.target_stub, this->run_on.reference)
            );
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
   /*static*/ void condition::generate_use_info(tes_record_reader& record, form_stub_use_info_builder& uib) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord.signature() == 'CTDA' && "Condition::read should only be called just after the CTDA subrecord is opened.");
      if (!subrecord.is_in_bounds(0x14))
         return;
      uint8_t   type;
      uint16_t  function;
      form_id_t formID;
      subrecord.unchecked_read(type);
      subrecord.skip_bytes(3);
      if (type & flag::compare_to_global) {
         subrecord.unchecked_read(formID);
         uib.add_outbound_reference(formID);
      } else
         subrecord.skip_bytes(4);
      subrecord.unchecked_read(function);
      subrecord.skip_bytes(2);
      {
         bool uses_aliases  = type & flag::use_aliases;
         bool uses_packdata = type & flag::use_packdata;
         uint32_t firstValue = 0; // needed for when the second arg is a union
         //
         auto func = condition_info::function::lookup_by_id(function);
         for (int i = 0; i < 2; ++i) {
            auto* type  = func->argument_types[0];
            if (i == 1 && type->isUnion) { // resolve the union
               condition_arg_value value;
               value.dword = firstValue;
               type = type->resolve_union(type, &value);
            }
            //
            auto under = type->underlying;
            if (type->can_be_alias) {
               if (uses_aliases)
                  under = condition_info::arg_underlying_type::aliasID;
               if (uses_packdata)
                  under = condition_info::arg_underlying_type::package_data;
            }
            //
            if (under == condition_info::arg_underlying_type::formID) {
               subrecord.unchecked_read(formID);
               uib.add_outbound_reference(formID);
            } else {
               subrecord.skip_bytes(4);
            }
         }
         if (func && func->uses_event_data) {
            if (!subrecord.is_in_bounds(8))
               return;
            subrecord.skip_bytes(4);
            subrecord.unchecked_read(formID);
            uib.add_outbound_reference(formID);
         } else {
            if (!subrecord.is_in_bounds(12))
               return;
            subrecord.skip_bytes(4);
            subrecord.unchecked_read(formID);
            uib.add_outbound_reference(formID);
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
   void condition::save(tes_record_writer& record, load_order_interfaces::form_save& intfc) {
      auto& subrecord = record.open_next_subrecord('CTDA');
      {
         uint8_t type = this->flags | (uint8_t(this->comparison.op) << 5); // flags | (operator << 5)
         subrecord.write(type);
      }
      subrecord.skip_bytes(3);
      if (this->flags & flag::compare_to_global)
         subrecord.write(this->comparison.operand.global);
      else
         subrecord.write(this->comparison.operand.constant);
      subrecord.write(this->function);
      subrecord.skip_bytes(2);
      {
         auto func = condition_info::function::lookup_by_id(this->function);
         for (int i = 0; i < 2; i++) {
            if (func && this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID)
               subrecord.write(this->parameters[i].form);
            else
               subrecord.write(this->parameters[i].dword);
         }
         if (func && func->uses_event_data) {
            subrecord.write(this->event_parameters.function);
            subrecord.write(this->event_parameters.member);
            subrecord.write(this->event_parameters.form);
         } else {
            subrecord.write(this->run_on.type);
            subrecord.write(this->run_on.reference);
            if (this->run_on.type == run_on_t::event_data) {
               subrecord.write_signature(this->run_on.index);
            } else {
               subrecord.write(this->run_on.index);
            }
         }
      }
      subrecord.close();
      //
      if (this->get_argument_underlying_type(0) == condition_info::arg_underlying_type::string) {
         auto& CIS1 = record.open_next_subrecord('CIS1');
         CIS1.write(this->parameters[0].string);
         CIS1.close();
      }
      if (this->get_argument_underlying_type(1) == condition_info::arg_underlying_type::string) {
         auto& CIS1 = record.open_next_subrecord('CIS2');
         CIS1.write(this->parameters[1].string);
         CIS1.close();
      }
   }
   void condition::clone_from(const condition& other, loaded_forms::Form& my_owner) noexcept {
      this->clear(my_owner);
      //
      this->flags = other.flags;
      this->comparison.op = other.comparison.op;
      this->comparison.operand.constant = other.comparison.operand.constant;
      this->comparison.operand.global.set(my_owner, other.comparison.operand.global);
      this->function = other.function;
      //
      auto func = condition_info::function::lookup_by_id(this->function);
      for (int i = 0; i < 2; i++) {
         auto& param = this->parameters[i];
         auto& from  = other.parameters[i];
         if (func && this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID)
            param.form.set(my_owner, from.form);
         else
            param.dword = from.dword;
         param.string = from.string;
      }
      //
      this->run_on.type  = other.run_on.type;
      this->run_on.index = other.run_on.index;
      this->run_on.reference.set(my_owner, other.run_on.reference);
      //
      this->event_parameters.function = other.event_parameters.function;
      this->event_parameters.member   = other.event_parameters.member;
      if (func && func->uses_event_data) {
         this->event_parameters.form.set(my_owner, other.event_parameters.form);
      }
   }
   void condition::sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept {
      this->comparison.operand.global.clear_if(my_owner, target);
      //
      auto func = condition_info::function::lookup_by_id(this->function);
      for (int i = 0; i < 2; i++) {
         auto& param = this->parameters[i];
         if (func && this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID) {
            param.form.clear_if(my_owner, target);
         }
      }
      //
      this->run_on.reference.clear_if(my_owner, target);
      this->event_parameters.form.clear_if(my_owner, target);
   }
   void condition::clear(loaded_forms::Form& my_owner) {
      auto func = condition_info::function::lookup_by_id(this->function);
      for (int i = 0; i < 2; i++) {
         auto& param = this->parameters[i];
         if (func && this->get_argument_underlying_type(i) == condition_info::arg_underlying_type::formID) {
            //
            // NOTE: form parameters MUST be cleared BEFORE setting flags, because the underlying 
            // type of a condition's parameters can vary depending on its flags.
            //
            param.form.set(my_owner, nullptr);
         }
      }
      //
      this->run_on.reference.set(my_owner, nullptr);
      this->event_parameters.form.set(my_owner, nullptr);
      //
      this->function = 0;
      this->flags = 0;
      this->comparison.op = operator_t::equal;
      this->comparison.operand.constant = 0.0F;
      this->comparison.operand.global.set(my_owner, nullptr);
      this->run_on.index = -1;
      this->event_parameters.function = 0;
      this->event_parameters.member   = 0;
   }
   #pragma endregion
}