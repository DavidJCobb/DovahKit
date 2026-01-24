#pragma once
#include "../bare_form_id_t.h"

namespace dovah::hardcoded_form_ids {
   static constexpr bare_form_id_t DoorMarker             = 0x001;
   static constexpr bare_form_id_t TravelMarker           = 0x002;
   static constexpr bare_form_id_t NorthMarker            = 0x003;
   static constexpr bare_form_id_t PrisonMarker           = 0x004;
   static constexpr bare_form_id_t DivineMarker           = 0x005;
   static constexpr bare_form_id_t TempleMarker           = 0x006;
   static constexpr bare_form_id_t Player                 = 0x007; // NPC_
   static constexpr bare_form_id_t BobbyPin               = 0x00A;
   // Unused after Oblivion: [MISC:00C]RepairHammer
   static constexpr bare_form_id_t LootBag                = 0x00E;
   static constexpr bare_form_id_t Caps001                = 0x00F;
   static constexpr bare_form_id_t MapMarker              = 0x010;
   // Unused after Oblivion: [CONT:011]StolenGoods
   static constexpr bare_form_id_t HorseMarker            = 0x012;
   static constexpr bare_form_id_t CreatureFaction        = 0x013;
   static constexpr bare_form_id_t PlayerRef              = 0x014; // ACHR
   static constexpr bare_form_id_t MultiBoundMarker       = 0x015;
   // Seen in Fallout 4: [CPTH:016]DefaultHitRight01O
   static constexpr bare_form_id_t PlaneMarker            = 0x017;
   static constexpr bare_form_id_t DefaultWater           = 0x018;
   static constexpr bare_form_id_t DefaultRace            = 0x019;
   static constexpr bare_form_id_t eyeReanimate           = 0x01A;
   static constexpr bare_form_id_t DefaultAshPile1        = 0x01B;
   static constexpr bare_form_id_t PlayerBodyPartData     = 0x01C;
   static constexpr bare_form_id_t DefaultBodyPartData    = 0x01D;
   static constexpr bare_form_id_t NoZoneZone             = 0x01E;
   static constexpr bare_form_id_t RoomMarker             = 0x01F;
   static constexpr bare_form_id_t PortalMarker           = 0x020;
   static constexpr bare_form_id_t CollisionMarker        = 0x021;
   static constexpr bare_form_id_t DefaultAshPile2        = 0x022;
   // Seen in Fallout 4: [STAT:023]LODClipVolume
   // Unused after Fallout: New Vegas: [STAT:024]AudioBuoyMarker
   static constexpr bare_form_id_t NavmeshGenCell         = 0x025;
   // Seen in Fallout 4: [NOCM:026]
   // Seen in Fallout 4: [OVIS:027]
   static constexpr bare_form_id_t NullTextureSet         = 0x028;
   static constexpr bare_form_id_t AdultMaleVoice1        = 0x02D;
   static constexpr bare_form_id_t AdultFemaleVoice1      = 0x02E;
   static constexpr bare_form_id_t COCMarkerHeading       = 0x032;
   // Seen in Fallout 4: [STAT:033]RadiationMarker
   static constexpr bare_form_id_t XMarkerHeading         = 0x034;
   static constexpr bare_form_id_t GameYear               = 0x035;
   static constexpr bare_form_id_t GameMonth              = 0x036;
   static constexpr bare_form_id_t GameDay                = 0x037;
   static constexpr bare_form_id_t GameHour               = 0x038;
   static constexpr bare_form_id_t GameDaysPassed         = 0x039;
   static constexpr bare_form_id_t TimeScale              = 0x03A;
   static constexpr bare_form_id_t XMarker                = 0x03B;
   static constexpr bare_form_id_t DefaultWorld           = 0x03C;
   static constexpr bare_form_id_t DefaultCombatStyle     = 0x03D;
   //
   // Form IDs in the range [03D, 051] were used in Oblivion for hardcoded SKIL forms, a form type that 
   // was removed in later games. Notably, form ID 0x3D was reused in later games for a different form.
   //
   static constexpr bare_form_id_t Root                   = 0x05B; // Story Manager Branch Node
   static constexpr bare_form_id_t CellWaterCurrentMarker = 0x061;
   static constexpr bare_form_id_t WaterCurrentMarker     = 0x062;
   static constexpr bare_form_id_t PlayCredits            = 0x063;
   static constexpr bare_form_id_t FurnitureMarker01      = 0x064;
   static constexpr bare_form_id_t FurnitureMarker02      = 0x065;
   static constexpr bare_form_id_t FurnitureMarker03      = 0x066;
   static constexpr bare_form_id_t FurnitureMarker04      = 0x067;
   static constexpr bare_form_id_t FurnitureMarker05      = 0x068;
   //
   // Forms in the range [069, 08B] were formerly STATs in Fallout 3 and Fallout: New Vegas, defining 
   // FurnitureMarker06 through FurnitureMarker40. These were removed from the engine as of Skyrim.
   // 
   // Added in Fallout 4: [STAT:0A0]AnimInteractionMarker
   // Added in Fallout 4: [STAT:0C2]SplineEndpointMarker
   static constexpr bare_form_id_t WaterCurrentZoneMarker = 0x0C4;
   // Seen in Oblivion: [DIAL:0AA]ADMIRE_HATE
   // Seen in Oblivion: [DIAL:0AB]ADMIRE_LOVE
   // Seen in Oblivion: [DIAL:0AC]ADMIRE_LIKE
   // Seen in Oblivion: [DIAL:0AD]ADMIRE_DISLIKE
   // Seen in Oblivion: [DIAL:0AE]COERCE_HATE
   // Seen in Oblivion: [DIAL:0AF]COERCE_LOVE
   // Seen in Oblivion: [DIAL:0B0]COERCE_LIKE
   // Seen in Oblivion: [DIAL:0B1]COERCE_DISLIKE
   // Seen in Oblivion: [DIAL:0B2]BOAST_HATE
   // Seen in Oblivion: [DIAL:0B3]BOAST_LOVE
   // Seen in Oblivion: [DIAL:0B4]BOAST_LIKE
   // Seen in Oblivion: [DIAL:0B5]BOAST_DISLIKE
   // Seen in Oblivion: [DIAL:0B6]JOKE_HATE
   // Seen in Oblivion: [DIAL:0B7]JOKE_LOVE
   // Seen in Oblivion: [DIAL:0B8]JOKE_LIKE
   // Seen in Oblivion: [DIAL:0B9]JOKE_DISLIKE
   // Seen in Oblivion: [DIAL:0BA]BRIBE
   // Seen in Oblivion: [DIAL:0BB]PERSUASION_ENTER
   // Seen in Oblivion: [DIAL:0BC]PERSUASION_EXIT
   // Seen in Oblivion, and the form ID is referenced by the Skyrim Creation Kit's code, but it doesn't actually exist anymore: [DIAL:0C8]GREETING
   // Seen in Oblivion: [DIAL:0D2]HELLO
   // Seen in Oblivion: [DIAL:0D3]ANY
   // Seen in Oblivion: [DIAL:0D4]GOODBYE
   // Seen in Oblivion: [DIAL:0D5]Idle Chatter
   // Seen in Oblivion: [DIAL:0D6]SPELL HELP
   // Seen in Oblivion: [DIAL:0D7]INFO GENERAL
   // Seen in Oblivion: [DIAL:0DC]Attack
   // Seen in Oblivion: [DIAL:0DD]Hit
   // Seen in Oblivion: [DIAL:0DE]Flee
   // Seen in Oblivion: [DIAL:0DF]Steal
   // Seen in Oblivion: [DIAL:0E0]Trespass
   // Seen in Oblivion: [DIAL:0E1]Yield
   // Seen in Oblivion: [DIAL:0E2]AcceptYield
   // Seen in Oblivion: [DIAL:0E3]Pickpocket
   // Seen in Oblivion: [DIAL:0E4]Assault
   // Seen in Oblivion: [DIAL:0E5]Murder
   // Seen in Oblivion: [DIAL:0E6]PowerAttack
   // Seen in Oblivion: [DIAL:0E7]AssaultNoCrime
   // Seen in Oblivion: [DIAL:0E8]MurderNoCrime
   // Seen in Oblivion: [DIAL:0E9]PickpocketNoCrime
   // Seen in Oblivion: [DIAL:0EA]StealNoCrime
   // Seen in Oblivion: [DIAL:0EB]TrespassNoCrime
   // Seen in Oblivion: [DIAL:0F0]AdmireSuccess
   // Seen in Oblivion: [DIAL:0F1]AdmireFail
   // Seen in Oblivion: [DIAL:0F2]AdmireNeutral
   // Seen in Oblivion: [DIAL:0F3]TauntSuccess
   // Seen in Oblivion: [DIAL:0F4]TauntFail
   // Seen in Oblivion: [DIAL:0F5]TauntNeutral
   // Seen in Oblivion: [DIAL:0F6]BoastSuccess
   // Seen in Oblivion: [DIAL:0F7]BoastFail
   // Seen in Oblivion: [DIAL:0F8]BoastNeutral
   // Seen in Oblivion: [DIAL:0F9]JokeSuccess
   // Seen in Oblivion: [DIAL:0FA]JokeFail
   // Seen in Oblivion: [DIAL:0FB]JokeNeutral
   // Seen in Oblivion: [DIAL:0FC]BribeSuccess
   // Seen in Oblivion: [DIAL:0FD]BribeFail
   // Seen in Oblivion: [DIAL:0FE]DemandSuccess
   // Seen in Oblivion: [DIAL:0FF]DemandFail
   // Seen in Oblivion: [DIAL:100]DemandNoMoney
   // Seen in Oblivion: [DIAL:101]DemandNoMoreCircle
   // Seen in Oblivion: [DIAL:102]BribeNoMoreCircle
   // Seen in Oblivion: [DIAL:103]BribeNoMoney
   // Seen in Oblivion: [DIAL:104]Noticed
   // Seen in Oblivion: [DIAL:105]Seen
   // Seen in Oblivion: [DIAL:106]Unseen
   // Seen in Oblivion: [DIAL:107]Lost
   // Seen in Oblivion: [DIAL:10E]ServiceRefusal
   // Seen in Oblivion: [DIAL:10F]BarterStart
   // Seen in Oblivion: [DIAL:110]BarterFail
   // Seen in Oblivion: [DIAL:111]Repair
   // Seen in Oblivion: [DIAL:112]Travel
   // Seen in Oblivion: [DIAL:113]Training
   // Seen in Oblivion: [DIAL:114]BarterBuyItem
   // Seen in Oblivion: [DIAL:115]BarterSellItem
   // Seen in Oblivion: [DIAL:116]BarterExit
   // Seen in Oblivion: [DIAL:117]BarterStolen
   // Seen in Oblivion: [DIAL:118]InfoRefusal
   // Seen in Oblivion: [DIAL:119]Idle
   // Seen in Oblivion: [DIAL:11A]ObserveCombat
   // Seen in Oblivion: [DIAL:11B]Corpse
   // Seen in Oblivion: [DIAL:11C]TimeToGo
   // Seen in Oblivion: [DIAL:11D]RepairExit
   // Seen in Oblivion: [DIAL:120]Recharge
   // Seen in Oblivion: [DIAL:121]RechargeExit
   // Seen in Oblivion: [DIAL:124]TrainingExit
   static constexpr bare_form_id_t LifeDetected           = 0x146; // EFSH // CK-only?
   static constexpr bare_form_id_t ScriptEffect           = 0x14A; // MGEF // CK-only?
   // Unused after Fallout: New Vegas: [MGEF:14B]FireDamage
   static constexpr bare_form_id_t WardConcSelf0          = 0x14C; // MGEF // CK-only?
   // Unused after Fallout: New Vegas: [MGEF:14D]ShockDamage
   // Unused after Fallout: New Vegas: [MGEF:14E]RestoreHealth
   // Used in FO3, FNV, and FO4:       [MGEF:14F]UMON
   static constexpr bare_form_id_t DefaultWeather         = 0x15E;
   static constexpr bare_form_id_t DefaultClimate         = 0x15F;
   static constexpr bare_form_id_t DefaultImageSpaceInterior = 0x160;
   static constexpr bare_form_id_t DefaultImageSpaceExterior = 0x161;
   static constexpr bare_form_id_t GetHit                    = 0x162; // IMAD
   static constexpr bare_form_id_t HelpManualPC              = 0x163; // FLST
   static constexpr bare_form_id_t ImageSpaceConcussion      = 0x164; // IMAD
   static constexpr bare_form_id_t HelpManualXbox            = 0x165; // FLST
   static constexpr bare_form_id_t ExplosionInFace           = 0x166; // IMAD
   static constexpr bare_form_id_t DefaultImageSpace         = 0x167;
   // 168: scrapped MESG?
   // 169: scrapped MESG?
   // 16A: scrapped MESG?
   // 16B: scrapped MESG?
   // 16C: scrapped MESG?
   static constexpr bare_form_id_t HelpPipBoyItems = 0x16D; // MESG
   static constexpr bare_form_id_t HelpPipBoyRepair = 0x16E; // MESG
   // 16F: scrapped MESG?
   // 170: scrapped MESG?
   // 171: scrapped MESG?
   // 172: scrapped MESG?
   // 173: scrapped MESG?
   // 174: scrapped MESG?
   // 175: scrapped MESG?
   static constexpr bare_form_id_t HelpCharGenTagSkills = 0x176; // MESG
   static constexpr bare_form_id_t HelpCharGenRace = 0x177; // MESG
   static constexpr bare_form_id_t HelpLeveling = 0x178; // MESG
   static constexpr bare_form_id_t HelpDialogue = 0x179; // MESG
   // 17A: scrapped MESG?
   static constexpr bare_form_id_t HelpHacking = 0x17B; // MESG
   static constexpr bare_form_id_t HelpLockpickingPC = 0x17C; // MESG
   static constexpr bare_form_id_t HelpVATSPC = 0x17D; // MESG
   static constexpr bare_form_id_t HelpContainer = 0x17E; // MESG
   static constexpr bare_form_id_t HelpBarter = 0x17F; // MESG
   static constexpr bare_form_id_t HelpTerminal = 0x180; // MESG
   static constexpr bare_form_id_t HelpPipBoyStats = 0x181; // MESG
   static constexpr bare_form_id_t HelpPipBoyData = 0x182; // MESG
   static constexpr bare_form_id_t HelpVATSXBox = 0x183; // MESG
   static constexpr bare_form_id_t HelpLockpickingXBox = 0x184; // MESG
   #pragma region Scrapped slots [185, 190]: formerly help messages from Fallout: New Vegas
      // [MESG:185]HelpItemMod
      // [MESG:186]HelpCaravanBetting
      // [MESG:187]HelpCaravanDeckBuilding
      // [MESG:188]HelpCaravanStartingCaravans
      // [MESG:187]HelpCaravanContractWar
      // [MESG:18A]HelpWeapons
      // [MESG:18B]HelpApparel
      // [MESG:18C]HelpAmmo
      // [MESG:18D]HelpAmmoXbox
      // [MESG:18E]HelpRecipe
      // [MESG:18F]HelpReputation
      // [MESG:190]HelpHardcoreNeeds
   #pragma endregion
   // Unused after Oblivion: [SLGM:191]WelkyndStone
   // Unused after Oblivion: [SLGM:192]BlackSoulGem
   // Unused after Oblivion: [SLGM:193]AzuraStone
   // Unused after Oblivion: [SLGM:194]VarlaStone
   static constexpr bare_form_id_t HairColorListDoNotUse = 0x1F3; // FLST // CK-only?
   static constexpr bare_form_id_t Unarmed = 0x1F4; // WEAP
   static constexpr bare_form_id_t DefaultWaterExplosion = 0x1F5; // EXPL // true editor ID is "Default Water Explosion", with spaces
   static constexpr bare_form_id_t GasTrapDummy = 0x1F6; // WEAP // true editor ID is "GasTrap Dummy"
   // Seen in Fallout 4: [STAT:1F7]LightBox
   // Unused after Oblivion: [SNDR:212]FootSoundDirt
   // Unused after Oblivion: [SNDR:213]FootSoundGrass
   // Unused after Oblivion: [SNDR:214]FootSoundStone
   // Unused after Oblivion: [SNDR:215]FootSoundWater
   // Unused after Oblivion: [SNDR:216]FootSoundWood
   // Unused after Oblivion: [SNDR:217]FootSoundHeavyArmor
   // Unused after Oblivion: [SNDR:218]FootSoundLightArmor
   // Unused after Oblivion: [SNDR:219]FootSoundEarthLand
   // Unused after Oblivion: [SNDR:21A]FootSoundGrassLand
   // Unused after Oblivion: [SNDR:21B]FootSoundMetalLand
   // Unused after Oblivion: [SNDR:21C]FootSoundStoneLand
   // Unused after Oblivion: [SNDR:21D]FootSoundWaterLand
   // Unused after Oblivion: [SNDR:21E]FootSoundWoodLand
   // Unused after Oblivion: [SNDR:21F]FSTSnow
   // Unused after Oblivion: [SNDR:220]FSTSnowLand
   // Unused after Oblivion: [SNDR:221]FSTEarthSneak
   // Unused after Oblivion: [SNDR:222]FSTGrassSneak
   // Unused after Oblivion: [SNDR:223]FSTMetalSneak
   // Unused after Oblivion: [SNDR:224]FSTSnowSneak
   // Unused after Oblivion: [SNDR:225]FSTStoneSneak
   // Unused after Oblivion: [SNDR:226]FSTWaterSneak
   // Unused after Oblivion: [SNDR:227]FSTWoodSneak
   // Unused after Oblivion: [SNDR:228]FSTArmorLightSneak
   // Unused after Oblivion: [SNDR:229]FSTArmorHeavySneak
   // Unused after Oblivion: [SNDR:22B]FSTMetal
   //
   // Form IDs in the range [258, 265] were formerly used by MICN forms in Fallout 3 and Fallout: New Vegas.
   //
   static constexpr bare_form_id_t DefaultImpactDataSet = 0x276;
   static constexpr bare_form_id_t PapyrusPersistenceForm    = 0x28A;
   static constexpr bare_form_id_t CommandingActorPersistenceForm = 0x294;

   // Form IDs in the range [0x3E8, 0x3F6] are actor values (attributes).
   // Form IDs in the range [0x44C, 0x45D] are actor values (skills).
   // Form IDs in the range [0x4B0, 0x4B5] are actor values (AI).
   // Form IDs in the range [0x5CE, 0x64A] are actor values (stats).
}