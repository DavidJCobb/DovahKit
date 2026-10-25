#pragma once
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   //
   // A cell was listed directly in a worldspace's "Worldspace Children" group, 
   // as opposed to being in a block or sub-block. This should only be done for 
   // the persistent cell, but the found cell is not persistent.
   //
   class apparent_persistent_cell_not_flagged_as_persistent final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr apparent_persistent_cell_not_flagged_as_persistent(
            const std::string& source_filename,
            form_stub&         stub
         ) :
            base_file_parse_warning(source_filename),
            stub(stub)
         {}

      public:
         form_stub& stub;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"