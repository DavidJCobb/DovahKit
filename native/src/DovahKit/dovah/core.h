#pragma once
#include <cstdint>
#include "./data/game.h"
#include "./form_id_t.h"
#include "./form_reference_t.h"
#include "./form_types.h"

namespace dovah {
   using bare_form_id_t = uint32_t;

   inline constexpr uint32_t hardcoded_form_id_mask = 0x000007FF; // Mask for form IDs that are hardcoded forms.
   inline constexpr uint32_t plugin_form_id_mask    = 0x00FFF800; // Mask for form IDs that are not hardcoded forms.
   inline constexpr uint32_t minimum_plugin_form_id = 0x00000800; // Minimum non-load-order-prefixed form ID for a non-hardcoded form.
   inline constexpr uint32_t form_id_prefix_mask    = 0xFF000000; // Mask to get the load order prefix of a form ID.
}