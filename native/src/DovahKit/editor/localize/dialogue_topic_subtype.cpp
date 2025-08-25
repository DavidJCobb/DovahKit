#include "./dialogue_topic_subtype.h"
#include <QCoreApplication>
#include "dovah/data/dialogue/topic_subtype.h"

namespace editor::localize {
   extern QString dialogue_topic_subtype(const dovah::dialogue::topic_subtype& subtype) {
      switch (subtype.signature) {
         case 'CUST':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Custom");
         case 'PFGT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ForceGreet");
         case 'RUMO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Rumors");
         case 'FVDL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Custom");
         case 'INTI':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Intimidate");
         case 'FLAT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Flatter");
         case 'BRIB':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Bribe");
         case 'ASKG':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AskGift");
         case 'GIFF':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Gift");
         case 'ASKF':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AskFavor");
         case 'FAVO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Favor");
         case 'SHRE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ShowRelationships");
         case 'FOLL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Follow");
         case 'FRJT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Reject");
         case 'SCEN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Custom");
         case 'SHOW':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Show");
         case 'AGRE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Agree");
         case 'REFU':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Refuse");
         case 'FEXT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ExitFavorState");
         case 'MREF':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "MoralRefusal");
         case 'FMLX':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountLand");
         case 'FMXL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountCancelLand");
         case 'FMAT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountAcceptTarget");
         case 'FMRT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountRejectTarget");
         case 'FMNT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountNoTarget");
         case 'FMDR':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "FlyingMountDestinationReached");
         case 'ATCK':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Attack");
         case 'POAT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PowerAttack");
         case 'BASH':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Bash");
         case 'HIT_':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Hit");
         case 'FLEE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Flee");
         case 'BLED':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "BleedOut");
         case 'AVTH':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AvoidThreat");
         case 'DETH':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Death");
         case 'GRST':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "GroupStrategy");
         case 'BLOC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Block");
         case 'TAUT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Taunt");
         case 'ALKL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AllyKilled");
         case 'STEA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Steal");
         case 'YIEL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Yield");
         case 'ACYI':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AcceptYield");
         case 'PICC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PickpocketCombat");
         case 'ASSA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Assault");
         case 'MURD':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Murder");
         case 'ASNC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AssaultNC");
         case 'MUNC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "MurderNC");
         case 'PICN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PickpocketNC");
         case 'STFN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "StealFromNC");
         case 'TRAN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "TrespassAgainstNC");
         case 'TRES':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Trespass");
         case 'WTCR':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "WereTransformCrime");
         case 'VPSS':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "VoicePowerStartShort");
         case 'VPSL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "VoicePowerStartLong");
         case 'VPES':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "VoicePowerEndShort");
         case 'VPEL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "VoicePowerEndLong");
         case 'ALIL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AlertIdle");
         case 'LOIL':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "LostIdle");
         case 'NOTA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "NormalToAlert");
         case 'ALTC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AlertToCombat");
         case 'NOTC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "NormalToCombat");
         case 'ALTN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "AlertToNormal");
         case 'COTN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "CombatToNormal");
         case 'COLO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "CombatToLost");
         case 'LOTN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "LostToNormal");
         case 'LOTC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "LostToCombat");
         case 'DFDA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "DetectFriendDie");
         case 'SERU':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ServiceRefusal");
         case 'REPA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Repair");
         case 'TRAV':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Travel");
         case 'TRAI':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Training");
         case 'BAEX':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "BarterExit");
         case 'REEX':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "RepairExit");
         case 'RECH':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Recharge");
         case 'RCEX':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "RechargeExit");
         case 'TREX':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "TrainingExit");
         case 'OBCO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ObserveCombat");
         case 'NOTI':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "NoticeCorpse");
         case 'TITG':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "TimeToGo");
         case 'GBYE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Goodbye");
         case 'HELO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Hello");
         case 'SWMW':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "SwingMeleeWeapon");
         case 'FIWE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ShootBow");
         case 'ZKEY':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ZKeyObject");
         case 'JUMP':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Jump");
         case 'KNOO':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "KnockOverObject");
         case 'DEOB':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "DestroyObject");
         case 'STOF':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "StandonFurniture");
         case 'LOOB':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "LockedObject");
         case 'PICT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PickpocketTopic");
         case 'PURS':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PursueIdleTopic");
         case 'IDAT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "SharedInfo");
         case 'PCPS':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PlayerCastProjectileSpell");
         case 'PCSS':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PlayerCastSelfSpell");
         case 'PCSH':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PlayerShout");
         case 'IDLE':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "Idle");
         case 'BREA':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "EnterSprintBreath");
         case 'ENBZ':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "EnterBowZoomBreath");
         case 'EXBZ':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ExitBowZoomBreath");
         case 'ACAC':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "ActorCollidewithActor");
         case 'PIRN':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "PlayerinIronSights");
         case 'OUTB':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "OutofBreath");
         case 'GRNT':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "CombatGrunt");
         case 'LWBS':
            return QCoreApplication::translate("dovah::dialogue::topic_subtype", "LeaveWaterBreath");
      }
      return "";
   }
}