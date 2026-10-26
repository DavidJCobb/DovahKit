#pragma once
#include "dovah/_passkeys/add_form_stub_source_file.h"
#include "dovah/facts/actor_value_info.h"
#include "dovah/facts/form_ids/hardcoded.h"
#include "dovah/form_stub.h"
#include "dovah/form_stub_collection.h"
#include "dovah/form_type.h"

//
// TODO: Once we have a form-data class written for STAT, with its 
//       form flags available, replace references to 0x800000 with 
//       the named "is marker" flag on that form type.
//

namespace dovah::impl::_form_stub_collection {
   inline void create_hardcoded_form_stubs(form_stub_collection& self, tes::file& hardcoded_forms_file) {
      auto _make_stub = [&hardcoded_forms_file](bare_form_id id, uint32_t record_flags = 0) -> form_stub& {
         auto* stub = new form_stub;
         stub->form_id = id;
         stub->flags.set_each_of<form_stub::flag::is_hardcoded>();
         stub->_add_source_file({}, hardcoded_forms_file, 0, record_flags);
         return *stub;
      };

      form_stub* Player = nullptr;
      {  // Actor values
         for (auto& item : facts::all_actor_value_info) {
            auto& stub = _make_stub(item.form_id);
            stub.form_type = form_type::actor_value_info;
            {
               std::string working;
               working.reserve(2 + item.name.size());
               working += "AV";
               working += item.name;
               stub.editor_id = working;
            }
            self._add_hardcoded_form(stub);
         }
      }
      {  // [STAT:001]"DoorMarker"
         // Flags: 0x800000 (STAT: Is Marker) (STAT: Is Marker)
         // Model File Name: "MarkerTeleport.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DoorMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "DoorMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:002]"TravelMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_Travel.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::TravelMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "TravelMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:003]"NorthMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_North.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::NorthMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "NorthMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [DOOR:004]"PrisonMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_Prison.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::PrisonMarker, 0x800000);
         stub.form_type = form_type::door;
         stub.editor_id = "PrisonMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:005]"DivineMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_Divine.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DivineMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "DivineMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:006]"TempleMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_Temple.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::TempleMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "TempleMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [NPC_:007]"Player"
         auto& stub = _make_stub(facts::form_ids::hardcoded::Player);
         stub.form_type = form_type::actor_base;
         stub.editor_id = "Player";
         self._add_hardcoded_form(stub);
      }
      {  // [MISC:00A]"BobbyPin"
         auto& stub = _make_stub(facts::form_ids::hardcoded::BobbyPin);
         stub.form_type = form_type::misc_item;
         stub.editor_id = "BobbyPin";
         self._add_hardcoded_form(stub);
      }
      {  // [CONT:00E]"LootBag"
         // Model File Name: "Clutter\\Sack01.NIF"
         auto& stub = _make_stub(facts::form_ids::hardcoded::LootBag);
         stub.form_type = form_type::container;
         stub.editor_id = "LootBag";
         self._add_hardcoded_form(stub);
      }
      {  // [MISC:00F]"Caps001"
         //
         // The hardcoded currency form; Skyrim.esm renames and reskins this to be gold coins.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::Caps001);
         stub.form_type = form_type::misc_item;
         stub.editor_id = "Caps001";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:010]"MapMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "Marker_Map.NIF"
         auto& stub = _make_stub(facts::form_ids::hardcoded::MapMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "MapMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:012]"HorseMarker"
         // Model File "Name: Marker_Horse.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HorseMarker);
         stub.form_type = form_type::statik;
         stub.editor_id = "HorseMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [FACT:013]"CreatureFaction"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x013 as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::CreatureFaction);
         stub.form_type = form_type::faction;
         stub.editor_id = "CreatureFaction";
         self._add_hardcoded_form(stub);
      }
      {  // [ACHR:014]"PlayerRef"
         auto& stub = _make_stub(facts::form_ids::hardcoded::PlayerRef, 1 << (uint32_t)tes::record_flag::persistent);
         stub.form_type = form_type::actor;
         stub.editor_id = "PlayerRef";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:015]"MultiBoundMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         auto& stub = _make_stub(facts::form_ids::hardcoded::MultiBoundMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "MultiBoundMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:017]"PlaneMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         auto& stub = _make_stub(facts::form_ids::hardcoded::PlaneMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "PlaneMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [WATR:018]"DefaultWater"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultWater);
         stub.form_type = form_type::water_type;
         stub.editor_id = "DefaultWater";
         self._add_hardcoded_form(stub);
      }
      {  // [RACE:019]"DefaultRace"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultRace);
         stub.form_type = form_type::race;
         stub.editor_id = "DefaultRace";
         self._add_hardcoded_form(stub);
      }
      {  // [EYES:01A]"eyeReanimate"
         // Full Name: "Reanimate Eyes"
         auto& stub = _make_stub(facts::form_ids::hardcoded::eyeReanimate);
         stub.form_type = form_type::eyes;
         stub.editor_id = "eyeReanimate";
         self._add_hardcoded_form(stub);
      }
      {  // [ACTI:01B]"DefaultAshPile1"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultAshPile1);
         stub.form_type = form_type::activator;
         stub.editor_id = "DefaultAshPile1";
         // Full Name: "Ash Pile 1"
         self._add_hardcoded_form(stub);
      }
      {  // [BPTD:01C]"PlayerBodyPartData"
         auto& stub = _make_stub(facts::form_ids::hardcoded::PlayerBodyPartData);
         stub.form_type = form_type::body_part_data;
         stub.editor_id = "PlayerBodyPartData";
         self._add_hardcoded_form(stub);
      }
      {  // [BPTD:01D]"DefaultBodyPartData"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultBodyPartData);
         stub.form_type = form_type::body_part_data;
         stub.editor_id = "DefaultBodyPartData";
         self._add_hardcoded_form(stub);
      }
      {  // [ECZN:01E]"NoZoneZone"
         auto& stub = _make_stub(facts::form_ids::hardcoded::NoZoneZone);
         stub.form_type = form_type::encounter_zone;
         stub.editor_id = "NoZoneZone";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:01F]"RoomMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         auto& stub = _make_stub(facts::form_ids::hardcoded::RoomMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "RoomMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:020]"PortalMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         auto& stub = _make_stub(facts::form_ids::hardcoded::PortalMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "PortalMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:021]"CollisionMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         auto& stub = _make_stub(facts::form_ids::hardcoded::CollisionMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "CollisionMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [ACTI:022]"DefaultAshPile2"
         // Full Name: "Ash Pile 2"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultAshPile2);
         stub.form_type = form_type::activator;
         stub.editor_id = "DefaultAshPile2";
         self._add_hardcoded_form(stub);
      }
      {  // [CELL:025]"NavmeshGenCell"
         auto& stub = _make_stub(facts::form_ids::hardcoded::NavmeshGenCell);
         stub.form_type = form_type::cell;
         stub.editor_id = "NavmeshGenCell";
         self._add_hardcoded_form(stub);
      }
      {  // [TXST:028]"NullTextureSet"
         auto& stub = _make_stub(facts::form_ids::hardcoded::NullTextureSet);
         stub.form_type = form_type::texture_set;
         stub.editor_id = "NullTextureSet";
         self._add_hardcoded_form(stub);
      }
      {  // [VTYP:02D]"AdultMaleVoice1"
         auto& stub = _make_stub(facts::form_ids::hardcoded::AdultMaleVoice1);
         stub.form_type = form_type::voicetype;
         stub.editor_id = "AdultMaleVoice1";
         self._add_hardcoded_form(stub);
      }
      {  // [VTYP:02E]"AdultFemaleVoice1"
         auto& stub = _make_stub(facts::form_ids::hardcoded::AdultFemaleVoice1);
         stub.form_type = form_type::voicetype;
         stub.editor_id = "AdultFemaleVoice1";
         self._add_hardcoded_form(stub);
      }
      {  // [DOBJ:031]
         //
         // This form is a singleton, so the game only creates it when it is first loaded or 
         // accessed. As far as I can see, the game doesn't bother giving it a specific form 
         // ID. Official game files use form ID 0x031, though, so I assume it's hardcoded 
         // into the Creation Kit.
         //
         auto& stub = _make_stub(0x031);
         stub.form_type = form_type::default_object_manager;
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:032]"COCMarkerHeading"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "MarkerCOCHeading.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::COCMarkerHeading, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "COCMarkerHeading";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:034]"XMarkerHeading"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "MarkerXHeading.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::XMarkerHeading, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "XMarkerHeading";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:035]"GameYear"
         // Value: 77
         auto& stub = _make_stub(facts::form_ids::hardcoded::GameYear);
         stub.form_type = form_type::global;
         stub.editor_id = "GameYear";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:036]"GameMonth"
         // Value: 7
         auto& stub = _make_stub(facts::form_ids::hardcoded::GameMonth);
         stub.form_type = form_type::global;
         stub.editor_id = "GameMonth";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:037]"GameDay"
         // Value: 17
         auto& stub = _make_stub(facts::form_ids::hardcoded::GameDay);
         stub.form_type = form_type::global;
         stub.editor_id = "GameDay";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:038]"GameHour"
         // Value: 12
         auto& stub = _make_stub(facts::form_ids::hardcoded::GameHour);
         stub.form_type = form_type::global;
         stub.editor_id = "GameHour";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:039]"GameDaysPassed"
         // Value: 1
         auto& stub = _make_stub(facts::form_ids::hardcoded::GameDaysPassed);
         stub.form_type = form_type::global;
         stub.editor_id = "GameDaysPassed";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:03A]"TimeScale"
         // Value: 30
         auto& stub = _make_stub(facts::form_ids::hardcoded::TimeScale);
         stub.form_type = form_type::global;
         stub.editor_id = "TimeScale";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:03B]"XMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: "MarkerX.nif"
         auto& stub = _make_stub(facts::form_ids::hardcoded::XMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "XMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [WRLD:03C]"DefaultWorld"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultWorld);
         stub.form_type = form_type::worldspace;
         stub.editor_id = "DefaultWorld";
         self._add_hardcoded_form(stub);
      }
      {  // [CSTY:03D]"DefaultCombatstyle"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but I haven't verified whether it exists 
         // in-game. It'd certainly be odd if it didn't, but the game doesn't *appear* to pass it into 
         // TESForm::SetFormID unless I missed a call somewhere.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultCombatStyle);
         stub.form_type = form_type::combat_style;
         stub.editor_id = "DefaultCombatstyle";
         self._add_hardcoded_form(stub);
      }
      {  // [SMBN:05B]"Root"
         auto& stub = _make_stub(facts::form_ids::hardcoded::Root);
         stub.form_type = form_type::story_branch_node;
         stub.editor_id = "Root";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:061]"CellWaterCurrentMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: ""
         auto& stub = _make_stub(facts::form_ids::hardcoded::CellWaterCurrentMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "CellWaterCurrentMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:062]"WaterCurrentMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: ""
         auto& stub = _make_stub(facts::form_ids::hardcoded::WaterCurrentMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "WaterCurrentMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [GLOB:063]"PlayCredits"
         // Value: 0
         auto& stub = _make_stub(facts::form_ids::hardcoded::PlayCredits);
         stub.form_type = form_type::global;
         stub.editor_id = "PlayCredits";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:064]"FurnitureMarker01"
         auto& stub = _make_stub(facts::form_ids::hardcoded::FurnitureMarker01);
         stub.form_type = form_type::statik;
         stub.editor_id = "FurnitureMarker01";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:065]"FurnitureMarker02"
         auto& stub = _make_stub(facts::form_ids::hardcoded::FurnitureMarker02);
         stub.form_type = form_type::statik;
         stub.editor_id = "FurnitureMarker02";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:066]"FurnitureMarker03"
         auto& stub = _make_stub(facts::form_ids::hardcoded::FurnitureMarker03);
         stub.form_type = form_type::statik;
         stub.editor_id = "FurnitureMarker03";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:067]"FurnitureMarker04"
         auto& stub = _make_stub(facts::form_ids::hardcoded::FurnitureMarker04);
         stub.form_type = form_type::statik;
         stub.editor_id = "FurnitureMarker04";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:068]"FurnitureMarker05"
         auto& stub = _make_stub(facts::form_ids::hardcoded::FurnitureMarker05);
         stub.form_type = form_type::statik;
         stub.editor_id = "FurnitureMarker05";
         self._add_hardcoded_form(stub);
      }
      {  // [STAT:0C4]"WaterCurrentZoneMarker"
         // Flags: 0x800000 (STAT: Is Marker)
         // Model File Name: ""
         auto& stub = _make_stub(facts::form_ids::hardcoded::WaterCurrentZoneMarker, 0x800000);
         stub.form_type = form_type::statik;
         stub.editor_id = "WaterCurrentZoneMarker";
         self._add_hardcoded_form(stub);
      }
      {  // [EFSH:146]"LifeDetected"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x146 as an argument to any function, let alone TESForm::SetFormID, which means 
         // that it does not instantiate this form programmatically.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::LifeDetected);
         stub.form_type = form_type::effect_shader;
         stub.editor_id = "LifeDetected";
         self._add_hardcoded_form(stub);
      }
      {  // [MGEF:14A]"ScriptEffect"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x14A as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::ScriptEffect);
         stub.form_type = form_type::magic_effect;
         stub.editor_id = "ScriptEffect";
         self._add_hardcoded_form(stub);
      }
      {  // [MGEF:14C]"WardConcSelf0"
         //
         // NOTE: This form is hardcoded into the Creation Kit, but NOT into the game itself. Skyrim.esm 
         // never passes 0x14C as an argument to TESForm::SetFormID, which means that it does not instan-
         // tiate this form programmatically.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::WardConcSelf0);
         stub.form_type = form_type::magic_effect;
         stub.editor_id = "WardConcSelf0";
         self._add_hardcoded_form(stub);
      }
      {  // [WTHR:15E]"DefaultWeather"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultWeather);
         stub.form_type = form_type::weather;
         stub.editor_id = "DefaultWeather";
         self._add_hardcoded_form(stub);
      }
      {  // [WTHR:15F]"DefaultClimate"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultClimate);
         stub.form_type = form_type::climate;
         stub.editor_id = "DefaultClimate";
         self._add_hardcoded_form(stub);
      }
      {  // [IMGS:160]"DefaultImageSpaceInterior"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultImageSpaceInterior);
         stub.form_type = form_type::imagespace;
         stub.editor_id = "DefaultImageSpaceInterior";
         self._add_hardcoded_form(stub);
      }
      {  // [IMGS:161]"DefaultImageSpaceExterior"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultImageSpaceExterior);
         stub.form_type = form_type::imagespace;
         stub.editor_id = "DefaultImageSpaceExterior";
         self._add_hardcoded_form(stub);
      }
      {  // [IMAD:162]"GetHit"
         auto& stub = _make_stub(facts::form_ids::hardcoded::GetHit);
         stub.form_type = form_type::imagespace_modifier;
         stub.editor_id = "GetHit";
         self._add_hardcoded_form(stub);
      }
      {  // [FLST:163]"HelpManualPC"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpManualPC);
         stub.form_type = form_type::formlist;
         stub.editor_id = "HelpManualPC";
         self._add_hardcoded_form(stub);
      }
      {  // [IMAD:164]"ImageSpaceConcussion"
         auto& stub = _make_stub(facts::form_ids::hardcoded::ImageSpaceConcussion);
         stub.form_type = form_type::imagespace_modifier;
         stub.editor_id = "ImageSpaceConcussion";
         self._add_hardcoded_form(stub);
      }
      {  // [FLST:165]"HelpManualXBox"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpManualXbox);
         stub.form_type = form_type::formlist;
         stub.editor_id = "HelpManualXBox";
         self._add_hardcoded_form(stub);
      }
      {  // [IMAD:166]"ExplosionInFace"
         auto& stub = _make_stub(facts::form_ids::hardcoded::ExplosionInFace);
         stub.form_type = form_type::imagespace_modifier;
         stub.editor_id = "ExplosionInFace";
         self._add_hardcoded_form(stub);
      }
      {  // [IMGS:167]"DefaultImageSpace"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultImageSpace);
         stub.form_type = form_type::imagespace;
         stub.editor_id = "DefaultImageSpace";
         self._add_hardcoded_form(stub);
      }
      //
      // Form IDs from 0x168 to 0x187 are reserved for Message forms, but Skyrim doesn't 
      // actually create all of those forms. The function that creates most hardcoded forms 
      // has a local array of editor IDs as const char* values, but some are nullptr; it 
      // loops over that range of form IDs and creates BGSMessage instances only for the 
      // editor IDs that aren't nullptr.
      //
      {  // [MESG:16D]"HelpPipBoyItems"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpPipBoyItems);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpPipBoyItems";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:16E]"HelpPipBoyRepair"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpPipBoyRepair);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpPipBoyRepair";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:176]"HelpChargenTagSkills"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpCharGenTagSkills);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpChargenTagSkills";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:177]"HelpChargenRace"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpCharGenRace);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpChargenRace";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:178]"HelpLeveling"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpLeveling);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpLeveling";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:179]"HelpDialogue"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpDialogue);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpDialogue";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:17B]"HelpHacking"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpHacking);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpHacking";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:17C]"HelpLockpickingPC"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpLockpickingPC);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpLockpickingPC";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:17D]"HelpVATSPC"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpVATSPC);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpVATSPC";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:17E]"HelpContainer"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpContainer);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpContainer";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:17F]"HelpBarter"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpBarter);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpBarter";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:180]"HelpTerminal"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpTerminal);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpTerminal";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:181]"HelpPipBoyStats"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpPipBoyStats);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpPipBoyStats";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:182]"HelpPipBoyData"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpPipBoyData);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpPipBoyData";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:183]"HelpVATSXBox"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpVATSXBox);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpVATSXBox";
         self._add_hardcoded_form(stub);
      }
      {  // [MESG:184]"HelpLockpickingXBox"
         auto& stub = _make_stub(facts::form_ids::hardcoded::HelpLockpickingXBox);
         stub.form_type = form_type::message;
         stub.editor_id = "HelpLockpickingXBox";
         self._add_hardcoded_form(stub);
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
         auto& stub = _make_stub(facts::form_ids::hardcoded::HairColorListDoNotUse);
         stub.form_type = form_type::formlist;
         stub.editor_id = "HairColorListDoNotUse";
         self._add_hardcoded_form(stub);
      }
      {  // [WEAP:1F4]"Unarmed"
         // DATA - Animation Type: HandToHandMelee (0)
         auto& stub = _make_stub(facts::form_ids::hardcoded::Unarmed);
         stub.form_type = form_type::weapon;
         stub.editor_id = "Unarmed";
         self._add_hardcoded_form(stub);
      }
      {  // [EXPL:1F5]"Default Water Explosion"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultWaterExplosion);
         stub.form_type = form_type::explosion;
         stub.editor_id = "Default Water Explosion";
         // Full Name: "Water Explosion"
         self._add_hardcoded_form(stub);
      }
      {  // [WEAP:1F6]"GasTrap Dummy"
         // Full Name: "GasTrap Dummy"
         auto& stub = _make_stub(facts::form_ids::hardcoded::GasTrapDummy);
         stub.form_type = form_type::weapon;
         stub.editor_id = "GasTrap Dummy";
         self._add_hardcoded_form(stub);
      }
      {  // [IPDS:276]"DefaultImpactDataSet"
         auto& stub = _make_stub(facts::form_ids::hardcoded::DefaultImpactDataSet);
         stub.form_type = form_type::impact_data_set;
         stub.editor_id = "DefaultImpactDataSet";
         self._add_hardcoded_form(stub);
      }
      {  // [NONE:28A]"PapyrusPersistenceForm"
         //
         // Behaves similarly to the CommandingActorPersistenceForm (see below), but with some 
         // additional ties to the Papyrus VM. Probably used to ensure that TESObjectREFRs stay 
         // loaded (i.e. "promoted" and flagged as persistent) while a Papyrus variable is 
         // pointing to them.
         //
         auto& stub = _make_stub(facts::form_ids::hardcoded::PapyrusPersistenceForm);
         stub.form_type = form_type::none;
         stub.editor_id = "PapyrusPersistenceForm";
         self._add_hardcoded_form(stub);
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
         auto& stub = _make_stub(facts::form_ids::hardcoded::CommandingActorPersistenceForm);
         stub.form_type = form_type::none;
         stub.editor_id = "CommandingActorPersistenceForm";
         self._add_hardcoded_form(stub);
      }
   }
}