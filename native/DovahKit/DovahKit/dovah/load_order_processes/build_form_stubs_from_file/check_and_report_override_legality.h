#pragma once
#include "dovah/callback_sets/data_load.h"
#include "dovah/form_stub.h"
#include "dovah/form_type.h"
#include "dovah/tes/file.h"

#include "dovah/exceptions/file_parse_errors/form_override_has_type_mismatch.h"
#include "dovah/notices/file_parse_warnings/form_override_has_armo_arma_mismatch.h"

namespace dovah::load_order_processes::build_form_stubs_from_file {
   inline void check_and_report_override_legality(
      const callback_sets::data_load& callbacks,
      form_stub&   overriding_stub,
      form_stub&   overridden_stub,
      size_t       override_record_offset,
      bare_form_id override_record_id
   ) {
      auto type_a = overridden_stub.form_type;
      auto type_b = overriding_stub.form_type;
      if (type_a == type_b) {
         return;
      }
      auto* overridden_file = overridden_stub.get_file_at_index(0);
      auto* overriding_file = overriding_stub.get_file_at_index(-1);
      //
      // We're loading an override, but its form type doesn't match that of the overridden form. 
      // This is a hard error in almost all cases, but there is one exception: Skyrim allows 
      // ARMA and ARMO records to override each other. This is a legacy behavior originating 
      // from Fallout 3's engine. We should warn on all mismatched overrides, but skip any 
      // mismatched overrides that aren't ARMA/ARMO.
      //
      const bool is_armo_arma = (type_a == form_type::armor || type_b == form_type::armor) && (type_a == form_type::armor_addon || type_b == form_type::armor_addon);
      if (is_armo_arma) {
         if (callbacks.on_warning) {
            (callbacks.on_warning)(
               notices::file_parse_warnings::form_override_has_armo_arma_mismatch(
                  overriding_file->path().filename().string(),
                  override_record_id,
                  overriding_stub.form_id,
                  overridden_file->path().filename().string(),
                  overridden_stub
               )
            );
         }
         return;
      }
      throw exceptions::file_parse_errors::form_override_has_type_mismatch(
         *overriding_file,
         overridden_stub.form_id,
         override_record_offset,
         override_record_id,
         overriding_stub.form_type,
         *overridden_file,
         overridden_stub.form_type
      );
   }
}