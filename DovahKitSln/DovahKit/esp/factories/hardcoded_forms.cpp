#include "hardcoded_forms.h"
#include "../LoadOrder.h"

#include "../../forms/loaded/Activator.h"
#include "../../forms/loaded/ActorBase.h"
#include "../../forms/loaded/Container.h"
#include "../../forms/loaded/Form.h"
#include "../../forms/loaded/FormList.h"
#include "../../forms/loaded/Voicetype.h"

void _addHardcodedFormsToLoadOrder() {
   auto& lo = LoadOrder::get();
      {  // [EXPL:1F5]"Default Water Explosion"
      auto stub = new FormStub();
      stub->formID   = 0x1F5;
      stub->formType = signatureToFormType('EXPL');
      stub->editorID = "Default Water Explosion";
      // Full Name: "Water Explosion"
      lo._acceptHardcodedForm(stub);
   }
   {  // [WEAP:1F6]"GasTrap Dummy"
      auto stub = new FormStub();
      stub->formID   = 0x1F6;
      stub->formType = signatureToFormType('WEAP');
      stub->editorID = "GasTrap Dummy";
      // Full Name: "GasTrap Dummy"
      lo._acceptHardcodedForm(stub);
   }
   {  // [EYES:01A]"eyeReanimate"
      auto stub = new FormStub();
      stub->formID   = 0x01A;
      stub->formType = signatureToFormType('EYES');
      stub->editorID = "eyeReanimate";
      // Full Name: "Reanimate Eyes"
      lo._acceptHardcodedForm(stub);
   }
   {  // [ACTI:01B]"DefaultAshPile1"
      auto stub = new FormStub();
      stub->formID   = 0x01B;
      stub->formType = signatureToFormType('ACTI');
      stub->editorID = "DefaultAshPile1";
      // Full Name: "Ash Pile 1"
      lo._acceptHardcodedForm(stub);
   }
   {  // [ACTI:022]"DefaultAshPile2"
      auto stub = new FormStub();
      stub->formID   = 0x022;
      stub->formType = signatureToFormType('ACTI');
      stub->editorID = "DefaultAshPile2";
      // Full Name: "Ash Pile 2"
      lo._acceptHardcodedForm(stub);
   }
   {  // [VTYP:02D]"AdultMaleVoice1"
      auto stub = new FormStub();
      stub->formID   = 0x02D;
      stub->formType = signatureToFormType('VTYP');
      stub->editorID = "AdultMaleVoice1";
      lo._acceptHardcodedForm(stub);
      //
      auto form = new LoadedForms::Voicetype;
      form->stub = stub;
      stub->form = form;
   }
   {  // [VTYP:02E]"AdultFemaleVoice1"
      auto stub = new FormStub();
      stub->formID   = 0x02E;
      stub->formType = signatureToFormType('VTYP');
      stub->editorID = "AdultFemaleVoice1";
      lo._acceptHardcodedForm(stub);
      //
      auto form = new LoadedForms::Voicetype;
      form->stub = stub;
      stub->form = form;
   }
   {  // [WATR:018]"DefaultWater"
      auto stub = new FormStub();
      stub->formID   = 0x018;
      stub->formType = signatureToFormType('WATR');
      stub->editorID = "DefaultWater";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:012]"HorseMarker"
      auto stub = new FormStub();
      stub->formID   = 0x012;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "HorseMarker";
      // Model File "Name: Marker_Horse.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [MISC:00A]"BobbyPin"
      auto stub = new FormStub();
      stub->formID   = 0x00A;
      stub->formType = signatureToFormType('MISC');
      stub->editorID = "BobbyPin";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:001]"DoorMarker"
      auto stub = new FormStub();
      stub->formID   = 0x001;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "DoorMarker";
      // Flags: 0x800000
      // Model File Name: "MarkerTeleport.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [WRLD:03C]"DefaultWorld"
      auto stub = new FormStub();
      stub->formID   = 0x03C;
      stub->formType = signatureToFormType('WRLD');
      stub->editorID = "DefaultWorld";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:015]"MultiBoundMarker"
      auto stub = new FormStub();
      stub->formID   = 0x015;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "MultiBoundMarker";
      // Flags: 0x800000
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:017]"PlaneMarker"
      auto stub = new FormStub();
      stub->formID   = 0x017;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "PlaneMarker";
      // Flags: 0x800000
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:01F]"RoomMarker"
      auto stub = new FormStub();
      stub->formID   = 0x01F;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "RoomMarker";
      // Flags: 0x800000
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:020]"PortalMarker"
      auto stub = new FormStub();
      stub->formID   = 0x020;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "PortalMarker";
      // Flags: 0x800000
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:021]"CollisionMarker"
      auto stub = new FormStub();
      stub->formID   = 0x021;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "CollisionMarker";
      // Flags: 0x800000
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:03B]"XMarker"
      auto stub = new FormStub();
      stub->formID   = 0x03B;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "XMarker";
      // Flags: 0x800000
      // Model File Name: "MarkerX.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:034]"XMarkerHeading"
      auto stub = new FormStub();
      stub->formID   = 0x034;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "XMarkerHeading";
      // Flags: 0x800000
      // Model File Name: "MarkerXHeading.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:032]"COCMarkerHeading"
      auto stub = new FormStub();
      stub->formID   = 0x032;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "COCMarkerHeading";
      // Flags: 0x800000
      // Model File Name: "MarkerCOCHeading.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:010]"MapMarker"
      auto stub = new FormStub();
      stub->formID   = 0x010;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "MapMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_Map.NIF"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:002]"TravelMarker"
      auto stub = new FormStub();
      stub->formID   = 0x002;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "TravelMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_Travel.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:003]"NorthMarker"
      auto stub = new FormStub();
      stub->formID   = 0x003;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "NorthMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_North.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [DOOR:004]"PrisonMarker"
      auto stub = new FormStub();
      stub->formID   = 0x004;
      stub->formType = signatureToFormType('DOOR');
      stub->editorID = "PrisonMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_Prison.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:004]"TempleMarker"
      auto stub = new FormStub();
      stub->formID   = 0x004;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "TempleMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_Temple.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:005]"DivineMarker"
      auto stub = new FormStub();
      stub->formID   = 0x005;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "DivineMarker";
      // Flags: 0x800000
      // Model File Name: "Marker_Divine.nif"
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:062]"WaterCurrentMarker"
      auto stub = new FormStub();
      stub->formID   = 0x062;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "WaterCurrentMarker";
      // Flags: 0x800000
      // Model File Name: ""
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:061]"CellWaterCurrentMarker"
      auto stub = new FormStub();
      stub->formID   = 0x061;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "CellWaterCurrentMarker";
      // Flags: 0x800000
      // Model File Name: ""
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:0C4]"WaterCurrentZoneMarker"
      auto stub = new FormStub();
      stub->formID   = 0x0C4;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "WaterCurrentZoneMarker";
      // Flags: 0x800000
      // Model File Name: ""
      lo._acceptHardcodedForm(stub);
   }
   {  // [MISC:00F]"Caps001"
      auto stub = new FormStub();
      stub->formID   = 0x00F;
      stub->formType = signatureToFormType('MISC');
      stub->editorID = "Caps001";
      lo._acceptHardcodedForm(stub);
   }
   {  // [CONT:00E]"LootBag"
      auto stub = new FormStub();
      stub->formID   = 0x00E;
      stub->formType = signatureToFormType('CONT');
      stub->editorID = "LootBag";
      // Model File Name: "Clutter\\Sack01.NIF"
      lo._acceptHardcodedForm(stub);
   }
   {  // [WEAP:00E]"Unarmed"
      auto stub = new FormStub();
      stub->formID   = 0x00E;
      stub->formType = signatureToFormType('WEAP');
      stub->editorID = "Unarmed";
      // DATA - Animation Type: HandToHandMelee (0)
      lo._acceptHardcodedForm(stub);
   }
   {  // [NONE:28A]"PapyrusPersistenceForm"
      auto stub = new FormStub();
      stub->formID   = 0x28A;
      stub->formType = signatureToFormType('NONE');
      stub->editorID = "PapyrusPersistenceForm";
      lo._acceptHardcodedForm(stub);
   }
   {  // [NONE:294]"CommandingActorPersistenceForm"
      auto stub = new FormStub();
      stub->formID   = 0x294;
      stub->formType = signatureToFormType('NONE');
      stub->editorID = "CommandingActorPersistenceForm";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:064]"FurnitureMarker01"
      auto stub = new FormStub();
      stub->formID   = 0x064;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "FurnitureMarker01";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:065]"FurnitureMarker02"
      auto stub = new FormStub();
      stub->formID   = 0x065;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "FurnitureMarker02";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:066]"FurnitureMarker03"
      auto stub = new FormStub();
      stub->formID   = 0x066;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "FurnitureMarker03";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:067]"FurnitureMarker04"
      auto stub = new FormStub();
      stub->formID   = 0x067;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "FurnitureMarker04";
      lo._acceptHardcodedForm(stub);
   }
   {  // [STAT:068]"FurnitureMarker05"
      auto stub = new FormStub();
      stub->formID   = 0x068;
      stub->formType = signatureToFormType('STAT');
      stub->editorID = "FurnitureMarker05";
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:035]"GameYear"
      auto stub = new FormStub();
      stub->formID   = 0x035;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "GameYear";
      // Value: 77
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:036]"GameMonth"
      auto stub = new FormStub();
      stub->formID   = 0x036;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "GameMonth";
      // Value: 7
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:037]"GameDay"
      auto stub = new FormStub();
      stub->formID   = 0x037;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "GameDay";
      // Value: 17
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:038]"GameHour"
      auto stub = new FormStub();
      stub->formID   = 0x038;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "GameHour";
      // Value: 12
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:039]"GameDaysPassed"
      auto stub = new FormStub();
      stub->formID   = 0x039;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "GameDaysPassed";
      // Value: 1
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:03A]"TimeScale"
      auto stub = new FormStub();
      stub->formID   = 0x03A;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "TimeScale";
      // Value: 30
      lo._acceptHardcodedForm(stub);
   }
   {  // [NPC_:007]"Player"
      auto stub = new FormStub();
      stub->formID   = 0x007;
      stub->formType = signatureToFormType('NPC_');
      stub->editorID = "Player";
      lo._acceptHardcodedForm(stub);
   }
   {  // [WTHR:15E]"DefaultWeather"
      auto stub = new FormStub();
      stub->formID   = 0x15E;
      stub->formType = signatureToFormType('WTHR');
      stub->editorID = "DefaultWeather";
      lo._acceptHardcodedForm(stub);
   }
   {  // [WTHR:15F]"DefaultClimate"
      auto stub = new FormStub();
      stub->formID   = 0x15F;
      stub->formType = signatureToFormType('WTHR');
      stub->editorID = "DefaultClimate";
      lo._acceptHardcodedForm(stub);
   }
   {  // [FLST:163]"HelpManualPC"
      auto stub = new FormStub();
      stub->formID   = 0x163;
      stub->formType = signatureToFormType('FLST');
      stub->editorID = "HelpManualPC";
      lo._acceptHardcodedForm(stub);
      //
      auto form = new LoadedForms::FormList;
      form->stub = stub;
      stub->form = form;
   }
   {  // [FLST:165]"HelpManualXBox"
      auto stub = new FormStub();
      stub->formID   = 0x165;
      stub->formType = signatureToFormType('FLST');
      stub->editorID = "HelpManualXBox";
      lo._acceptHardcodedForm(stub);
      //
      auto form = new LoadedForms::FormList;
      form->stub = stub;
      stub->form = form;
   }
   {  // [MESG:16B]"HelpPipBoyItems"
      auto stub = new FormStub();
      stub->formID   = 0x16B;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpPipBoyItems";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:16C]"HelpPipBoyRepair"
      auto stub = new FormStub();
      stub->formID   = 0x16C;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpPipBoyRepair";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:171]"HelpChargenTagSkills"
      auto stub = new FormStub();
      stub->formID   = 0x171;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpChargenTagSkills";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:172]"HelpChargenRace"
      auto stub = new FormStub();
      stub->formID   = 0x172;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpChargenRace";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:173]"HelpLeveling"
      auto stub = new FormStub();
      stub->formID   = 0x173;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpLeveling";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:174]"HelpDialogue"
      auto stub = new FormStub();
      stub->formID   = 0x174;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpDialogue";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:176]"HelpHacking"
      auto stub = new FormStub();
      stub->formID   = 0x176;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpHacking";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:177]"HelpLockpickingPC"
      auto stub = new FormStub();
      stub->formID   = 0x177;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpLockpickingPC";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:178]"HelpVATSPC"
      auto stub = new FormStub();
      stub->formID   = 0x178;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpVATSPC";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:179]"HelpContainer"
      auto stub = new FormStub();
      stub->formID   = 0x179;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpContainer";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17A]"HelpBarter"
      auto stub = new FormStub();
      stub->formID   = 0x17A;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpBarter";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17B]"HelpTerminal"
      auto stub = new FormStub();
      stub->formID   = 0x17B;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpTerminal";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17C]"HelpPipBoyStats"
      auto stub = new FormStub();
      stub->formID   = 0x17C;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpPipBoyStats";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17D]"HelpPipBoyData"
      auto stub = new FormStub();
      stub->formID   = 0x17D;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpPipBoyData";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17E]"HelpVATSXBox"
      auto stub = new FormStub();
      stub->formID   = 0x17E;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpVATSXBox";
      lo._acceptHardcodedForm(stub);
   }
   {  // [MESG:17F]"HelpLockpickingXBox"
      auto stub = new FormStub();
      stub->formID   = 0x17F;
      stub->formType = signatureToFormType('MESG');
      stub->editorID = "HelpLockpickingXBox";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMGS:167]"DefaultImageSpace"
      auto stub = new FormStub();
      stub->formID   = 0x167;
      stub->formType = signatureToFormType('IMGS');
      stub->editorID = "DefaultImageSpace";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMGS:160]"DefaultImageSpaceInterior"
      auto stub = new FormStub();
      stub->formID   = 0x160;
      stub->formType = signatureToFormType('IMGS');
      stub->editorID = "DefaultImageSpaceInterior";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMGS:161]"DefaultImageSpaceExterior"
      auto stub = new FormStub();
      stub->formID   = 0x161;
      stub->formType = signatureToFormType('IMGS');
      stub->editorID = "DefaultImageSpaceExterior";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMAD:162]"GetHit"
      auto stub = new FormStub();
      stub->formID   = 0x162;
      stub->formType = signatureToFormType('IMAD');
      stub->editorID = "GetHit";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMAD:166]"ExplosionInFace"
      auto stub = new FormStub();
      stub->formID   = 0x166;
      stub->formType = signatureToFormType('IMAD');
      stub->editorID = "ExplosionInFace";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IMAD:164]"ImageSpaceConcussion"
      auto stub = new FormStub();
      stub->formID   = 0x164;
      stub->formType = signatureToFormType('IMAD');
      stub->editorID = "ImageSpaceConcussion";
      lo._acceptHardcodedForm(stub);
   }
   {  // [IPDS:276]"DefaultImpactDataSet"
      auto stub = new FormStub();
      stub->formID   = 0x276;
      stub->formType = signatureToFormType('IPDS');
      stub->editorID = "DefaultImpactDataSet";
      lo._acceptHardcodedForm(stub);
   }
   {  // [BPTD:01D]"DefaultBodyPartData"
      auto stub = new FormStub();
      stub->formID   = 0x01D;
      stub->formType = signatureToFormType('BPTD');
      stub->editorID = "DefaultBodyPartData";
      lo._acceptHardcodedForm(stub);
   }
   {  // [BPTD:01C]"PlayerBodyPartData"
      auto stub = new FormStub();
      stub->formID   = 0x01C;
      stub->formType = signatureToFormType('BPTD');
      stub->editorID = "PlayerBodyPartData";
      lo._acceptHardcodedForm(stub);
   }
   {  // [TXST:028]"NullTextureSet"
      auto stub = new FormStub();
      stub->formID   = 0x028;
      stub->formType = signatureToFormType('TXST');
      stub->editorID = "NullTextureSet";
      lo._acceptHardcodedForm(stub);
   }
   {  // [ECZN:01E]"NoZoneZone"
      auto stub = new FormStub();
      stub->formID   = 0x01E;
      stub->formType = signatureToFormType('ECZN');
      stub->editorID = "NoZoneZone";
      lo._acceptHardcodedForm(stub);
   }
   {  // [GLOB:063]"PlayCredits"
      auto stub = new FormStub();
      stub->formID   = 0x063;
      stub->formType = signatureToFormType('GLOB');
      stub->editorID = "PlayCredits";
      // Value: 0
      lo._acceptHardcodedForm(stub);
   }
   {  // [SMBN:05B]"Root"
      auto stub = new FormStub();
      stub->formID   = 0x05B;
      stub->formType = signatureToFormType('SMBN');
      stub->editorID = "Root";
      lo._acceptHardcodedForm(stub);
   }
}