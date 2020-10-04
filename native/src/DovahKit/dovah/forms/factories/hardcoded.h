#pragma once
#include "../../core.h"

namespace dovah {
   class file_load_order;
   class form_stub;

   void add_hardcoded_forms_to_load_order(file_load_order&); // create form_stubs and loaded form data for hardcoded forms, and add them to the load order. call this before reading any files.
   void build_hardcoded_form_outbound_refs(form_stub&); // create outbound use info for hardcoded forms that have not been overridden.

   namespace hardcoded_form_ids {
      static constexpr bare_form_id_t Player       = 0x00000007;
      static constexpr bare_form_id_t PlayerRef    = 0x00000014;
      static constexpr bare_form_id_t DefaultWater = 0x00000018;
   }
}