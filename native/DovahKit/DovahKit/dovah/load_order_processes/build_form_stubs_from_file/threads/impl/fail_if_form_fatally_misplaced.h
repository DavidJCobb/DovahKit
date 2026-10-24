#pragma once
#include "dovah/form_stub.h"
#include "dovah/form_type.h"
#include "dovah/tes/reading/record_view.h"
#include "../_base.h"

#include "dovah/exceptions/file_parse_errors/record_fatally_misplaced.h"

namespace dovah::load_order_processes::build_form_stubs_from_file::threads::impl {
   inline void fail_if_form_fatally_misplaced(
      initialization_params&  ip,
      const tes::record_view& record_view,
      form_stub&              stub
   ) {
      bool fatal = false;
      switch (stub.form_type) {
         case form_type::cell:       // must be in the correct groups; code dedicated to loading other group types can't load these forms' children
         case form_type::setting:    // must be in the GMST top-group; code dedicated to loading other group types can't load game settings
         case form_type::topic:      // must be in the DIAL top-group; code dedicated to loading other group types can't load these forms' children
         case form_type::worldspace: // must be in the WRLD top-group, so we can create them before we load their child cells, and queue parsing of their cells properly
            fatal = true;
            break;
      }
      if (!fatal) [[likely]] {
         return;
      }

      delete &stub;
      throw exceptions::file_parse_errors::record_fatally_misplaced(
         ip.source_file,
         record_view.file_offset_of_header(),
         record_view.header()
      );
   }
}