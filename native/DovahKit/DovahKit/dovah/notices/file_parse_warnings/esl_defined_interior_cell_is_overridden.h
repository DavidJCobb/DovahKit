#pragma once
#include "dovah/types/bare_form_id.h"
#include "../base_file_parse_warning.h"
namespace dovah {
   class form_stub;
}

#include "dovah/notices/_boilerplate/base_warning.define.h"
namespace dovah::notices::file_parse_warnings {
   //
   // Interior cells defined in an ESL+ESM are unstable: Skyrim Special Edition 
   // fails to properly map their form IDs to CELL GRUP IDs, which causes two 
   // problems:
   // 
   //  - If you reload a save, the game won't reload references in these cells.
   // 
   //  - If these cells are overridden in another plug-in, they break completely.
   //
   class esl_defined_interior_cell_is_overridden final : public base_file_parse_warning {
      public:
         DEFINE_NOTICE_BOILERPLATE;
      public:
         constexpr esl_defined_interior_cell_is_overridden(
            const std::string& source_filename,
            bare_form_id       overriding_record_id,
            form_stub&         overridden_stub
         ) :
            base_file_parse_warning(source_filename),
            overriding_record_id(overriding_record_id),
            overridden_stub(overridden_stub)
         {}

      public:
         bare_form_id overriding_record_id;
         form_stub&   overridden_stub;
   };
}
#include "dovah/notices/_boilerplate/base_warning.undef.h"