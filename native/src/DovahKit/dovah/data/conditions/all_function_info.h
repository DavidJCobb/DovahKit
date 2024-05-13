#pragma once
#include "./all_parameter_types.h"
#include "./function_info.h"

namespace dovah::conditions {
   constexpr const auto gapless_function_info_list = std::array{
      function_info(  0, "GetWantBlocking"),
      function_info(  1, "GetDistance"),
      function_info(  5, "GetLocked"),
      function_info(  6, "GetPos",                                  parameter_types::Axis),
      function_info(  8, "GetAngle",                                parameter_types::Axis),
      function_info( 10, "GetStartingPos",                          parameter_types::Axis),
      function_info( 11, "GetStartingAngle",                        parameter_types::Axis),
      function_info( 12, "GetSecondsPassed"),
      function_info( 14, "GetActorValue",                           parameter_types::ActorValue),
      function_info( 18, "GetCurrentTime"),
      function_info( 24, "GetScale"),
      function_info( 25, "IsMoving"),
      function_info( 26, "IsTurning"),
      function_info( 27, "GetLineOfSight",                           parameter_types::ObjectReference),
      function_info( 32, "GetInSameCell",                            parameter_types::ObjectReference),
      function_info( 35, "GetDisabled"),
      function_info( 36, "MenuMode",                                 parameter_types::Integer),
      function_info( 39, "GetDisease"),
      function_info( 41, "GetClothingValue"),
      function_info( 42, "SameFaction",                              parameter_types::Actor),
      function_info( 43, "SameRace",                                 parameter_types::Actor),
      function_info( 44, "SameSex",                                  parameter_types::Actor),
      function_info( 45, "GetDetected",                              parameter_types::Actor),
      function_info( 46, "GetDead"),
      function_info( 47, "GetItemCount",                             parameter_types::InventoryItem),
      function_info( 48, "GetGold"),
      function_info( 49, "GetSleeping"),
      function_info( 50, "GetTalkedToPC"),
      function_info( 53, "GetScriptVariable",                        parameter_types::ObjectReference,   parameter_types::String),
      function_info( 56, "GetQuestRunning",                          parameter_types::Quest),
      function_info( 58, "GetStage",                                 parameter_types::Quest),
      function_info( 59, "GetStageDone",                             parameter_types::Quest,             parameter_types::QuestStage),
      function_info( 60, "GetFactionRankDifference",                 parameter_types::Faction,           parameter_types::Actor),
      function_info( 61, "GetAlarmed"),
      function_info( 62, "IsRaining"),
      function_info( 63, "GetAttacked"),
      function_info( 64, "GetIsCreature"),
      function_info( 65, "GetLockLevel"),
      function_info( 66, "GetShouldAttack",                          parameter_types::Actor),
      function_info( 67, "GetInCell",                                parameter_types::Cell),
      function_info( 68, "GetIsClass",                               parameter_types::Class),
      function_info( 69, "GetIsRace",                                parameter_types::Race),
      function_info( 70, "GetIsSex",                                 parameter_types::Sex),
      function_info( 71, "GetInFaction",                             parameter_types::Faction),
      function_info( 72, "GetIsID",                                  parameter_types::BaseForm),
      function_info( 73, "GetFactionRank",                           parameter_types::Faction),
      function_info( 74, "GetGlobalValue",                           parameter_types::Global),
      function_info( 75, "IsSnowing"),
      function_info( 77, "GetRandomPercent"),
      function_info( 79, "GetQuestVariable",                         parameter_types::Quest,             parameter_types::String),
      function_info( 80, "GetLevel"),
      function_info( 81, "IsRotating"),
      function_info( 84, "GetDeadCount",                             parameter_types::ActorBase),
      function_info( 91, "GetIsAlerted"),
      function_info( 98, "GetPlayerControlsDisabled",                parameter_types::Integer,           parameter_types::Integer),
      function_info( 99, "GetHeadingAngle",                          parameter_types::ObjectReference),
      function_info(101, "IsWeaponMagicOut"),
      function_info(102, "IsTorchOut"),
      function_info(103, "IsShieldOut"),
      function_info(106, "IsFacingUp"),
      function_info(107, "GetKnockedState"),
      function_info(108, "GetWeaponAnimType"),
      function_info(109, "IsWeaponSkillType",                       parameter_types::ActorValue),
      function_info(110, "GetCurrentAIPackage"),
      function_info(111, "IsWaiting"),
      function_info(112, "IsIdlePlaying"),
      function_info(116, "IsIntimidatedByPlayer"),
      function_info(117, "IsPlayerInRegion",                        parameter_types::Region),
      function_info(118, "GetActorAggroRadiusViolated"),
      function_info(122, "GetCrime",                                parameter_types::Actor,             parameter_types::CrimeType),
      function_info(123, "IsGreetingPlayer"),
      function_info(125, "IsGuard"),
      function_info(127, "HasBeenEaten"),
      function_info(128, "GetStaminaPercentage"),
      function_info(129, "GetPCIsClass",                            parameter_types::Class),
      function_info(130, "GetPCIsRace",                             parameter_types::Race),
      function_info(131, "GetPCIsSex",                              parameter_types::Sex),
      function_info(132, "GetPCInFaction",                          parameter_types::Faction),
      function_info(133, "SameFactionAsPC"),
      function_info(134, "SameRaceAsPC"),
      function_info(135, "SameSexAsPC"),
      function_info(136, "GetIsReference",                          parameter_types::ObjectReference),
      function_info(141, "IsTalking"),
      function_info(142, "GetWalkSpeed"),
      function_info(143, "GetCurrentAIProcedure"),
      function_info(144, "GetTrespassWarningLevel"),
      function_info(145, "IsTrespassing"),
      function_info(146, "IsInMyOwnedCell"),
      function_info(147, "GetWindSpeed"),
      function_info(148, "GetCurrentWeatherPercent"),
      function_info(149, "GetIsCurrentWeather",                     parameter_types::Weather),
      function_info(150, "IsContinuingPackagePCNear"),
      function_info(152, "GetIsCrimeFaction",                       parameter_types::Faction),
      function_info(153, "CanHaveFlames"),
      function_info(154, "HasFlames"),
      function_info(157, "GetOpenState"),
      function_info(159, "GetSitting"),
      function_info(161, "GetIsCurrentPackage",                     parameter_types::Package),
      function_info(162, "IsCurrentFurnitureRef",                   parameter_types::ObjectReference),
      function_info(163, "IsCurrentFurnitureObj",                   parameter_types::Furniture),
      function_info(170, "GetDayOfWeek"),
      function_info(172, "GetTalkedToPCParam",                      parameter_types::Actor),
      function_info(175, "IsPCSleeping"),
      function_info(176, "IsPCAMurderer"),
      function_info(180, "HasSameEditorLocAsRef",                   parameter_types::ObjectReference,   parameter_types::Keyword),
      function_info(181, "HasSameEditorLocAsRefAlias",              parameter_types::Alias,             parameter_types::Keyword),
      function_info(182, "GetEquipped",                             parameter_types::InventoryItem),
      function_info(185, "IsSwimming"),
      function_info(190, "GetAmountGoldStolen"),
      function_info(192, "GetIgnoreCrime"),
      function_info(193, "GetPCExpelled",                           parameter_types::Faction),
      function_info(195, "GetPCFactionMurder",                      parameter_types::Faction),
      function_info(197, "GetPCEnemyofFaction",                     parameter_types::Faction),
      function_info(199, "GetPCFactionAttack",                      parameter_types::Faction),
      function_info(203, "GetDestroyed"),
      function_info(214, "HasMagicEffect",                          parameter_types::MagicEffect),
      function_info(215, "GetDefaultOpen"),
      function_info(219, "GetAnimAction"),
      function_info(223, "IsSpellTarget",                           parameter_types::Spell),
      function_info(224, "GetVATSMode"),
      function_info(225, "GetPersuasionNumber"),
      function_info(226, "GetVampireFeed"),
      function_info(227, "GetCannibal"),
      function_info(228, "GetIsClassDefault",                       parameter_types::Class),
      function_info(229, "GetClassDefaultMatch"),
      function_info(230, "GetInCellParam",                          parameter_types::Cell,              parameter_types::ObjectReference),
      function_info(235, "GetVatsTargetHeight"),
      function_info(237, "GetIsGhost"),
      function_info(242, "GetUnconscious"),
      function_info(244, "GetRestrained"),
      function_info(246, "GetIsUsedItem",                           parameter_types::BaseForm),
      function_info(247, "GetIsUsedItemType",                       parameter_types::FormType),
      function_info(248, "IsScenePlaying",                          parameter_types::Scene),
      function_info(249, "IsInDialogueWithPlayer"),
      function_info(250, "GetLocationCleared",                      parameter_types::Location),
      function_info(254, "GetIsPlayableRace"),
      function_info(255, "GetOffersServicesNow"),
      function_info(258, "HasAssociationType",                      parameter_types::Actor,             parameter_types::AssociationType),
      function_info(259, "HasFamilyRelationship",                   parameter_types::Actor),
      function_info(261, "HasParentRelationship",                   parameter_types::Actor),
      function_info(262, "IsWarningAbout",                          parameter_types::FormList),
      function_info(263, "IsWeaponOut"),
      function_info(264, "HasSpell",                                parameter_types::Spell),
      function_info(265, "IsTimePassing"),
      function_info(266, "IsPleasant"),
      function_info(267, "IsCloudy"),
      function_info(274, "IsSmallBump"),
      function_info(277, "GetBaseActorValue",                       parameter_types::ActorValue),
      function_info(278, "IsOwner",                                 parameter_types::OwnerForm),
      function_info(280, "IsCellOwner",                             parameter_types::Cell,              parameter_types::OwnerForm),
      function_info(282, "IsHorseStolen"),
      function_info(285, "IsLeftUp"),
      function_info(286, "IsSneaking"),
      function_info(287, "IsRunning"),
      function_info(288, "GetFriendHit"),
      function_info(289, "IsInCombat",                              parameter_types::Integer),
      function_info(300, "IsInInterior"),
      function_info(304, "IsWaterObject"),
      function_info(305, "GetPlayerAction"),
      function_info(306, "IsActorUsingATorch"),
      function_info(309, "IsXBox"),
      function_info(310, "GetInWorldspace",                         parameter_types::Worldspace),
      function_info(312, "GetPCMiscStat",                           parameter_types::MiscStat),
      function_info(313, "GetPairedAnimation"),
      function_info(314, "IsActorAVictim"),
      function_info(315, "GetTotalPersuasionNumber"),
      function_info(318, "GetIdleDoneOnce"),
      function_info(320, "GetNoRumors"),
      function_info(323, "GetCombatState"),
      function_info(325, "GetWithinPackageLocation",                parameter_types::PackageData),
      function_info(327, "IsRidingMount"),
      function_info(329, "IsFleeing"),
      function_info(332, "IsInDangerousWater"),
      function_info(338, "GetIgnoreFriendlyHits"),
      function_info(339, "IsPlayersLastRiddenMount"),
      function_info(353, "IsActor"),
      function_info(354, "IsEssential"),
      function_info(358, "IsPlayerMovingIntoNewSpace"),
      function_info(359, "GetInCurrentLoc",                         parameter_types::Location),
      function_info(360, "GetInCurrentLocAlias",                    parameter_types::Alias),
      function_info(361, "GetTimeDead"),
      function_info(362, "HasLinkedRef",                            parameter_types::Keyword),
      function_info(365, "IsChild"),
      function_info(366, "GetStolenItemValueNoCrime",               parameter_types::Faction),
      function_info(367, "GetLastPlayerAction"),
      function_info(368, "IsPlayerActionActive",                    parameter_types::Integer),
      function_info(370, "IsTalkingActivatorActor",                 parameter_types::Actor),
      function_info(372, "IsInList",                                parameter_types::FormList),
      function_info(373, "GetStolenItemValue",                      parameter_types::Faction),
      function_info(375, "GetCrimeGoldViolent",                     parameter_types::Faction),
      function_info(376, "GetCrimeGoldNonViolent",                  parameter_types::Faction),
      function_info(378, "HasShout",                                parameter_types::Shout),
      function_info(381, "GetHasNote",                              parameter_types::Note),
      function_info(390, "GetHitLocation"),
      function_info(391, "IsPC1stPerson"),
      function_info(396, "GetCauseofDeath"),
      function_info(397, "IsLimbGone",                              parameter_types::Integer), // CK actually does expose this as an int
      function_info(398, "IsWeaponInList",                          parameter_types::FormList),
      function_info(402, "IsBribedByPlayer"),
      function_info(403, "GetRelationshipRank",                     parameter_types::ObjectReference),
      function_info(407, "GetVATSValue",                            parameter_types::VATSValueFunction, parameter_types::VATSValue),
      function_info(408, "IsKiller",                                parameter_types::Actor),
      function_info(409, "IsKillerObject",                          parameter_types::FormList),
      function_info(410, "GetFactionCombatReaction",                parameter_types::Faction,           parameter_types::Faction),
      function_info(414, "Exists",                                  parameter_types::ObjectReference),
      function_info(415, "GetGroupMemberCount"),
      function_info(416, "GetGroupTargetCount"),
      function_info(426, "GetIsVoiceType",                          parameter_types::Voicetype),
      function_info(427, "GetPlantedExplosive"),
      function_info(429, "IsScenePackageRunning"),
      function_info(430, "GetHealthPercentage"),
      function_info(432, "GetIsObjectType",                         parameter_types::FormType),
      function_info(434, "GetDialogueEmotion"),
      function_info(435, "GetDialogueEmotionValue"),
      function_info(437, "GetIsCreatureType",                       parameter_types::Integer),
      function_info(444, "GetInCurrentLocFormList",                 parameter_types::FormList),
      function_info(445, "GetInZone",                               parameter_types::EncounterZone),
      function_info(446, "GetVelocity",                             parameter_types::Axis),
      function_info(447, "GetGraphVariableFloat",                   parameter_types::String),
      function_info(448, "HasPerk",                                 parameter_types::Perk),
      function_info(449, "GetFactionRelation",                      parameter_types::Actor),
      function_info(450, "IsLastIdlePlayed",                        parameter_types::Idle),
      function_info(453, "GetPlayerTeammate"),
      function_info(454, "GetPlayerTeammateCount"),
      function_info(458, "GetActorCrimePlayerEnemy"),
      function_info(459, "GetCrimeGold",                            parameter_types::Faction),
      function_info(463, "IsPlayerGrabbedRef",                      parameter_types::ObjectReference),
      function_info(465, "GetKeywordItemCount",                     parameter_types::Keyword),
      function_info(470, "GetDestructionStage"),
      function_info(473, "GetIsAlignment",                          parameter_types::Alignment),
      function_info(476, "IsProtected"),
      function_info(477, "GetThreatRatio",                          parameter_types::Actor),
      function_info(479, "GetIsUsedItemEquipType",                  parameter_types::EquipType),
      function_info(487, "IsCarryable"),
      function_info(488, "GetConcussed"),
      function_info(491, "GetMapMarkerVisible"),
      function_info(493, "PlayerKnows",                             parameter_types::KnowableForm),
      function_info(494, "GetPermanentActorValue",                  parameter_types::ActorValue),
      function_info(495, "GetKillingBlowLimb"),
      function_info(497, "CanPayCrimeGold"),
      function_info(499, "GetDaysInJail"),
      function_info(500, "EPAlchemyGetMakingPoison"),
      function_info(501, "EPAlchemyEffectHasKeyword",               parameter_types::Keyword),
      function_info(503, "GetAllowWorldInteractions"),
      function_info(508, "GetLastHitCritical"),
      function_info(513, "IsCombatTarget",                          parameter_types::Actor),
      function_info(515, "GetVATSRightAreaFree",                    parameter_types::ObjectReference),
      function_info(516, "GetVATSLeftAreaFree",                     parameter_types::ObjectReference),
      function_info(517, "GetVATSBackAreaFree",                     parameter_types::ObjectReference),
      function_info(518, "GetVATSFrontAreaFree",                    parameter_types::ObjectReference),
      function_info(519, "GetLockIsBroken"),
      function_info(520, "IsPS3"),
      function_info(521, "IsWin32"),
      function_info(522, "GetVATSRightTargetVisible",               parameter_types::ObjectReference),
      function_info(523, "GetVATSLeftTargetVisible",                parameter_types::ObjectReference),
      function_info(524, "GetVATSBackTargetVisible",                parameter_types::ObjectReference),
      function_info(525, "GetVATSFrontTargetVisible",               parameter_types::ObjectReference),
      function_info(528, "IsInCriticalStage",                       parameter_types::CriticalStage),
      function_info(530, "GetXPForNextLevel"),
      function_info(533, "GetInfamy",                               parameter_types::Faction),
      function_info(534, "GetInfamyViolent",                        parameter_types::Faction),
      function_info(535, "GetInfamyNonViolent",                     parameter_types::Faction),
      function_info(543, "GetQuestCompleted",                       parameter_types::Quest),
      function_info(547, "IsGoreDisabled"),
      function_info(550, "IsSceneActionComplete",                   parameter_types::Scene,             parameter_types::Integer), // TODO: the int is an action index
      function_info(552, "GetSpellUsageNum",                        parameter_types::Spell),
      function_info(554, "GetActorsInHigh"),
      function_info(555, "HasLoaded3D"),
      function_info(560, "HasKeyword",                              parameter_types::Keyword),
      function_info(561, "HasRefType",                              parameter_types::LocRefType),
      function_info(562, "LocationHasKeyword",                      parameter_types::Keyword),
      function_info(563, "LocationHasRefType",                      parameter_types::LocRefType),
      function_info(565, "GetIsEditorLocation",                     parameter_types::Location),
      function_info(566, "GetIsAliasRef",                           parameter_types::Alias),
      function_info(567, "GetIsEditorLocAlias",                     parameter_types::Alias),
      function_info(568, "IsSprinting"),
      function_info(569, "IsBlocking"),
      function_info(570, "HasEquippedSpell",                        parameter_types::CastingSource),
      function_info(571, "GetCurrentCastingType",                   parameter_types::CastingSource),
      function_info(572, "GetCurrentDeliveryType",                  parameter_types::CastingSource),
      function_info(574, "GetAttackState"),
      function_info::make_event_data_function(576, "GetEventData"),
      function_info(577, "IsCloserToAThanB",                        parameter_types::ObjectReference,   parameter_types::ObjectReference),
      function_info(579, "GetEquippedShout",                        parameter_types::Shout),
      function_info(580, "IsBleedingOut"),
      function_info(584, "GetRelativeAngle",                        parameter_types::ObjectReference,   parameter_types::Axis),
      function_info(589, "GetMovementDirection"),
      function_info(590, "IsInScene"),
      function_info(591, "GetRefTypeDeadCount",                     parameter_types::Location,          parameter_types::LocRefType),
      function_info(592, "GetRefTypeAliveCount",                    parameter_types::Location,          parameter_types::LocRefType),
      function_info(594, "GetIsFlying"),
      function_info(595, "IsCurrentSpell",                          parameter_types::Spell,             parameter_types::CastingSource),
      function_info(596, "SpellHasKeyword",                         parameter_types::CastingSource,     parameter_types::Keyword),
      function_info(597, "GetEquippedItemType",                     parameter_types::CastingSource),
      function_info(598, "GetLocationAliasCleared",                 parameter_types::Alias),
      function_info(600, "GetLocAliasRefTypeDeadCount",             parameter_types::Alias,             parameter_types::LocRefType),
      function_info(601, "GetLocAliasRefTypeAliveCount",            parameter_types::Alias,             parameter_types::LocRefType),
      function_info(602, "IsWardState",                             parameter_types::WardState),
      function_info(603, "IsInSameCurrentLocAsRef",                 parameter_types::ObjectReference,   parameter_types::Keyword),
      function_info(604, "IsInSameCurrentLocAsRefAlias",            parameter_types::Alias,             parameter_types::Keyword),
      function_info(605, "LocAliasIsLocation",                      parameter_types::Alias,             parameter_types::Location),
      function_info(606, "GetKeywordDataForLocation",               parameter_types::Location,          parameter_types::Keyword),
      function_info(608, "GetKeywordDataForAlias",                  parameter_types::Alias,             parameter_types::Keyword),
      function_info(610, "LocAliasHasKeyword",                      parameter_types::Alias,             parameter_types::Keyword),
      function_info(611, "IsNullPackageData",                       parameter_types::PackageData),
      function_info(612, "GetNumericPackageData",                   parameter_types::PackageData),
      function_info(613, "IsFurnitureAnimType",                     parameter_types::FurnitureAnim),
      function_info(614, "IsFurnitureEntryType",                    parameter_types::FurnitureEntry),
      function_info(615, "GetHighestRelationshipRank"),
      function_info(616, "GetLowestRelationshipRank"),
      function_info(617, "HasAssociationTypeAny",                   parameter_types::AssociationType),
      function_info(618, "HasFamilyRelationshipAny"),
      function_info(619, "GetPathingTargetOffset",                  parameter_types::Axis),
      function_info(620, "GetPathingTargetAngleOffset",             parameter_types::Axis),
      function_info(621, "GetPathingTargetSpeed"),
      function_info(622, "GetPathingTargetSpeedAngle",              parameter_types::Axis),
      function_info(623, "GetMovementSpeed"),
      function_info(624, "GetInContainer",                          parameter_types::ObjectReference),
      function_info(625, "IsLocationLoaded",                        parameter_types::Location),
      function_info(626, "IsLocAliasLoaded",                        parameter_types::Alias),
      function_info(627, "IsDualCasting"),
      function_info(629, "GetVMQuestVariable",                      parameter_types::Quest,             parameter_types::String),
      function_info(630, "GetVMScriptVariable",                     parameter_types::ObjectReference,   parameter_types::String),
      function_info(631, "IsEnteringInteractionQuick"),
      function_info(632, "IsCasting"),
      function_info(633, "GetFlyingState"),
      function_info(635, "IsInFavorState"),
      function_info(636, "HasTwoHandedWeaponEquipped"),
      function_info(637, "IsExitingInstant"),
      function_info(638, "IsInFriendStateWithPlayer"),
      function_info(639, "GetWithinDistance",                       parameter_types::ObjectReference,   parameter_types::Float),
      function_info(640, "GetActorValuePercent",                    parameter_types::ActorValue),
      function_info(641, "IsUnique"),
      function_info(642, "GetLastBumpDirection"),
      function_info(644, "IsInFurnitureState",                      parameter_types::FurnitureAnim),
      function_info(645, "GetIsInjured"),
      function_info(646, "GetIsCrashLandRequest"),
      function_info(647, "GetIsHastyLandRequest"),
      function_info(650, "IsLinkedTo",                              parameter_types::ObjectReference,   parameter_types::Keyword),
      function_info(651, "GetKeywordDataForCurrentLocation",        parameter_types::Keyword),
      function_info(652, "GetInSharedCrimeFaction",                 parameter_types::ObjectReference),
      function_info(654, "GetBribeSuccess"),
      function_info(655, "GetIntimidateSuccess"),
      function_info(656, "GetArrestedState"),
      function_info(657, "GetArrestingActor"),
      function_info(659, "EPTemperingItemIsEnchanted"),
      function_info(660, "EPTemperingItemHasKeyword",               parameter_types::Keyword),
      function_info(664, "GetReplacedItemType",                     parameter_types::CastingSource),
      function_info(672, "IsAttacking"),
      function_info(673, "IsPowerAttacking"),
      function_info(674, "IsLastHostileActor"),
      function_info(675, "GetGraphVariableInt",                     parameter_types::String),
      function_info(676, "GetCurrentShoutVariation"),
      function_info(678, "ShouldAttackKill",                        parameter_types::Actor),
      function_info(681, "EPMagic_IsAdvanceSkill",                  parameter_types::ActorValue),
      function_info(682, "WornHasKeyword",                          parameter_types::Keyword),
      function_info(683, "GetPathingCurrentSpeed"),
      function_info(684, "GetPathingCurrentSpeedAngle",             parameter_types::Axis),
      function_info(691, "EPModSkillUsage_AdvanceObjectHasKeyword", parameter_types::Keyword),
      function_info(692, "EPModSkillUsage_IsAdvanceAction",         parameter_types::AdvanceAction),
      function_info(693, "EPMagic_SpellHasKeyword",                 parameter_types::Keyword),
      function_info(694, "GetNoBleedoutRecovery"),
      function_info(696, "EPMagic_SpellHasSkill",                   parameter_types::ActorValue),
      function_info(697, "IsAttackType",                            parameter_types::Keyword),
      function_info(698, "IsAllowedToFly"),
      function_info(699, "HasMagicEffectKeyword",                   parameter_types::Keyword),
      function_info(700, "IsCommandedActor"),
      function_info(701, "IsStaggered"),
      function_info(702, "IsRecoiling"),
      function_info(703, "IsExitingInteractionQuick"),
      function_info(704, "IsPathing"),
      function_info(705, "GetShouldHelp",                           parameter_types::Actor),
      function_info(706, "HasBoundWeaponEquipped",                  parameter_types::CastingSource),
      function_info(707, "GetCombatTargetHasKeyword",               parameter_types::Keyword),
      function_info(709, "GetCombatGroupMemberCount"),
      function_info(710, "IsIgnoringCombat"),
      function_info(711, "GetLightLevel"),
      function_info(713, "SpellHasCastingPerk",                     parameter_types::Perk),
      function_info(714, "IsBeingRidden"),
      function_info(715, "IsUndead"),
      function_info(716, "GetRealHoursPassed"),
      function_info(718, "IsUnlockedDoor"),
      function_info(719, "IsHostileToActor",                        parameter_types::Actor),
      function_info(720, "GetTargetHeight",                         parameter_types::ObjectReference),
      function_info(721, "IsPoison"),
      function_info(722, "WornApparelHasKeywordCount",              parameter_types::Keyword),
      function_info(723, "GetItemHealthPercent"),
      function_info(724, "EffectWasDualCast"),
      function_info(725, "GetKnockedStateEnum"),
      function_info(726, "DoesNotExist"),
      function_info(730, "IsOnFlyingMount"),
      function_info(731, "CanFlyHere"),
      function_info(732, "IsFlyingMountPatrolQueued"),
      function_info(733, "IsFlyingMountFastTravelling"),
      function_info(734, "IsOverencumbered").mark_as_sse_only(),
      function_info(735, "GetActorWarmth").mark_as_sse_only(),
   };

