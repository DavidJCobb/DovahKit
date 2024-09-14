#pragma once
#include <array>
#include <cstdint>
#include <string_view>
#include "./category.h"

namespace dovah::dialogue {
   struct topic_subtype {
      uint32_t         signature = 0;
      category         category  = category::topic;
      std::string_view internal_name;
      bool             is_reusable = false; // Can multiple topics of this subtype exist in the same branch / top-level in the same quest?

      // Every subtype has a corresponding game setting. It's unclear what this is used for; 
      // possibly default topic text, if the topic itself has none?
      constexpr std::string game_setting_for_name() const;
   };

   constexpr const topic_subtype* topic_subtype_by_signature(uint32_t signature);
   constexpr size_t topic_subtype_signature_to_index(uint32_t);

   constexpr size_t topic_subtype_index(const topic_subtype&);

   constexpr const topic_subtype* default_subtype_for_category(category);

   inline constexpr const std::array<topic_subtype, 0x67> all_topic_subtypes = {
      topic_subtype{
         .signature     = 'CUST',
         .category      = category::topic,
         .internal_name = "Custom",
         .is_reusable   = true,
      },
      topic_subtype{
         .signature     = 'PFGT',
         .category      = category::topic,
         .internal_name = "ForceGreet",
      },
      topic_subtype{
         .signature     = 'RUMO',
         .category      = category::topic,
         .internal_name = "Rumors",
      },
      topic_subtype{
         .signature     = 'FVDL',
         .category      = category::favor_dialogue,
         .internal_name = "Custom",
         .is_reusable   = true,
      },
      topic_subtype{
         .signature     = 'INTI',
         .category      = category::favor_dialogue,
         .internal_name = "Intimidate",
      },
      topic_subtype{
         .signature     = 'FLAT',
         .category      = category::favor_dialogue,
         .internal_name = "Flatter",
      },
      topic_subtype{
         .signature     = 'BRIB',
         .category      = category::favor_dialogue,
         .internal_name = "Bribe",
      },
      topic_subtype{
         .signature     = 'ASKG',
         .category      = category::favor_dialogue,
         .internal_name = "AskGift",
      },
      topic_subtype{
         .signature     = 'GIFF',
         .category      = category::favor_dialogue,
         .internal_name = "Gift",
      },
      topic_subtype{
         .signature     = 'ASKF',
         .category      = category::favor_dialogue,
         .internal_name = "AskFavor",
      },
      topic_subtype{
         .signature     = 'FAVO',
         .category      = category::favor_dialogue,
         .internal_name = "Favor",
      },
      topic_subtype{
         .signature     = 'SHRE',
         .category      = category::favor_dialogue,
         .internal_name = "ShowRelationships",
      },
      topic_subtype{
         .signature     = 'FOLL',
         .category      = category::favor_dialogue,
         .internal_name = "Follow",
      },
      topic_subtype{
         .signature     = 'FRJT',
         .category      = category::favor_dialogue,
         .internal_name = "Reject",
      },
      topic_subtype{
         .signature     = 'SCEN',
         .category      = category::scene,
         .internal_name = "Custom",
         .is_reusable   = true,
      },
      topic_subtype{
         .signature     = 'SHOW',
         .category      = category::favors,
         .internal_name = "Show",
      },
      topic_subtype{
         .signature     = 'AGRE',
         .category      = category::favors,
         .internal_name = "Agree",
      },
      topic_subtype{
         .signature     = 'REFU',
         .category      = category::favors,
         .internal_name = "Refuse",
      },
      topic_subtype{
         .signature     = 'FEXT',
         .category      = category::favors,
         .internal_name = "ExitFavorState",
      },
      topic_subtype{
         .signature     = 'MREF',
         .category      = category::favors,
         .internal_name = "MoralRefusal",
      },
      #pragma region Added in Dragonborn DLC
      topic_subtype{
         .signature     = 'FMLX',
         .category      = category::favors,
         .internal_name = "FlyingMountLand",
      },
      topic_subtype{
         .signature     = 'FMXL',
         .category      = category::favors,
         .internal_name = "FlyingMountCancelLand",
      },
      topic_subtype{
         .signature     = 'FMAT',
         .category      = category::favors,
         .internal_name = "FlyingMountAcceptTarget",
      },
      topic_subtype{
         .signature     = 'FMRT',
         .category      = category::favors,
         .internal_name = "FlyingMountRejectTarget",
      },
      topic_subtype{
         .signature     = 'FMNT',
         .category      = category::favors,
         .internal_name = "FlyingMountNoTarget",
      },
      topic_subtype{
         .signature     = 'FMDR',
         .category      = category::favors,
         .internal_name = "FlyingMountDestinationReached",
      },
      #pragma endregion
      topic_subtype{
         .signature     = 'ATCK',
         .category      = category::combat,
         .internal_name = "Attack",
      },
      topic_subtype{
         .signature     = 'POAT',
         .category      = category::combat,
         .internal_name = "PowerAttack",
      },
      topic_subtype{
         .signature     = 'BASH',
         .category      = category::combat,
         .internal_name = "Bash",
      },
      topic_subtype{
         .signature     = 'HIT_',
         .category      = category::combat,
         .internal_name = "Hit",
      },
      topic_subtype{
         .signature     = 'FLEE',
         .category      = category::combat,
         .internal_name = "Flee",
      },
      topic_subtype{
         .signature     = 'BLED',
         .category      = category::combat,
         .internal_name = "BleedOut",
      },
      topic_subtype{
         .signature     = 'AVTH',
         .category      = category::combat,
         .internal_name = "AvoidThreat",
      },
      topic_subtype{
         .signature     = 'DETH',
         .category      = category::combat,
         .internal_name = "Death",
      },
      topic_subtype{
         .signature     = 'GRST',
         .category      = category::combat,
         .internal_name = "GroupStrategy",
      },
      topic_subtype{
         .signature     = 'BLOC',
         .category      = category::combat,
         .internal_name = "Block",
      },
      topic_subtype{
         .signature     = 'TAUT',
         .category      = category::combat,
         .internal_name = "Taunt",
      },
      topic_subtype{
         .signature     = 'ALKL',
         .category      = category::combat,
         .internal_name = "AllyKilled",
      },
      topic_subtype{
         .signature     = 'STEA',
         .category      = category::combat,
         .internal_name = "Steal",
      },
      topic_subtype{
         .signature     = 'YIEL',
         .category      = category::combat,
         .internal_name = "Yield",
      },
      topic_subtype{
         .signature     = 'ACYI',
         .category      = category::combat,
         .internal_name = "AcceptYield",
      },
      topic_subtype{
         .signature     = 'PICC',
         .category      = category::combat,
         .internal_name = "PickpocketCombat",
      },
      topic_subtype{
         .signature     = 'ASSA',
         .category      = category::combat,
         .internal_name = "Assault",
      },
      topic_subtype{
         .signature     = 'MURD',
         .category      = category::combat,
         .internal_name = "Murder",
      },
      topic_subtype{
         .signature     = 'ASNC',
         .category      = category::combat,
         .internal_name = "AssaultNC",
      },
      topic_subtype{
         .signature     = 'MUNC',
         .category      = category::combat,
         .internal_name = "MurderNC",
      },
      topic_subtype{
         .signature     = 'PICN',
         .category      = category::combat,
         .internal_name = "PickpocketNC",
      },
      topic_subtype{
         .signature     = 'STFN',
         .category      = category::combat,
         .internal_name = "StealFromNC",
      },
      topic_subtype{
         .signature     = 'TRAN',
         .category      = category::combat,
         .internal_name = "TrespassAgainstNC",
      },
      topic_subtype{
         .signature     = 'TRES',
         .category      = category::combat,
         .internal_name = "Trespass",
      },
      topic_subtype{
         .signature     = 'WTCR',
         .category      = category::combat,
         .internal_name = "WereTransformCrime",
      },
      topic_subtype{
         .signature     = 'VPSS',
         .category      = category::combat,
         .internal_name = "VoicePowerStartShort",
      },
      topic_subtype{
         .signature     = 'VPSL',
         .category      = category::combat,
         .internal_name = "VoicePowerStartLong",
      },
      topic_subtype{
         .signature     = 'VPES',
         .category      = category::combat,
         .internal_name = "VoicePowerEndShort",
      },
      topic_subtype{
         .signature     = 'VPEL',
         .category      = category::combat,
         .internal_name = "VoicePowerEndLong",
      },
      topic_subtype{
         .signature     = 'ALIL',
         .category      = category::detection,
         .internal_name = "AlertIdle",
      },
      topic_subtype{
         .signature     = 'LOIL',
         .category      = category::detection,
         .internal_name = "LostIdle",
      },
      topic_subtype{
         .signature     = 'NOTA',
         .category      = category::detection,
         .internal_name = "NormalToAlert",
      },
      topic_subtype{
         .signature     = 'ALTC',
         .category      = category::detection,
         .internal_name = "AlertToCombat",
      },
      topic_subtype{
         .signature     = 'NOTC',
         .category      = category::detection,
         .internal_name = "NormalToCombat",
      },
      topic_subtype{
         .signature     = 'ALTN',
         .category      = category::detection,
         .internal_name = "AlertToNormal",
      },
      topic_subtype{
         .signature     = 'COTN',
         .category      = category::detection,
         .internal_name = "CombatToNormal",
      },
      topic_subtype{
         .signature     = 'COLO',
         .category      = category::detection,
         .internal_name = "CombatToLost",
      },
      topic_subtype{
         .signature     = 'LOTN',
         .category      = category::detection,
         .internal_name = "LostToNormal",
      },
      topic_subtype{
         .signature     = 'LOTC',
         .category      = category::detection,
         .internal_name = "LostToCombat",
      },
      topic_subtype{
         .signature     = 'DFDA',
         .category      = category::detection,
         .internal_name = "DetectFriendDie",
      },
      topic_subtype{
         .signature     = 'SERU',
         .category      = category::service,
         .internal_name = "ServiceRefusal",
      },
      topic_subtype{
         .signature     = 'REPA',
         .category      = category::service,
         .internal_name = "Repair",
      },
      topic_subtype{
         .signature     = 'TRAV',
         .category      = category::service,
         .internal_name = "Travel",
      },
      topic_subtype{
         .signature     = 'TRAI',
         .category      = category::service,
         .internal_name = "Training",
      },
      topic_subtype{
         .signature     = 'BAEX',
         .category      = category::service,
         .internal_name = "BarterExit",
      },
      topic_subtype{
         .signature     = 'REEX',
         .category      = category::service,
         .internal_name = "RepairExit",
      },
      topic_subtype{
         .signature     = 'RECH',
         .category      = category::service,
         .internal_name = "Recharge",
      },
      topic_subtype{
         .signature     = 'RCEX',
         .category      = category::service,
         .internal_name = "RechargeExit",
      },
      topic_subtype{
         .signature     = 'TREX',
         .category      = category::service,
         .internal_name = "TrainingExit",
      },
      topic_subtype{
         .signature     = 'OBCO',
         .category      = category::miscellaneous,
         .internal_name = "ObserveCombat",
      },
      topic_subtype{
         .signature     = 'NOTI',
         .category      = category::miscellaneous,
         .internal_name = "NoticeCorpse",
      },
      topic_subtype{
         .signature     = 'TITG',
         .category      = category::miscellaneous,
         .internal_name = "TimeToGo",
      },
      topic_subtype{
         .signature     = 'GBYE',
         .category      = category::miscellaneous,
         .internal_name = "Goodbye",
      },
      topic_subtype{
         .signature     = 'HELO',
         .category      = category::miscellaneous,
         .internal_name = "Hello",
      },
      topic_subtype{
         .signature     = 'SWMW',
         .category      = category::miscellaneous,
         .internal_name = "SwingMeleeWeapon",
      },
      topic_subtype{
         .signature     = 'FIWE',
         .category      = category::miscellaneous,
         .internal_name = "ShootBow",
      },
      topic_subtype{
         .signature     = 'ZKEY',
         .category      = category::miscellaneous,
         .internal_name = "ZKeyObject",
      },
      topic_subtype{
         .signature     = 'JUMP',
         .category      = category::miscellaneous,
         .internal_name = "Jump",
      },
      topic_subtype{
         .signature     = 'KNOO',
         .category      = category::miscellaneous,
         .internal_name = "KnockOverObject",
      },
      topic_subtype{
         .signature     = 'DEOB',
         .category      = category::miscellaneous,
         .internal_name = "DestroyObject",
      },
      topic_subtype{
         .signature     = 'STOF',
         .category      = category::miscellaneous,
         .internal_name = "StandonFurniture",
      },
      topic_subtype{
         .signature     = 'LOOB',
         .category      = category::miscellaneous,
         .internal_name = "LockedObject",
      },
      topic_subtype{
         .signature     = 'PICT',
         .category      = category::miscellaneous,
         .internal_name = "PickpocketTopic",
      },
      topic_subtype{
         .signature     = 'PURS',
         .category      = category::miscellaneous,
         .internal_name = "PursueIdleTopic",
      },
      topic_subtype{
         .signature     = 'IDAT',
         .category      = category::miscellaneous,
         .internal_name = "SharedInfo",
      },
      topic_subtype{
         .signature     = 'PCPS',
         .category      = category::miscellaneous,
         .internal_name = "PlayerCastProjectileSpell",
      },
      topic_subtype{
         .signature     = 'PCSS',
         .category      = category::miscellaneous,
         .internal_name = "PlayerCastSelfSpell",
      },
      topic_subtype{
         .signature     = 'PCSH',
         .category      = category::miscellaneous,
         .internal_name = "PlayerShout",
      },
      topic_subtype{
         .signature     = 'IDLE',
         .category      = category::miscellaneous,
         .internal_name = "Idle",
      },
      topic_subtype{
         .signature     = 'BREA',
         .category      = category::miscellaneous,
         .internal_name = "EnterSprintBreath",
      },
      topic_subtype{
         .signature     = 'ENBZ',
         .category      = category::miscellaneous,
         .internal_name = "EnterBowZoomBreath",
      },
      topic_subtype{
         .signature     = 'EXBZ',
         .category      = category::miscellaneous,
         .internal_name = "ExitBowZoomBreath",
      },
      topic_subtype{
         .signature     = 'ACAC',
         .category      = category::miscellaneous,
         .internal_name = "ActorCollidewithActor",
      },
      topic_subtype{
         .signature     = 'PIRN',
         .category      = category::miscellaneous,
         .internal_name = "PlayerinIronSights",
      },
      topic_subtype{
         .signature     = 'OUTB',
         .category      = category::miscellaneous,
         .internal_name = "OutofBreath",
      },
      topic_subtype{
         .signature     = 'GRNT',
         .category      = category::miscellaneous,
         .internal_name = "CombatGrunt",
      },
      topic_subtype{
         .signature     = 'LWBS',
         .category      = category::miscellaneous,
         .internal_name = "LeaveWaterBreath",
      },
   };
}

#include "./topic_subtype.inl"