#include "./load_hardcoded_form_data.h"
#include <array>
#include <utility> // std::pair
#include "dovah/facts/form_ids/hardcoded.h"
#include "dovah/form_data/base_managed_form_data.h"
#include "dovah/form_stub.h"

namespace {
   constexpr const auto hardcoded_global_values = std::array{
      std::pair{ dovah::facts::form_ids::hardcoded::GameYear,       77 },
      std::pair{ dovah::facts::form_ids::hardcoded::GameMonth,       7 },
      std::pair{ dovah::facts::form_ids::hardcoded::GameDay,        17 },
      std::pair{ dovah::facts::form_ids::hardcoded::GameHour,       12 },
      std::pair{ dovah::facts::form_ids::hardcoded::GameDaysPassed,  1 },
      std::pair{ dovah::facts::form_ids::hardcoded::TimeScale,      30 },
      std::pair{ dovah::facts::form_ids::hardcoded::PlayCredits,     0 },
   };
}

namespace dovah::form_data_helpers {
   extern void load_hardcoded_form_data(base_managed_form_data& data) {
      const auto& stub = data.stub;
      auto&       alo  = stub.get_load_order();
      
      /*//
      data.setup(alo); // currently only needed for ImagespaceModifier forms
      switch (stub.formID) {
         //
         // Extra configuration:
         //
         case facts::form_ids::hardcoded::DoorMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "MarkerTeleport.nif";
            break;
         case facts::form_ids::hardcoded::TravelMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Travel.nif";
            break;
         case facts::form_ids::hardcoded::NorthMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_North.nif";
            break;
         case facts::form_ids::hardcoded::PrisonMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Prison.nif";
            break;
         case facts::form_ids::hardcoded::DivineMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Divine.nif";
            break;
         case facts::form_ids::hardcoded::TempleMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Temple.nif";
            break;
         case facts::form_ids::hardcoded::MapMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Map.nif";
            break;
         case facts::form_ids::hardcoded::HorseMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "Marker_Horse.nif";
            break;
         case facts::form_ids::hardcoded::PlayerRef: // PlayerRef
            ((form_data::actor<managed_form_params>&)stub).base_form.unmanaged_set(lo.get_form(form_type::actor_base, 0x007)); // PlayerRef's base form is Player
            break;
         case facts::form_ids::hardcoded::eyeReanimate:
            ((form_data::eyes<managed_form_params>&)stub).name = "Reanimate Eyes";
            break;
         case facts::form_ids::hardcoded::AdultFemaleVoice1:
            ((form_data::voicetype<managed_form_params>&)stub).voicetype_flags |= loaded_forms::Voicetype::voicetype_flag::female;
            break;
         case facts::form_ids::hardcoded::COCMarkerHeading:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "MarkerCOCHeading.nif";
            break;
         case facts::form_ids::hardcoded::XMarkerHeading:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "MarkerXHeading.nif";
            break;
         case facts::form_ids::hardcoded::XMarker:
            ((form_data::statik<managed_form_params>&)stub).model.model_path = "MarkerX.nif";
            break;
         case facts::form_ids::hardcoded::PlayCredits:
            ((form_data::global<managed_form_params>&)stub).value_type = form_data::global<managed_form_params>::value_type::int16;
            break;
         default:
            {
               bool done = false;
               for (const auto& pair : hardcoded_global_values) {
                  if (stub.formID == pair.first) {
                     ((form_data::global<managed_form_params>&)stub).value = pair.second;
                     done = true;
                     break;
                  }
               }
               if (done)
                  break;
            }
            break;
      }
      //*/
   }
}