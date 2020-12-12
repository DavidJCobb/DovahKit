#include "hardcoded.h"
#include "../../core.h"
#include "../../form_stub.h"
#include "../../files/tes_file_reading/file_loader.h"
#include "../../files/file_load_order.h"
#include "../../data/actor_values.h"
#include "../Activator.h"
#include "../Actor.h"
#include "../ActorBase.h"
#include "../Container.h"
#include "../DefaultObjectManager.h"
#include "../Form.h"
#include "../FormList.h"
#include "../Voicetype.h"
#include "../Worldspace.h"

namespace dovah {
   extern loaded_forms::Form* instantiate_hardcoded_form(form_stub& stub) {
      assert(stub.form == nullptr && "Why are we trying to create a loaded-form instance for a stub (for a hardcoded form) that already has one?");
      //
      // First, let's handle singleton forms.
      //
      if (stub.formType == form_type::default_object_manager)
         return new loaded_forms::DefaultObjectManager;
      //
      // Next, we'll check the form ID. First, we'll look into the actor values.
      //
      {  // Actor values
         auto& list = data::actor_Value_info_list::get();
         for (uint16_t i = 0; i < list.count; i++) {
            auto& entry = list.list[i];
            if (stub.formID == entry.formID)
               return nullptr; // TODO: we need a class for these
         }
      }
      //
      // Now, some of the other forms have relationships to each other, which we'll need to set 
      // up, so first, we need to get the stub's owning load order in order to be able to look 
      // up other forms by their IDs.
      //
      // When setting up these relationships, we should use (form_reference_t::unmanaged_set), 
      // because use info should've been set up already in (build_hardcoded_form_outbound_refs).
      //
      auto* file = stub.get_file_at_index(0);
      if (!file) {
         #if _DEBUG
            __debugbreak();
         #endif
         return nullptr;
      }
      auto& lo = file->get_load_order();
      switch (stub.formID) {
         case 0x14:
            {
               auto* form = new loaded_forms::Actor;
               form->base_form.unmanaged_set(lo.get_form(form_type::actor_base, 0x007));
               return form;
            }
            break;
         case 0x2D:
            return new loaded_forms::Voicetype;
         case 0x2E:
            return new loaded_forms::Voicetype;
         case 0x3C:
            return new loaded_forms::Worldspace;
         case 0x163:
            return new loaded_forms::FormList;
         case 0x165:
            return new loaded_forms::FormList;
         case 0x1F3:
            return new loaded_forms::FormList;
      }
      return nullptr;
   }
   void add_hardcoded_forms_to_load_order(file_load_order& lo) {
      //
      // NOTE: Some forms below are listed as being hardcoded into the CK but not the game. This 
      //       is an educated guess; I have not reverse-engineered the CK to verify this. These 
      //       IDs are in the range reserved for hardcoded forms, but it's equally possible that 
      //       they *were* hardcoded at some point during early development and remained inside 
      //       of Skyrim.esm as leftovers after being removed from both the game *and* the CK.
      //
      form_stub* Player = nullptr;
      {  // Actor values
         auto& list = data::actor_Value_info_list::get();
         for (uint16_t i = 0; i < list.count; i++) {
            auto& entry = list.list[i];
            //
            auto stub = new form_stub();
            stub->formID   = entry.formID;
            stub->formType = form_type_info::signature_to_form_type('AVIF');
            stub->editorID = entry.name;
            lo._accept_hardcoded_form(stub);
         }
      }
      {  // [STAT:001]"DoorMarker"
         auto stub = new form_stub();
         stub->formID   = 0x001;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "DoorMarker";
         // Flags: 0x800000
         // Model File Name: "MarkerTeleport.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:002]"TravelMarker"
         auto stub = new form_stub();
         stub->formID   = 0x002;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "TravelMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_Travel.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:003]"NorthMarker"
         auto stub = new form_stub();
         stub->formID   = 0x003;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "NorthMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_North.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [DOOR:004]"PrisonMarker"
         auto stub = new form_stub();
         stub->formID   = 0x004;
         stub->formType = form_type_info::signature_to_form_type('DOOR');
         stub->editorID = "PrisonMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_Prison.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:005]"DivineMarker"
         auto stub = new form_stub();
         stub->formID   = 0x005;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "DivineMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_Divine.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:006]"TempleMarker"
         auto stub = new form_stub();
         stub->formID   = 0x006;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "TempleMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_Temple.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [NPC_:007]"Player"
         auto stub = new form_stub();
         stub->formID   = hardcoded_form_ids::Player;
         stub->formType = form_type_info::signature_to_form_type('NPC_');
         stub->editorID = "Player";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MISC:00A]"BobbyPin"
         auto stub = new form_stub();
         stub->formID   = 0x00A;
         stub->formType = form_type_info::signature_to_form_type('MISC');
         stub->editorID = "BobbyPin";
         lo._accept_hardcoded_form(stub);
      }
      {  // [CONT:00E]"LootBag"
         auto stub = new form_stub();
         stub->formID   = 0x00E;
         stub->formType = form_type_info::signature_to_form_type('CONT');
         stub->editorID = "LootBag";
         // Model File Name: "Clutter\\Sack01.NIF"
         lo._accept_hardcoded_form(stub);
      }
      {  // [MISC:00F]"Caps001"
         //
         // The hardcoded currency form; Skyrim.esm renames and reskins this to be gold coins.
         //
         auto stub = new form_stub();
         stub->formID   = 0x00F;
         stub->formType = form_type_info::signature_to_form_type('MISC');
         stub->editorID = "Caps001";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:010]"MapMarker"
         auto stub = new form_stub();
         stub->formID   = 0x010;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "MapMarker";
         // Flags: 0x800000
         // Model File Name: "Marker_Map.NIF"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:012]"HorseMarker"
         auto stub = new form_stub();
         stub->formID   = 0x012;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "HorseMarker";
         // Model File "Name: Marker_Horse.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [FACT:013]"CreatureFaction"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x013 as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x013;
         stub->formType = form_type_info::signature_to_form_type('FACT');
         stub->editorID = "CreatureFaction";
         lo._accept_hardcoded_form(stub);
      }
      {  // [ACHR:014]"PlayerRef"
         auto stub = new form_stub();
         stub->formID   = hardcoded_form_ids::PlayerRef;
         stub->formType = form_type_info::signature_to_form_type('ACHR');
         stub->editorID = "PlayerRef";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:015]"MultiBoundMarker"
         auto stub = new form_stub();
         stub->formID   = 0x015;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "MultiBoundMarker";
         // Flags: 0x800000
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:017]"PlaneMarker"
         auto stub = new form_stub();
         stub->formID   = 0x017;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "PlaneMarker";
         // Flags: 0x800000
         lo._accept_hardcoded_form(stub);
      }
      {  // [WATR:018]"DefaultWater"
         auto stub = new form_stub();
         stub->formID   = hardcoded_form_ids::DefaultWater;
         stub->formType = form_type_info::signature_to_form_type('WATR');
         stub->editorID = "DefaultWater";
         lo._accept_hardcoded_form(stub);
      }
      {  // [RACE:019]"DefaultRace"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x019 as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x019;
         stub->formType = form_type_info::signature_to_form_type('RACE');
         stub->editorID = "DefaultRace";
         lo._accept_hardcoded_form(stub);
      }
      {  // [EYES:01A]"eyeReanimate"
         auto stub = new form_stub();
         stub->formID   = 0x01A;
         stub->formType = form_type_info::signature_to_form_type('EYES');
         stub->editorID = "eyeReanimate";
         // Full Name: "Reanimate Eyes"
         lo._accept_hardcoded_form(stub);
      }
      {  // [ACTI:01B]"DefaultAshPile1"
         auto stub = new form_stub();
         stub->formID   = 0x01B;
         stub->formType = form_type_info::signature_to_form_type('ACTI');
         stub->editorID = "DefaultAshPile1";
         // Full Name: "Ash Pile 1"
         lo._accept_hardcoded_form(stub);
      }
      {  // [BPTD:01C]"PlayerBodyPartData"
         auto stub = new form_stub();
         stub->formID   = 0x01C;
         stub->formType = form_type_info::signature_to_form_type('BPTD');
         stub->editorID = "PlayerBodyPartData";
         lo._accept_hardcoded_form(stub);
      }
      {  // [BPTD:01D]"DefaultBodyPartData"
         auto stub = new form_stub();
         stub->formID   = 0x01D;
         stub->formType = form_type_info::signature_to_form_type('BPTD');
         stub->editorID = "DefaultBodyPartData";
         lo._accept_hardcoded_form(stub);
      }
      {  // [ECZN:01E]"NoZoneZone"
         auto stub = new form_stub();
         stub->formID   = 0x01E;
         stub->formType = form_type_info::signature_to_form_type('ECZN');
         stub->editorID = "NoZoneZone";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:01F]"RoomMarker"
         auto stub = new form_stub();
         stub->formID   = 0x01F;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "RoomMarker";
         // Flags: 0x800000
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:020]"PortalMarker"
         auto stub = new form_stub();
         stub->formID   = 0x020;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "PortalMarker";
         // Flags: 0x800000
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:021]"CollisionMarker"
         //
         // Base form used for collision primitives placed in the Creation Kit.
         //
         auto stub = new form_stub();
         stub->formID   = 0x021;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "CollisionMarker";
         // Flags: 0x800000
         lo._accept_hardcoded_form(stub);
      }
      {  // [ACTI:022]"DefaultAshPile2"
         auto stub = new form_stub();
         stub->formID   = 0x022;
         stub->formType = form_type_info::signature_to_form_type('ACTI');
         stub->editorID = "DefaultAshPile2";
         // Full Name: "Ash Pile 2"
         lo._accept_hardcoded_form(stub);
      }
      {  // [TXST:028]"NullTextureSet"
         auto stub = new form_stub();
         stub->formID   = 0x028;
         stub->formType = form_type_info::signature_to_form_type('TXST');
         stub->editorID = "NullTextureSet";
         lo._accept_hardcoded_form(stub);
      }
      {  // [VTYP:02D]"AdultMaleVoice1"
         auto stub = new form_stub();
         stub->formID   = 0x02D;
         stub->formType = form_type_info::signature_to_form_type('VTYP');
         stub->editorID = "AdultMaleVoice1";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [VTYP:02E]"AdultFemaleVoice1"
         auto stub = new form_stub();
         stub->formID   = 0x02E;
         stub->formType = form_type_info::signature_to_form_type('VTYP');
         stub->editorID = "AdultFemaleVoice1";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [DOBJ:031]
         //
         // This form is a singleton, so the game only creates it when it is first loaded or 
         // accessed. As far as I can see, the game doesn't bother giving it a specific form 
         // ID. Official game files use form ID 0x031, though, so I assume it's hardcoded 
         // into the Creation Kit.
         //
         auto stub = new form_stub();
         stub->formID   = 0x031;
         stub->formType = form_type_info::signature_to_form_type('DOBJ');
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:032]"COCMarkerHeading"
         auto stub = new form_stub();
         stub->formID   = 0x032;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "COCMarkerHeading";
         // Flags: 0x800000
         // Model File Name: "MarkerCOCHeading.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:034]"XMarkerHeading"
         auto stub = new form_stub();
         stub->formID   = 0x034;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "XMarkerHeading";
         // Flags: 0x800000
         // Model File Name: "MarkerXHeading.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:035]"GameYear"
         auto stub = new form_stub();
         stub->formID   = 0x035;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "GameYear";
         // Value: 77
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:036]"GameMonth"
         auto stub = new form_stub();
         stub->formID   = 0x036;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "GameMonth";
         // Value: 7
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:037]"GameDay"
         auto stub = new form_stub();
         stub->formID   = 0x037;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "GameDay";
         // Value: 17
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:038]"GameHour"
         auto stub = new form_stub();
         stub->formID   = 0x038;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "GameHour";
         // Value: 12
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:039]"GameDaysPassed"
         auto stub = new form_stub();
         stub->formID   = 0x039;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "GameDaysPassed";
         // Value: 1
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:03A]"TimeScale"
         auto stub = new form_stub();
         stub->formID   = 0x03A;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "TimeScale";
         // Value: 30
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:03B]"XMarker"
         auto stub = new form_stub();
         stub->formID   = 0x03B;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "XMarker";
         // Flags: 0x800000
         // Model File Name: "MarkerX.nif"
         lo._accept_hardcoded_form(stub);
      }
      {  // [WRLD:03C]"DefaultWorld"
         auto stub = new form_stub();
         stub->formID   = 0x03C;
         stub->formType = form_type_info::signature_to_form_type('WRLD');
         stub->editorID = "DefaultWorld";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [CSTY:03D]"DefaultCombatstyle"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x03D as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x03D;
         stub->formType = form_type_info::signature_to_form_type('CSTY');
         stub->editorID = "DefaultCombatstyle";
         lo._accept_hardcoded_form(stub);
      }
      {  // [SMBN:05B]"Root"
         auto stub = new form_stub();
         stub->formID   = 0x05B;
         stub->formType = form_type_info::signature_to_form_type('SMBN');
         stub->editorID = "Root";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:061]"CellWaterCurrentMarker"
         auto stub = new form_stub();
         stub->formID   = 0x061;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "CellWaterCurrentMarker";
         // Flags: 0x800000
         // Model File Name: ""
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:062]"WaterCurrentMarker"
         auto stub = new form_stub();
         stub->formID   = 0x062;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "WaterCurrentMarker";
         // Flags: 0x800000
         // Model File Name: ""
         lo._accept_hardcoded_form(stub);
      }
      {  // [GLOB:063]"PlayCredits"
         auto stub = new form_stub();
         stub->formID   = 0x063;
         stub->formType = form_type_info::signature_to_form_type('GLOB');
         stub->editorID = "PlayCredits";
         // Value: 0
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:064]"FurnitureMarker01"
         auto stub = new form_stub();
         stub->formID   = 0x064;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "FurnitureMarker01";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:065]"FurnitureMarker02"
         auto stub = new form_stub();
         stub->formID   = 0x065;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "FurnitureMarker02";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:066]"FurnitureMarker03"
         auto stub = new form_stub();
         stub->formID   = 0x066;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "FurnitureMarker03";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:067]"FurnitureMarker04"
         auto stub = new form_stub();
         stub->formID   = 0x067;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "FurnitureMarker04";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:068]"FurnitureMarker05"
         auto stub = new form_stub();
         stub->formID   = 0x068;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "FurnitureMarker05";
         lo._accept_hardcoded_form(stub);
      }
      {  // [STAT:0C4]"WaterCurrentZoneMarker"
         auto stub = new form_stub();
         stub->formID   = 0x0C4;
         stub->formType = form_type_info::signature_to_form_type('STAT');
         stub->editorID = "WaterCurrentZoneMarker";
         // Flags: 0x800000
         // Model File Name: ""
         lo._accept_hardcoded_form(stub);
      }
      {  // [EFSH:146]"LifeDetected"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x146 as an argument to any function, let alone TESForm::SetFormID, which means 
         // that it does not instantiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x146;
         stub->formType = form_type_info::signature_to_form_type('EFSH');
         stub->editorID = "LifeDetected";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MGEF:14A]"ScriptEffect"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x14A as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x14A;
         stub->formType = form_type_info::signature_to_form_type('MGEF');
         stub->editorID = "ScriptEffect";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MGEF:14C]"WardConcSelf0"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x14C as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x14C;
         stub->formType = form_type_info::signature_to_form_type('MGEF');
         stub->editorID = "WardConcSelf0";
         lo._accept_hardcoded_form(stub);
      }
      {  // [WTHR:15E]"DefaultWeather"
         auto stub = new form_stub();
         stub->formID   = 0x15E;
         stub->formType = form_type_info::signature_to_form_type('WTHR');
         stub->editorID = "DefaultWeather";
         lo._accept_hardcoded_form(stub);
      }
      {  // [WTHR:15F]"DefaultClimate"
         auto stub = new form_stub();
         stub->formID   = 0x15F;
         stub->formType = form_type_info::signature_to_form_type('CLMT');
         stub->editorID = "DefaultClimate";
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMGS:160]"DefaultImageSpaceInterior"
         auto stub = new form_stub();
         stub->formID   = 0x160;
         stub->formType = form_type_info::signature_to_form_type('IMGS');
         stub->editorID = "DefaultImageSpaceInterior";
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMGS:161]"DefaultImageSpaceExterior"
         auto stub = new form_stub();
         stub->formID   = 0x161;
         stub->formType = form_type_info::signature_to_form_type('IMGS');
         stub->editorID = "DefaultImageSpaceExterior";
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMAD:162]"GetHit"
         auto stub = new form_stub();
         stub->formID   = 0x162;
         stub->formType = form_type_info::signature_to_form_type('IMAD');
         stub->editorID = "GetHit";
         lo._accept_hardcoded_form(stub);
      }
      {  // [FLST:163]"HelpManualPC"
         auto stub = new form_stub();
         stub->formID   = 0x163;
         stub->formType = form_type_info::signature_to_form_type('FLST');
         stub->editorID = "HelpManualPC";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMAD:164]"ImageSpaceConcussion"
         auto stub = new form_stub();
         stub->formID   = 0x164;
         stub->formType = form_type_info::signature_to_form_type('IMAD');
         stub->editorID = "ImageSpaceConcussion";
         lo._accept_hardcoded_form(stub);
      }
      {  // [FLST:165]"HelpManualXBox"
         auto stub = new form_stub();
         stub->formID   = 0x165;
         stub->formType = form_type_info::signature_to_form_type('FLST');
         stub->editorID = "HelpManualXBox";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMAD:166]"ExplosionInFace"
         auto stub = new form_stub();
         stub->formID   = 0x166;
         stub->formType = form_type_info::signature_to_form_type('IMAD');
         stub->editorID = "ExplosionInFace";
         lo._accept_hardcoded_form(stub);
      }
      {  // [IMGS:167]"DefaultImageSpace"
         auto stub = new form_stub();
         stub->formID   = 0x167;
         stub->formType = form_type_info::signature_to_form_type('IMGS');
         stub->editorID = "DefaultImageSpace";
         lo._accept_hardcoded_form(stub);
      }
      //
      // Form IDs from 0x168 to 0x187 are reserved for Message forms, but Skyrim doesn't 
      // actually create all of those forms. The function that creates most hardcoded forms 
      // has a local array of editor IDs as const char* values, but some are nullptr; it 
      // loops over that range of form IDs and creates BGSMessage instances only for the 
      // editor IDs that aren't nullptr.
      //
      {  // [MESG:16D]"HelpPipBoyItems"
         auto stub = new form_stub();
         stub->formID   = 0x16D;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpPipBoyItems";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:16E]"HelpPipBoyRepair"
         auto stub = new form_stub();
         stub->formID   = 0x16E;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpPipBoyRepair";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:176]"HelpChargenTagSkills"
         auto stub = new form_stub();
         stub->formID   = 0x176;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpChargenTagSkills";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:177]"HelpChargenRace"
         auto stub = new form_stub();
         stub->formID   = 0x177;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpChargenRace";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:178]"HelpLeveling"
         auto stub = new form_stub();
         stub->formID   = 0x178;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpLeveling";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:179]"HelpDialogue"
         auto stub = new form_stub();
         stub->formID   = 0x179;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpDialogue";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:17B]"HelpHacking"
         auto stub = new form_stub();
         stub->formID   = 0x17B;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpHacking";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:17C]"HelpLockpickingPC"
         auto stub = new form_stub();
         stub->formID   = 0x17C;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpLockpickingPC";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:17D]"HelpVATSPC"
         auto stub = new form_stub();
         stub->formID   = 0x17D;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpVATSPC";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:17E]"HelpContainer"
         auto stub = new form_stub();
         stub->formID   = 0x17E;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpContainer";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:17F]"HelpBarter"
         auto stub = new form_stub();
         stub->formID   = 0x17F;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpBarter";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:180]"HelpTerminal"
         auto stub = new form_stub();
         stub->formID   = 0x180;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpTerminal";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:181]"HelpPipBoyStats"
         auto stub = new form_stub();
         stub->formID   = 0x181;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpPipBoyStats";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:182]"HelpPipBoyData"
         auto stub = new form_stub();
         stub->formID   = 0x182;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpPipBoyData";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:183]"HelpVATSXBox"
         auto stub = new form_stub();
         stub->formID   = 0x183;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpVATSXBox";
         lo._accept_hardcoded_form(stub);
      }
      {  // [MESG:184]"HelpLockpickingXBox"
         auto stub = new form_stub();
         stub->formID   = 0x184;
         stub->formType = form_type_info::signature_to_form_type('MESG');
         stub->editorID = "HelpLockpickingXBox";
         lo._accept_hardcoded_form(stub);
      }
      //
      // Hardcoded Message forms end at (and include) 0x187 (though forms are not defined 
      // for all of these IDs).
      //
      {  // [FLST:1F3]"HairColorListDoNotUse"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x1F3 as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto stub = new form_stub();
         stub->formID   = 0x1F3;
         stub->formType = form_type_info::signature_to_form_type('FLST');
         stub->editorID = "HairColorListDoNotUse";
         //
         lo._accept_hardcoded_form(stub);
      }
      {  // [WEAP:1F4]"Unarmed"
         auto stub = new form_stub();
         stub->formID   = 0x1F4;
         stub->formType = form_type_info::signature_to_form_type('WEAP');
         stub->editorID = "Unarmed";
         // DATA - Animation Type: HandToHandMelee (0)
         lo._accept_hardcoded_form(stub);
      }
      {  // [EXPL:1F5]"Default Water Explosion"
         auto stub = new form_stub();
         stub->formID   = 0x1F5;
         stub->formType = form_type_info::signature_to_form_type('EXPL');
         stub->editorID = "Default Water Explosion";
         // Full Name: "Water Explosion"
         lo._accept_hardcoded_form(stub);
      }
      {  // [WEAP:1F6]"GasTrap Dummy"
         auto stub = new form_stub();
         stub->formID   = 0x1F6;
         stub->formType = form_type_info::signature_to_form_type('WEAP');
         stub->editorID = "GasTrap Dummy";
         // Full Name: "GasTrap Dummy"
         lo._accept_hardcoded_form(stub);
      }
      {  // [IPDS:276]"DefaultImpactDataSet"
         auto stub = new form_stub();
         stub->formID   = 0x276;
         stub->formType = form_type_info::signature_to_form_type('IPDS');
         stub->editorID = "DefaultImpactDataSet";
         lo._accept_hardcoded_form(stub);
      }
      {  // [NONE:28A]"PapyrusPersistenceForm"
         //
         // Behaves similarly to the CommandingActorPersistenceForm (see below), but with some 
         // additional ties to the Papyrus VM. Probably used to ensure that TESObjectREFRs stay 
         // loaded (i.e. "promoted" and flagged as persistent) while a Papyrus variable is 
         // pointing to them.
         //
         auto stub = new form_stub();
         stub->formID   = 0x28A;
         stub->formType = form_type_info::signature_to_form_type('NONE');
         stub->editorID = "PapyrusPersistenceForm";
         lo._accept_hardcoded_form(stub);
      }
      {  // [NONE:294]"CommandingActorPersistenceForm"
         //
         // This form is initialized in game as a generic TESForm, and can appear in references' 
         // ExtraPromotedRef arrays. The code that removes it from these arrays checks whether 
         // doing so has emptied the array; if so, it un-flags the reference as persistent and, 
         // if it has no parent cell, passes it to the garbage collector immediately. Presumably, 
         // then, this form is used as a sentinel to "promote" NPCs that are being commanded by 
         // other actors.
         //
         auto stub = new form_stub();
         stub->formID   = 0x294;
         stub->formType = form_type_info::signature_to_form_type('NONE');
         stub->editorID = "CommandingActorPersistenceForm";
         lo._accept_hardcoded_form(stub);
      }
   }
   void build_hardcoded_form_outbound_refs(form_stub_use_info_builder& uib) {
      auto* stub = uib.stub();
      switch (stub->formID) {
         case 0x00000014: // [ACHR]PlayerRef
            uib.add_outbound_reference(0x00000007, use_info_entry::flag::object_reference);
            break;
         case 0x0000015E: // [WTHR]DefaultWeather
            //
            // The Creation Kit lists this form as referring to [IMGS:167]DefaultImageSpace, but 
            // when the game initializes both forms, it doesn't actually link them together on 
            // its own. Presumably, it's just Skyrim.esm that does it, though that doesn't mean 
            // it isn't a Creation Kit-level default.
            //
            break;
      }
   }
}