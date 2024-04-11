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
      static constexpr bare_form_id_t PrisonMarker     = 0x00000004;
      static constexpr bare_form_id_t DivineMarker     = 0x00000005;
      static constexpr bare_form_id_t TempleMarker     = 0x00000006;
      static constexpr bare_form_id_t Player           = 0x00000007;
      static constexpr bare_form_id_t MapMarker        = 0x00000010;
      static constexpr bare_form_id_t HorseMarker      = 0x00000012;
      static constexpr bare_form_id_t PlayerRef        = 0x00000014;
      static constexpr bare_form_id_t MultiBoundMarker = 0x00000015;
      static constexpr bare_form_id_t DefaultWater     = 0x00000018;
      static constexpr bare_form_id_t RoomMarker       = 0x0000001F;
      static constexpr bare_form_id_t NullTextureSet   = 0x00000028;
      static constexpr bare_form_id_t COCMarkerHeading = 0x00000032;
      static constexpr bare_form_id_t XMarkerHeading   = 0x00000034;
      static constexpr bare_form_id_t XMarker          = 0x0000003B;
   }
}