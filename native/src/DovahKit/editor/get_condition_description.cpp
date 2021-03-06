#include "get_condition_description.h"
#include <array>
#include <QObject>

namespace {
   struct _condition_desc {
      uint16_t    id;
      const char* name;
      QString     desc;
      //
      _condition_desc(uint16_t i, const char* n, const char* d) : id(i), name(n), desc(QObject::tr(d, "condition function description")) {}
   };

   constexpr char* text_returns_immediately = "This condition is a no-op, does nothing, and cannot be expected to return a meaningful or consistent result.";
   constexpr char* text_returns_zero        = "This function is a no-op and always returns 0.";

   std::array _descriptions = {
      _condition_desc(0, "GetWantBlocking",   ""),
      _condition_desc(1, "GetDistance",       "Returns the distance between this reference and another reference."),
      //
      _condition_desc(5, "GetLocked",         "Returns 1 if this reference is a locked door or container, or 0 otherwise."),
      _condition_desc(6, "GetPos",            "Returns this reference's position along the given axis."),
      //
      _condition_desc(8, "GetAngle",          "Returns this reference's rotation along the given axis."),
      //
      _condition_desc(10, "GetStartingPos",    "Returns this reference's starting position along the given axis."),
      _condition_desc(11, "GetStartingAngle",  "Returns this reference's starting rotation along the given axis."),
      _condition_desc(12, "GetSecondsPassed",  ""),
      //
      _condition_desc(14, "GetActorValue",     "Returns the current value of the specified ActorValue."),
      //
      _condition_desc(18, "GetCurrentTime",    "Returns the current in-game time of day as the number of hours since midnight, such that 4:30 PM, for example, would be 16.5."),
      //
      _condition_desc(24, "GetScale",          "Returns this reference's scale."),
      _condition_desc(25, "IsMoving",          "Returns a non-zero value if this actor is attempting to move, or zero otherwise. The values indicate the desired direction of movement; 1, 2, 3, and 4 indicate forward, backward, left, and right, respectively."),
      _condition_desc(26, "IsTurning",         "Returns 1 if this actor is turning to the left, 2 if they are turning to the right, or 0 otherwise."),
      _condition_desc(27, "GetLineOfSight",    "When run on the player, returns 1 if the specified reference is in the camera's line of sight, or 0 otherwise. When run on an NPC, returns 1 if the specified reference is an actor and is in the NPC's line of sight, or 0 otherwise. When run on a disabled actor, crashes the game."),
      //
      _condition_desc(32, "GetInSameCell",     "Returns 1 if this reference is in the same cell as the specified reference, or 0 otherwise."),
      //
      _condition_desc(35, "GetDisabled",       "Returns 1 if this reference is disabled, or 0 otherwise."),
      _condition_desc(36, "MenuMode",          text_returns_immediately),
      //
      _condition_desc(39, "GetDisease",        "Returns 1 if any of this actor's active magic effects came from a spell whose type was set to \"Disease,\" or 0 otherwise."),
      //
      _condition_desc(41, "GetClothingValue",  "Returns a value between 0 and 100 representing the estimated worth of this actor's worn armor and clothing. The value of each worn item is scaled according to how much of the body it covers, and rounded to a multiple of 0.1."), // TODO: can we RE more detail on this?
      _condition_desc(42, "SameFaction",       "Returns 1 if this reference is in the same faction as the specified actor, or 0 otherwise."),
      _condition_desc(43, "SameRace",          "Returns 1 if this reference is of the same race as the specified actor, or 0 otherwise."),
      _condition_desc(44, "SameSex",           "Returns 1 if this reference is of the same sex as the specified actor, or 0 otherwise."),
      _condition_desc(45, "GetDetected",       "Returns 1 if this actor is aware of the specified actor's presence."),
      _condition_desc(46, "GetDead",           "Returns 1 if this actor is dead, or 0 otherwise."),
      _condition_desc(47, "GetItemCount",      "Returns how many of the specified form this reference has in its inventory."),
      _condition_desc(48, "GetGold",           "Returns how much gold this reference is carrying."),
      _condition_desc(49, "GetSleeping",       ""),
      _condition_desc(50, "GetTalkedToPC",     ""),
      //
      _condition_desc(53, "GetScriptVariable",  ""),
      //
      _condition_desc(56, "GetQuestRunning",    "Returns 1 if the specified quest is running, or 0 otherwise."),
      //
      _condition_desc(58, "GetStage",           "Returns the specified quest's current stage number."),
      _condition_desc(59, "GetStageDone",       "Returns 1 if the specified quest stage is complete, or 0 otherwise."),
      _condition_desc(60, "GetFactionRankDifference", ""),
      _condition_desc(61, "GetAlarmed",         ""),
      _condition_desc(62, "IsRaining",          "Returns 1 if the current weather is rainy, or 0 otherwise."),
      _condition_desc(63, "GetAttacked",        ""),
      _condition_desc(64, "GetIsCreature",      text_returns_zero),
      _condition_desc(65, "GetLockLevel",       ""),
      _condition_desc(66, "GetShouldAttack",    ""),
      _condition_desc(67, "GetInCell",          "Returns 1 if this reference is in ttext_returns_zerohe specified cell, or 0 otherwise."),
      _condition_desc(68, "GetIsClass",         "Returns 1 if this actor is of the specified class, or 0 otherwise."),
      _condition_desc(69, "GetIsRace",          "Returns 1 if this actor is of the specified race, or 0 otherwise."),
      _condition_desc(70, "GetIsSex",           "Returns 1 if this actor is of the specified sex, or 0 otherwise."),
      _condition_desc(71, "GetInFaction",       ""),
      _condition_desc(72, "GetIsID",            "Returns 1 if the specified form is this reference's base form; returns 1 if this reference is a leveled creature and the specified form is the leveled actor base; returns 0 otherwise."), // function early-outs with 0 if the form argument isn't a valid base form (TESForm virtual member function 0x27)
      _condition_desc(73, "GetFactionRank",     "Returns this actor's rank in the specified faction, or -1 if this reference isn't in that faction or isn't an actor."),
      _condition_desc(74, "GetGlobalValue",     "Returns the value of the specified Global."),
      _condition_desc(75, "IsSnowing",          "Returns 1 if the current weather is snowy, or 0 otherwise."),
      //
      _condition_desc(77, "GetRandomPercent",   "Returns a random number between 0 and 100, inclusive."),
      //
      _condition_desc(79, "GetQuestVariable",   ""),
      _condition_desc(80, "GetLevel",           "Returns this actor's level."),
      _condition_desc(81, "IsRotating",         ""),
      //
      _condition_desc(84, "GetDeadCount",       "Returns the number of times this ActorBase has died. This value is tracked for all ActorBases, and would be meaningful for non-unique actors and for actors who can be resurrected."),
      //
      _condition_desc(91, "GetIsAlerted",       "Returns 1 if this actor is alerted, or 0 otherwise."),
      //
      _condition_desc(98, "GetPlayerControlsDisabled", ""),
      _condition_desc(99, "GetHeadingAngle", ""),
      //
      _condition_desc(101, "IsWeaponMagicOut", ""),
      _condition_desc(102, "IsTorchOut", ""),
      _condition_desc(103, "IsShieldOut", "Returns 1 if this actor has a shield equipped, or 0 otherwise."), // checks the actor biped i.e. body slots
      //
      _condition_desc(106, "IsFacingUp", ""),
      _condition_desc(107, "GetKnockedState", "Returns 1 if this actor is knocked down (i.e. ragdolling or paralyzed) for any reason, or 0 otherwise."), // The Creation Kit wiki lists a 2 value, but the game never returns that.
      _condition_desc(108, "GetWeaponAnimType", ""),
      _condition_desc(109, "IsWeaponSkillType", ""),
      _condition_desc(110, "GetCurrentAIPackage", ""),
      _condition_desc(111, "IsWaiting", "Skyrim's developers have marked this function as obsolete. It checks this actor's current AI package and procedure and returns 1 if the actor is waiting, or 0 otherwise; however, the precise operational definition of \"waiting\" is not known."),
      _condition_desc(112, "IsIdlePlaying", text_returns_zero),
      //
      _condition_desc(116, "IsIntimidatedByPlayer", ""),
      _condition_desc(117, "IsPlayerInRegion", "Returns 1 if this reference's parent cell belongs to or overlaps the specified region, or 0 otherwise."),
      _condition_desc(118, "GetActorAggroRadiusViolated", ""),
      //
      _condition_desc(122, "GetCrime", "Returns 1 if this actor is aware that they are the victim of a crime of the specified type, committed by the specified actor."),
      _condition_desc(123, "IsGreetingPlayer", ""),
      //
      _condition_desc(125, "IsGuard", "Returns 1 if this actor is a guard, or 0 otherwise."), // unconfirmed, but "being a guard" appears to be influenced by membership in the IsGuardFaction, provided to the game via a DOBJ
      //
      _condition_desc(127, "HasBeenEaten",               "Returns 1 if this actor has been fed on by a cannibal or werewolf, or 0 otherwise."),
      _condition_desc(128, "GetStaminaPercentage",       ""),
      _condition_desc(129, "GetPCIsClass",               "Returns 1 if the player-character is of the specified class, or 0 otherwise."),
      _condition_desc(130, "GetPCIsRace",                "Returns 1 if the player-character is of the specified race, or 0 otherwise."),
      _condition_desc(131, "GetPCIsSex",                 "Returns 1 if the player-character is of the specified sex, or 0 otherwise."),
      _condition_desc(132, "GetPCInFaction",             ""),
      _condition_desc(133, "SameFactionAsPC",            ""),
      _condition_desc(134, "SameRaceAsPC",               "Returns 1 if this actor is of the same race as the player-character, or 0 otherwise."),
      _condition_desc(135, "SameSexAsPC",                "Returns 1 if this actor is of the same sex as the player-character, or 0 otherwise."),
      _condition_desc(136, "GetIsReference",             "Returns 1 if this reference is the specified reference, or 0 otherwise."),
      //
      _condition_desc(141, "IsTalking", ""),
      _condition_desc(142, "GetWalkSpeed", ""),
      _condition_desc(143, "GetCurrentAIProcedure",      ""),
      _condition_desc(144, "GetTrespassWarningLevel",    ""),
      _condition_desc(145, "IsTrespassing", ""),
      _condition_desc(146, "IsInMyOwnedCell", ""),
      _condition_desc(147, "GetWindSpeed", ""),
      _condition_desc(148, "GetCurrentWeatherPercent",   ""),
      _condition_desc(149, "GetIsCurrentWeather",        "Returns 1 if the specified weather is the current weather, or 0 otherwise."),
      _condition_desc(150, "IsContinuingPackagePCNear",  "Returns 1 if this actor wants to switch to another AI package but can't, because its current package has the \"Continue If PC Near\" flag set and the player is in the same cell. Returns 0 otherwise."),
      //
      _condition_desc(152, "GetIsCrimeFaction",          "Returns 1 if this actor's crime faction is the specified faction, or 0 otherwise."),
      _condition_desc(153, "CanHaveFlames",              "Returns 1 if this reference's 3D model has BSXFlag 0x10 set, or 0 otherwise."),
      _condition_desc(154, "HasFlames",                  ""),
      //
      _condition_desc(157, "GetOpenState",               ""),
      //
      _condition_desc(159, "GetSitting",                 ""),
      //
      _condition_desc(161, "GetIsCurrentPackage",        ""),
      _condition_desc(162, "IsCurrentFurnitureRef",      ""),
      _condition_desc(163, "IsCurrentFurnitureObj",      ""),
      //
      _condition_desc(170, "GetDayOfWeek",               "Returns the current day of the week. Days are numbered starting from Sunday/Sundas (0) and Monday/Morndas (1) through to Friday/Fredas (5) and Saturday/Loredas (6)."),
      //
      _condition_desc(172, "GetTalkedToPCParam",         ""),
      //
      _condition_desc(175, "IsPCSleeping",               ""),
      _condition_desc(176, "IsPCAMurderer",              "Returns 1 if the player has ever murdered an NPC, or 0 otherwise."),
      //
      _condition_desc(180, "HasSameEditorLocAsRef",      ""),
      _condition_desc(181, "HasSameEditorLocAsRefAlias", ""),
      _condition_desc(182, "GetEquipped",                ""),
      //
      _condition_desc(185, "IsSwimming",                 "Returns 1 if this actor is swimming, or 0 otherwise."),
      //
      _condition_desc(190, "GetAmountSoldStolen",        "Returns the gold value of all stolen goods that the player has fenced?"), // returns value tracked on the PlayerCharacter class; if it's not gold, then maybe it's an item quantity?
      //
      _condition_desc(192, "GetIgnoreCrime",             ""),
      _condition_desc(193, "GetPCExpelled",              "Returns 1 if the player is currently expelled from the specified faction, or 0 otherwise."),
      //
      _condition_desc(195, "GetPCFactionMurder",         ""),
      //
      _condition_desc(197, "GetPCEnemyofFaction",        "Returns 1 if the player is an enemy of the specified faction, or 0 otherwise."),
      //
      _condition_desc(199, "GetPCFactionAttack",         ""),
      //
      _condition_desc(203, "GetDestroyed",               "Returns 1 if this reference is destroyed, or 0 otherwise."),
      //
      _condition_desc(214, "HasMagicEffect",             ""),
      _condition_desc(215, "GetDefaultOpen",             "Returns 1 if this reference's base form is set to be open by default, or 0 otherwise."),
      //
      _condition_desc(219, "GetAnimAction",              ""),
      //
      _condition_desc(223, "IsSpellTarget",              ""),
      _condition_desc(224, "GetVATSMode",                ""),
      _condition_desc(225, "GetPersuasionNumber",        text_returns_immediately),
      _condition_desc(226, "GetVampireFeed",             "Returns 1 if this actor is a vampire currently feeding on another actor, or 0 otherwise."),
      _condition_desc(227, "GetCannibal",                "Returns 1 if this actor is currently a cannibal feeding on another actor, or 0 otherwise."),
      _condition_desc(228, "GetIsClassDefault",          ""), // same eval-function as GetIsClass? GetInCellParam seems to use the same eval-function as GetInCell, so that may not mean so much...
      _condition_desc(229, "GetClassDefaultMatch",       text_returns_immediately),
      _condition_desc(230, "GetInCellParam",             "Returns 1 if the specified reference is inside of the specified cell, or 0 otherwise."),
      //
      _condition_desc(235, "GetVatsTargetHeight",        ""),
      //
      _condition_desc(237, "GetIsGhost",                 "Returns 1 if this actor's base form is flagged as a ghost, or 0 otherwise."),
      //
      _condition_desc(242, "GetUnconscious",             "Returns 1 if this actor is unconscious, or 0 otherwise."),
      //
      _condition_desc(244, "GetRestrained",              "Returns 1 if this actor is restrained, or 0 otherwise."),
      //
      _condition_desc(246, "GetIsUsedItem",              ""),
      _condition_desc(247, "GetIsUsedItemType",          ""),
      _condition_desc(248, "IsScenePlaying",             "Returns 1 if the specified scene is playing, or 0 otherwise."),
      _condition_desc(249, "IsInDialogueWithPlayer",     ""),
      _condition_desc(250, "GetLocationCleared",         "Returns 1 if the specified location is cleared, or 0 otherwise."),
      //
      _condition_desc(254, "GetIsPlayableRace",          "Returns 1 if this actor's race is flagged as playable, or 0 otherwise."),
      _condition_desc(255, "GetOffersServicesNow",       "Returns 1 if this actor is currently available to barter with, or 0 otherwise."),
      //
      _condition_desc(258, "HasAssociationType",         ""),
      _condition_desc(259, "HasFamilyRelationship",      ""),
      //
      _condition_desc(261, "HasParentRelationship",      ""),
      _condition_desc(262, "IsWarningAbout",             ""),
      _condition_desc(263, "IsWeaponOut",                ""),
      _condition_desc(264, "HasSpell",                   "Returns 1 if the actor knows the specified spell (or has the specified ability, etc.), or 0 otherwise."),
      _condition_desc(265, "IsTimePassing",              ""),
      _condition_desc(266, "IsPleasant",                 "Returns 1 if the current weather is pleasant, or 0 otherwise."),
      _condition_desc(267, "IsCloudy",                   "Returns 1 if the current weather is cloudy, or 0 otherwise."),
      //
      _condition_desc(274, "IsSmallBump",                ""),
      //
      _condition_desc(277, "GetBaseActorValue",          "Returns the base value of the specified ActorValue."),
      _condition_desc(278, "IsOwner",                    "Returns 1 if this reference is owned by the specified actor or faction, or 0 otherwise."),
      //
      _condition_desc(280, "IsCellOwner",                "Returns 1 if the specified actor or faction owns the specified cell, or 0 otherwise."),
      //
      _condition_desc(282, "IsHorseStolen",              text_returns_zero),
      //
      _condition_desc(285, "IsLeftUp",                   ""),
      _condition_desc(286, "IsSneaking",                 "Returns 1 if this actor is in sneak mode, or 0 otherwise."),
      _condition_desc(287, "IsRunning",                  ""),
      _condition_desc(288, "GetFriendHit",               "Returns the number of times that this actor has been hit with friendly fire from the specified actor."),
      _condition_desc(289, "IsInCombat",                 ""),
      //
      _condition_desc(300, "IsInInterior",               "Returns 1 if this reference is in an interior cell, or 0 otherwise."),
      //
      _condition_desc(304, "IsWaterObject",              "Returns 1 if calling TESObject::GetWaterType on this reference returns a water type pointer, or 0 otherwise."), // calls TESObject::GetWaterType
      _condition_desc(305, "GetPlayerAction",            ""),
      _condition_desc(306, "IsActorUsingATorch",         "This function is a no-op and always returns 0. Use IsTorchOut instead."),
      //
      _condition_desc(309, "IsXBox",                     "Returns 1 if this is an Xbox build of the game, or 0 otherwise."),
      _condition_desc(310, "GetInWorldspace",            "Returns 1 if this reference is inside of the specified worldspace, or 0 otherwise."),
      //
      _condition_desc(312, "GetPCMiscStat",              "Returns the value of the specified misc stat."),
      _condition_desc(313, "GetPairedAnimation",         ""),
      _condition_desc(314, "IsActorAVictim",             ""), // checks whether this actor is the one stored in a thread-local-storage variable
      _condition_desc(315, "GetTotalPersuasionNumber",   text_returns_immediately),
      //
      _condition_desc(318, "GetIdleDoneOnce",            ""),
      //
      _condition_desc(320, "GetNoRumors",                ""),
      //
      _condition_desc(323, "GetCombatState",             "Returns 2 if this actor is searching for an enemy, 1 if this actor is in open combat with an enemy, or 0 otherwise. Relies on internally cached data, and reportedly, using the condition just once may produce an out-of-date result."),
      //
      _condition_desc(325, "GetWithinPackageLocation",   ""),
      //
      _condition_desc(327, "IsRidingMount",              ""),
      //
      _condition_desc(329, "IsFleeing",                  ""),
      //
      _condition_desc(332, "IsInDangerousWater",         "Returns 1 if this actor is standing or swimming in a body of water that has been flagged as dangerous, or 0 otherwise."),
      //
      _condition_desc(338, "GetIgnoreFriendlyHits",      "Returns 1 if this actor is ignoring friendly fire, or 0 otherwise."),
      _condition_desc(339, "IsPlayersLastRiddenMount",   "Returns 1 if this actor is the player's last ridden mount, or 0 otherwise."),
      //
      _condition_desc(353, "IsActor",                    "Returns 1 if this reference is an actor, or 0 otherwise."),
      _condition_desc(354, "IsEssential",                "Returns 1 if this actor is flagged as essential, or 0 otherwise."),
      //
      _condition_desc(358, "IsPlayerMovingIntoNewSpace", ""),
      _condition_desc(359, "GetInCurrentLoc",            ""),
      _condition_desc(360, "GetInCurrentLocAlias",       ""),
      _condition_desc(361, "GetTimeDead",                ""),
      _condition_desc(362, "HasLinkedRef",               ""),
      //
      _condition_desc(365, "IsChild",                    ""),
      _condition_desc(366, "GetStolenItemValueNoCrime",  ""),
      _condition_desc(367, "GetLastPlayerAction",        ""),
      _condition_desc(368, "IsPlayerActionActive",       ""), // arg type needs verification
      //
      _condition_desc(370, "IsTalkingActivatorActor",    ""),
      //
      _condition_desc(372, "IsInList",                   "Returns 1 if this reference's base form is in the specified FormList, or 0 otherwise. If this reference is a leveled actor, then the ActorBase selected for spawn will be checked, rather than the LeveledActor form itself."),
      _condition_desc(373, "GetStolenItemValue",         ""),
      //
      _condition_desc(375, "GetCrimeGoldViolent",        "Returns how much of the player's bounty with the specified faction comes from murder and assault crimes. If no faction is specified and this reference is an actor, then their crime faction will be used."),
      _condition_desc(376, "GetCrimeGoldNonViolent",     "Returns how much of the player's bounty with the specified faction comes from crimes other than murder and assault. If no faction is specified and this reference is an actor, then their crime faction will be used."),
      //
      _condition_desc(378, "HasShout",                   ""),
      //
      _condition_desc(381, "GetHasNote",                 "If run on the player, returns 1 if they have ever collected a note of the specified type (unless they \"collected\" it from an EquipItem call, which doesn't count due to a bug). Returns 0 otherwise."),
      //
      _condition_desc(390, "GetHitLocation",             "Returns the ID of the last body part on this actor that was hit by an attack. Because Skyrim doesn't use the same limb IDs as Fallout, this function isn't terribly useful."),
      _condition_desc(391, "IsPC1stPerson",              ""),
      //
      _condition_desc(396, "GetCauseofDeath",            ""),
      _condition_desc(397, "IsLimbGone",                 "Returns 1 if this actor has lost the specified limb, or 0 otherwise. Limb ID 1 is the head, so this should be usable for testing decapitation."),
      _condition_desc(398, "IsWeaponInList",             ""),
      //
      _condition_desc(402, "IsBribedByPlayer", ""),
      _condition_desc(403, "GetRelationshipRank", ""),
      //
      _condition_desc(407, "GetVATSValue",               ""),
      _condition_desc(408, "IsKiller",                   ""),
      _condition_desc(409, "IsKillerObject",             ""),
      _condition_desc(410, "GetFactionCombatReaction",   ""),
      //
      _condition_desc(414, "Exists",                     "Returns 1 if this reference is the specified reference and if the specified reference exists, or 0 otherwise."),
      _condition_desc(415, "GetGroupMemberCount",        "Functionally identical to GetCombatGroupMemberCount but apparently deprecated. Use GetCombatGroupMemberCount instead."),
      _condition_desc(416, "GetGroupTargetCount",        "Returns the number of targets that this actor's combat group is engaging in combat. Possibly deprecated."),
      //
      _condition_desc(426, "GetIsVoiceType",             "Returns 1 if this actor uses the specified voicetype, or 0 otherwise."),
      _condition_desc(427, "GetPlantedExplosive",        ""),
      //
      _condition_desc(429, "IsScenePackageRunning",      "Returns 1 if this actor is currently running a scene AI package, or 0 otherwise."),
      _condition_desc(430, "GetHealthPercentage",        ""),
      //
      _condition_desc(432, "GetIsObjectType",            ""),
      //
      _condition_desc(434, "GetDialogueEmotion",         ""),
      _condition_desc(435, "GetDialogueEmotionValue",    ""),
      //
      _condition_desc(437, "GetIsCreatureType",          ""), // double-check this
      //
      _condition_desc(444, "GetInCurrentLocFormList",    ""),
      _condition_desc(445, "GetInZone",                  ""),
      _condition_desc(446, "GetVelocity",                ""),
      _condition_desc(447, "GetGraphVariableFloat",      ""),
      _condition_desc(448, "HasPerk",                    ""), // second arg type is not known
      _condition_desc(449, "GetFactionRelation",         "Returns one of the following values representing the faction relationship between this actor and the specified actor: neutral (0), enemy (1), ally (2), or friend (3)."),
      _condition_desc(450, "IsLastIdlePlayed",           ""),
      //
      _condition_desc(453, "GetPlayerTeammate",          ""),
      _condition_desc(454, "GetPlayerTeammateCount",     ""),
      //
      _condition_desc(458, "GetActorCrimePlayerEnemy",   ""),
      _condition_desc(459, "GetCrimeGold",               "Returns the player's bounty with the specified faction. If no faction is specified, uses this actor's crime faction."),
      //
      _condition_desc(463, "IsPlayerGrabbedRef",         "Returns 1 if the player is Z-keying or using telekinesis on the specified object, or 0 otherwise."),
      //
      _condition_desc(465, "GetKeywordItemCount",        "Returns the number of items in this reference's inventory that have the specified keyword."),
      //
      _condition_desc(470, "GetDestructionStage",        ""),
      //
      _condition_desc(473, "GetIsAlignment",             text_returns_zero),
      //
      _condition_desc(476, "IsProtected",                "Returns 1 if this actor is flagged as protected, or 0 otherwise."),
      _condition_desc(477, "GetThreatRatio",             ""),
      //
      _condition_desc(479, "GetIsUsedItemEquipType",     ""),
      //
      _condition_desc(487, "IsCarryable",                ""),
      _condition_desc(488, "GetConcussed",               text_returns_zero),
      //
      _condition_desc(491, "GetMapMarkerVisible",        "Returns 2 if the player can fast-travel to this map marker; 1 if they cannot, but this map marker is visible on their map; or 0 otherwise."),
      //
      _condition_desc(493, "PlayerKnows",                ""),
      _condition_desc(494, "GetPermanentActorValue",     "Returns the \"permanent modifier\" value of the specified ActorValue."),
      _condition_desc(495, "GetKillingBlowLimb",         ""),
      //
      _condition_desc(497, "CanPayCrimeGold",            "Returns 1 if the player has enough gold on hand to pay their bounty to this actor's crime faction, or 0 otherwise."),
      //
      _condition_desc(499, "GetDaysInJail",              ""),
      _condition_desc(500, "EPAlchemyGetMakingPoison",   "A function intended for use in a perk entry point."),
      _condition_desc(501, "EPAlchemyEffectHasKeyword",  "A function intended for use in a perk entry point."),
      //
      _condition_desc(503, "GetAllowWorldInteractions",  ""),
      //
      _condition_desc(508, "GetLastHitCritical",         "Returns 1 if the last attack that hit this actor was a critical hit, or 0 otherwise."),
      //
      _condition_desc(513, "IsCombatTarget",             ""),
      //
      _condition_desc(515, "GetVATSRightAreaFree",       ""),
      _condition_desc(516, "GetVATSLeftAreaFree",        ""),
      _condition_desc(517, "GetVATSBackAreaFree",        ""),
      _condition_desc(518, "GetVATSFrontAreaFree",       ""),
      _condition_desc(519, "GetLockIsBroken",            ""),
      _condition_desc(520, "IsPS3",                      "Returns 1 if this is a PlayStation build of the game, or 0 otherwise."),
      _condition_desc(521, "IsWin32",                    "Returns 1 if this is a PC build of the game, or 0 otherwise."),
      _condition_desc(522, "GetVATSRightTargetVisible",  ""),
      _condition_desc(523, "GetVATSLeftTargetVisible",   ""),
      _condition_desc(524, "GetVATSBackTargetVisible",   ""),
      _condition_desc(525, "GetVATSFrontTargetVisible",  ""),
      //
      _condition_desc(528, "IsInCriticalStage",          "Returns 1 if this actor is in the specified critical stage, or 0 otherwise."),
      //
      _condition_desc(530, "GetXPForNextLevel",          text_returns_zero),
      //
      _condition_desc(533, "GetInfamy",                  "Returns the player's infamy with the specified faction. If no faction is specified, the function uses this actor's crime faction."),
      _condition_desc(534, "GetInfamyViolent",           "Returns how much of the player's infamy with the specified faction comes from violent actions. If no faction is specified, the function uses this actor's crime faction."),
      _condition_desc(535, "GetInfamyNonViolent",        "Returns how much of the player's infamy with the specified faction comes from non-violent actions. If no faction is specified, the function uses this actor's crime faction."),
      //
      _condition_desc(543, "GetQuestCompleted",          "Returns 1 if the specified quest is completed, or 0 otherwise."),
      //
      _condition_desc(547, "IsGoreDisabled",             "Returns 1 if the bDisableAllGore INI setting is enabled, or 0 otherwise."),
      //
      _condition_desc(550, "IsSceneActionComplete",      "Returns 1 if the specified action in the specified scene is complete, or 0 otherwise."),
      //
      _condition_desc(552, "GetSpellUsageNum",           ""),
      //
      _condition_desc(554, "GetActorsInHigh",            "Returns the number of actors currently in \"high\" AI processing. This includes dead bodies and does not include the player."),
      _condition_desc(555, "HasLoaded3D",                "Returns 1 if this reference has any 3D loaded, or 0 otherwise."),
      //
      _condition_desc(560, "HasKeyword",                 "Returns 1 if this reference's base form has the specified keyword, or 0 otherwise."),
      _condition_desc(561, "HasRefType",                 "Returns 1 if this reference has the specified Location Ref Type, or 0 otherwise."),
      _condition_desc(562, "LocationHasKeyword",         "Returns 1 if this location has the specified keyword, or 0 otherwise."),
      _condition_desc(563, "LocationHasRefType",         ""),
      //
      _condition_desc(565, "GetIsEditorLocation",        "Returns 1 if the specified location is this reference's editor location, or 0 otherwise."),
      _condition_desc(566, "GetIsAliasRef",              "Returns 1 if this reference is the one referred to by the specified alias, or 0 otherwise."),
      _condition_desc(567, "GetIsEditorLocAlias",        ""),
      _condition_desc(568, "IsSprinting",                "Returns 1 if this actor is sprinting, or 0 otherwise."),
      _condition_desc(569, "IsBlocking",                 "Returns 1 if this actor is blocking, or 0 otherwise."),
      _condition_desc(570, "HasEquippedSpell",           ""),
      _condition_desc(571, "GetCurrentCastingType",      ""),
      _condition_desc(572, "GetCurrentDeliveryType",     ""),
      //
      _condition_desc(574, "GetAttackState",             ""),
      //
      _condition_desc(576, "GetEventData",               ""),
      _condition_desc(577, "IsCloserToAThanB",           "Returns 1 if this reference is closer to the first argument than it is to the second argument, or 0 otherwise."),
      //
      _condition_desc(579, "GetEquippedShout",           "Returns 1 if this actor has the specified shout equipped, or 0 otherwise."),
      _condition_desc(580, "IsBleedingOut",              "Returns 1 if this actor is in bleedout, or 0 otherwise."),
      //
      _condition_desc(584, "GetRelativeAngle",           ""),
      //
      _condition_desc(589, "GetMovementDirection",       ""),
      _condition_desc(590, "IsInScene",                  ""),
      _condition_desc(591, "GetRefTypeDeadCount",        ""),
      _condition_desc(592, "GetRefTypeAliveCount",       ""),
      //
      _condition_desc(594, "GetIsFlying",                "Returns 1 if this actor is flying or is in the middle of a landing animation, or 0 otherwise."),
      _condition_desc(595, "IsCurrentSpell",             ""),
      _condition_desc(596, "SpellHasKeyword",            ""),
      _condition_desc(597, "GetEquippedItemType",        ""),
      _condition_desc(598, "GetLocationAliasCleared",    ""),
      //
      _condition_desc(600, "GetLocAliasRefTypeDeadCount",  ""),
      _condition_desc(601, "GetLocAliasRefTypeAliveCount", ""),
      _condition_desc(602, "IsWardState", ""),
      _condition_desc(603, "IsInSameCurrentLocAsRef", ""),
      _condition_desc(604, "IsInSameCurrentLocAsRefAlias", ""),
      _condition_desc(605, "LocAliasIsLocation", ""),
      _condition_desc(606, "GetKeywordDataForLocation", ""),
      //
      _condition_desc(608, "GetKeywordDataForAlias", ""),
      //
      _condition_desc(610, "LocAliasHasKeyword", ""),
      _condition_desc(611, "IsNullPackageData", ""),
      _condition_desc(612, "GetNumericPackageData", ""),
      _condition_desc(613, "IsFurnitureAnimType", ""),
      _condition_desc(614, "IsFurnitureEntryType", ""),
      _condition_desc(615, "GetHighestRelationshipRank", ""),
      _condition_desc(616, "GetLowestRelationshipRank", ""),
      _condition_desc(617, "HasAssociationTypeAny", ""),
      _condition_desc(618, "HasFamilyRelationshipAny", ""),
      _condition_desc(619, "GetPathingTargetOffset", ""),
      _condition_desc(620, "GetPathingTargetAngleOffset", ""),
      _condition_desc(621, "GetPathingTargetSpeed", ""),
      _condition_desc(622, "GetPathingTargetSpeedAngle", ""),
      _condition_desc(623, "GetMovementSpeed", ""),
      _condition_desc(624, "GetInContainer", "Returns 1 if this reference is inside of a container's inventory, or 0 otherwise. Items in containers don't normally have ObjectReferences, but there are a small number of exceptions, including alias refs."),
      _condition_desc(625, "IsLocationLoaded", ""),
      _condition_desc(626, "IsLocAliasLoaded", ""),
      _condition_desc(627, "IsDualCasting", ""),
      //
      _condition_desc(629, "GetVMQuestVariable",  ""),
      _condition_desc(630, "GetVMScriptVariable", ""),
      _condition_desc(631, "IsEnteringInteractionQuick", ""),
      _condition_desc(632, "IsCasting", ""),
      _condition_desc(633, "GetFlyingState",             "Returns this actor's flying state, as one of the following values: none (0), takeoff (1), cruising (2), hovering (3), or landing (4)."),
      //
      _condition_desc(635, "IsInFavorState", ""),
      _condition_desc(636, "HasTwoHandedWeaponEquipped", "Returns 1 if this actor has a greatsword, battleaxe, bow, or crossbow equipped, or 0 otherwise."),
      _condition_desc(637, "IsExitingInstant", ""),
      _condition_desc(638, "IsInFriendStateWithPlayer",  ""),
      _condition_desc(639, "GetWithinDistance",          "Returns 1 if this reference is within the specified distance of the specified reference, or 0 otherwise."),
      _condition_desc(640, "GetActorValuePercent",       "Returns the value of the specified ActorValue on this actor, normalized to a range from 0 to 1."),
      _condition_desc(641, "IsUnique",                   "Returns 1 if this reference is an actor whose ActorBase is flagged as unique, or 0 otherwise."),
      _condition_desc(642, "GetLastBumpDirection", ""),
      //
      _condition_desc(644, "IsInFurnitureState",         "Returns 1 if this actor is currently in the idle animation for a furniture's sit marker, lean marker, or sleep marker."),
      _condition_desc(645, "GetIsInjured",               "Returns 1 if this actor is injured, or 0 otherwise. Actors will generally gain the \"injured\" status when their health falls below their race's Injured Health Percentage."),
      _condition_desc(646, "GetIsCrashLandRequest", ""),
      _condition_desc(647, "GetIsHastyLandRequest", ""),
      //
      _condition_desc(650, "IsLinkedTo", ""),
      _condition_desc(651, "GetKeywordDataForCurrentLocation", ""),
      _condition_desc(652, "GetInSharedCrimeFaction", ""),
      _condition_desc(652, "GetInSharedCrimeFaction", ""),
      //
      _condition_desc(654, "GetBribeSuccess",      ""),
      _condition_desc(655, "GetIntimidateSuccess", "Returns 1 if the player is capable of intimidating this actor, or 0 otherwise."),
      _condition_desc(656, "GetArrestedState",     "Returns 1 if this actor is being arrested, or 0 otherwise."),
      _condition_desc(657, "GetArrestingActor",    "Returns 1 if this actor is arresting someone, or 0 otherwise."),
      //
      _condition_desc(659, "EPTemperingItemIsEnchanted", "A function suitable for use in a perk entry point."),
      _condition_desc(660, "EPTemperingItemHasKeyword",  "A function suitable for use in a perk entry point."),
      //
      _condition_desc(664, "GetReplacedItemType", ""),
      //
      _condition_desc(672, "IsAttacking", ""),
      _condition_desc(673, "IsPowerAttacking", ""),
      _condition_desc(674, "IsLastHostileActor", ""),
      _condition_desc(675, "GetGraphVariableInt",                     "Returns the value of an integer variable on this reference's animation graph. If that isn't possible, returns 0."),
      _condition_desc(676, "GetCurrentShoutVariation", ""),
      //
      _condition_desc(678, "ShouldAttackKill", ""),
      //
      _condition_desc(681, "EPMagic_IsAdvanceSkill", ""),
      _condition_desc(682, "WornHasKeyword",                          "Returns 1 if this reference is wearing any items with the specified keyword."),
      _condition_desc(683, "GetPathingCurrentSpeed", ""),
      _condition_desc(684, "GetPathingCurrentSpeedAngle", ""),
      //
      _condition_desc(691, "EPModSkillUsage_AdvanceObjectHasKeyword", ""),
      _condition_desc(692, "EPModSkillUsage_IsAdvanceAction", ""),
      _condition_desc(693, "EPMagic_SpellHasKeyword", ""),
      _condition_desc(694, "GetNoBleedoutRecovery", ""),
      //
      _condition_desc(696, "EPMagic_SpellHasSkill", ""),
      _condition_desc(697, "IsAttackType", ""),
      _condition_desc(698, "IsAllowedToFly",             "Returns 1 if this actor is of a race that is flagged as capable of flight, if the actor's \"Injured\" animation graph variable isn't true, if nothing is blocking the actor from flying, and if its health is any value other than zero. Returns 0 otherwise."),
      _condition_desc(699, "HasMagicEffectKeyword", ""),
      _condition_desc(700, "IsCommandedActor", ""),
      _condition_desc(701, "IsStaggered",                "Returns 1 if this actor is currently staggered, or 0 otherwise."),
      _condition_desc(702, "IsRecoiling", ""),
      _condition_desc(703, "IsExitingInteractionQuick",  ""),
      _condition_desc(704, "IsPathing", ""),
      _condition_desc(705, "GetShouldHelp", ""),
      _condition_desc(706, "HasBoundWeaponEquipped",     "Returns 1 if this actor has a bound weapon equipped to the specified slot, or 0 otherwise. If a casting source other than the hands is selected, this function tests the right hand."),
      _condition_desc(707, "GetCombatTargetHasKeyword",  ""),
      //
      _condition_desc(709, "GetCombatGroupMemberCount",  "Returns the number of allies that are currently fighting at this actor's side, if this actor is in combat with at least one enemy. Returns 0 otherwise."),
      _condition_desc(710, "IsIgnoringCombat",           "Returns 1 if this actor is ignoring combat, or 0 otherwise."),
      _condition_desc(711, "GetLightLevel",              "Returns the actor's light level. This is how well-lit the game engine believes the actor is; the value is used for detection and stealth, and may not accurately represent the area's visuals."),
      //
      _condition_desc(713, "SpellHasCastingPerk", ""),
      _condition_desc(714, "IsBeingRidden", ""),
      _condition_desc(715, "IsUndead",                   "Returns 1 if this actor is undead (i.e. if they were reanimated or if they have the ActorTypeUndead keyword), or 0 otherwise."),
      _condition_desc(716, "GetRealHoursPassed", ""),
      //
      _condition_desc(718, "IsUnlockedDoor",             "Returns 1 if this reference's base form is a door and if the reference is unlocked, or 0 otherwise."),
      _condition_desc(719, "IsHostileToActor",           "If this reference and the specified reference are both actors, returns 1 if this actor is hostile to the specified actor, or 0 if they are not. If either reference is not an actor, the return value is undefined."),
      _condition_desc(720, "GetTargetHeight",            "Subtracts the Z-coordinate of the specified reference from the Z-coordinate of this reference, and returns the result. If either reference does not exist, returns 0."),
      _condition_desc(721, "IsPoison", ""),
      _condition_desc(722, "WornApparelHasKeywordCount", "Returns the number of items in this reference's inventory that are currently being worn and that have the specified keyword."),
      _condition_desc(723, "GetItemHealthPercent", ""),
      _condition_desc(724, "EffectWasDualCast", ""),
      _condition_desc(725, "GetKnockedStateEnum",        "If this reference is an actor, this function returns its internal knock state enum; otherwise, the function returns -1. The knocked state enum values range from 0 to 8 and are, respectively: normal; explode; explode lead-in; knocked out; knock-out lead-in; queued; getting up; down."),
      _condition_desc(726, "DoesNotExist",               "Returns 1 if this reference does not exist. This function could be used to test if a quest alias or package data is empty."),
      //
      _condition_desc(730, "IsOnFlyingMount", ""),
      _condition_desc(731, "CanFlyHere", "Returns 1 if this reference is in a worldspace and if that worldspace has max height data, or 0 otherwise. Max height data is used by flying actors to avoid clipping into terrain."),
      _condition_desc(732, "IsFlyingMountPatrolQueued", ""),
      _condition_desc(733, "IsFlyingMountFastTravelling", ""),
      _condition_desc(734, "IsOverencumbered", ""), // TODO: SSE only
      _condition_desc(735, "GetActorWarmth",   ""), // TODO: SSE only
      //
      // Extensions:
      //
      _condition_desc(1024, "GetSKSEVersion", ""),
      _condition_desc(1025, "GetSKSEVersionMinor", ""),
      _condition_desc(1026, "GetSKSEVersionBeta", ""),
      _condition_desc(1027, "GetSKSERelease", ""),
      _condition_desc(1028, "ClearInvalidRegistrations", ""),
   };
}

extern QString get__condition_desc_description(uint16_t id);