   static_assert(
      []() -> bool {
         for (const auto& function_info : gapless_function_info_list) {
            const auto* type_a = function_info.argument_types[0];
            if (type_a && type_a->is_union())
               return false;
         }
         return true;
      }(),
      "Union-type parameters determine their type based on the value of the previous-sibling parameter. "
      "Ergo it isn't valid for a condition function's first parameter to be a union-type parameter."
   );

   // Maps function IDs to `function_info` objects. Note that IDs are not contiguous; this is because 
   // they're the IDs of ObScript-style functions in general, not condition functions in specific.
   //
   // Prefer `function_info_by_id` for actual lookups, since it accounts for SKSE additions.
   constexpr const auto all_vanilla_function_info = []() {
      constexpr const auto highest_id = []() {
         size_t id = 0;
         for (const auto& item : gapless_function_info_list)
            if (item.id > id)
               id = item.id;
         return id;
      }();

      std::array<function_info, highest_id + 1> out = {};
      for (size_t i = 0; i < out.size(); ++i) {
         out[i].id    = i;
         out[i].valid = false;
      }
      for (const auto& src : gapless_function_info_list)
         out[src.id] = src;

      return out;
   }();

   constexpr const auto all_extended_function_info = std::array{
      function_info(1024, "GetSKSEVersion"),
      function_info(1025, "GetSKSEVersionMinor"),
      function_info(1026, "GetSKSEVersionBeta"),
      function_info(1027, "GetSKSERelease"),
      function_info(1028, "ClearInvalidRegistrations"),
   };

