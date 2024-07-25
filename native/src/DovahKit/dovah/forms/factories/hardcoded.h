#pragma once
#include "../../core.h"

namespace dovah {
   class file_load_order;
   class form_stub;
   class form_stub_use_info_builder;
   namespace loaded_forms {
      class Form;
   }

   extern void add_hardcoded_forms_to_load_order(file_load_order&); // create form_stubs and loaded form data for hardcoded forms, and add them to the load order. call this before reading any files.
   extern void build_hardcoded_form_outbound_refs(form_stub_use_info_builder&); // create outbound use info for hardcoded forms that have not been overridden.

   extern loaded_forms::Form* instantiate_hardcoded_form(form_stub&);

   namespace hardcoded_form_ids {
      static constexpr bare_form_id_t PrisonMarker       = 0x004;
      static constexpr bare_form_id_t DivineMarker       = 0x005;
      static constexpr bare_form_id_t TempleMarker       = 0x006;
      static constexpr bare_form_id_t Player             = 0x007;
      static constexpr bare_form_id_t MapMarker          = 0x010;
      static constexpr bare_form_id_t HorseMarker        = 0x012;
      static constexpr bare_form_id_t PlayerRef          = 0x014;
      static constexpr bare_form_id_t MultiBoundMarker   = 0x015;
      static constexpr bare_form_id_t DefaultWater       = 0x018;
      static constexpr bare_form_id_t PlayerBodyPartData = 0x01C;
      static constexpr bare_form_id_t RoomMarker         = 0x01F;
      static constexpr bare_form_id_t NullTextureSet     = 0x028;
      static constexpr bare_form_id_t AdultMaleVoice1    = 0x02D;
      static constexpr bare_form_id_t AdultFemaleVoice1  = 0x02E;
      static constexpr bare_form_id_t COCMarkerHeading   = 0x032;
      static constexpr bare_form_id_t XMarkerHeading     = 0x034;
      static constexpr bare_form_id_t GameYear           = 0x035;
      static constexpr bare_form_id_t GameMonth          = 0x036;
      static constexpr bare_form_id_t GameDay            = 0x037;
      static constexpr bare_form_id_t GameHour           = 0x038;
      static constexpr bare_form_id_t GameDaysPassed     = 0x039;
      static constexpr bare_form_id_t TimeScale          = 0x03A;
      static constexpr bare_form_id_t XMarker            = 0x03B;
      static constexpr bare_form_id_t DefaultWorld       = 0x03C;
      static constexpr bare_form_id_t PlayCredits        = 0x063;
   }
}