   // ---

   // Run a functor for all valid condition functions. Return true from the functor to stop looping early.
   template<typename Functor>
   constexpr void for_each_function_info(Functor&& functor, bool include_skse = true) {
      for (const auto& cf : all_vanilla_function_info) {
         if (!cf.valid)
            continue;
         if ((functor)(cf))
            return;
      }
      if (include_skse) {
         for (const auto& cf : all_extended_function_info) {
            if (!cf.valid)
               continue;
            if ((functor)(cf))
               return;
         }
      }
   };

   constexpr const function_info* function_info_by_id(uint16_t id) {
      if (id < all_vanilla_function_info.size())
         return &all_vanilla_function_info[id];
      if constexpr (!all_extended_function_info.empty()) {
         constexpr auto& list = all_extended_function_info;
         if (id >= list[0].id) {
            constexpr const bool extended_ids_are_contiguous = []() {
               for (size_t i = 1; i < list.size(); ++i)
                  if (list[i].id != list[i - 1].id + 1)
                     return false;
               return true;
            }();
            if constexpr (extended_ids_are_contiguous) {
               id -= list[0].id;
               if (id < list.size())
                  return &list[id];
            } else {
               for (const auto& item : list)
                  if (item.id == id)
                     return &item;
            }
         }
      }
      return nullptr;
   }

   constexpr const uint16_t function_id_by_name(const std::string_view name) {
      for (auto& item : all_vanilla_function_info)
         if (item.name == name)
            return item.id;
      for (auto& item : all_extended_function_info)
         if (item.name == name)
            return item.id;
      return no_function_id;
   }
}