#pragma once
#include "../base_file_load_warning.h"
#include "../../bare_form_id_t.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_warnings {
   //
   // Interior cells defined in an ESL+ESM are unstable: Skyrim Special Edition 
   // fails to properly map their form IDs to CELL GRUP IDs, which causes two 
   // problems:
   // 
   //  - If you reload a save, the game won't reload references in these cells.
   // 
   //  - If these cells are overridden in another plug-in, they break completely.
   //
   class esl_defined_an_interior_cell final : public base_file_load_warning {
      public:
         MAKE_CLONE_OVERLOAD;

      public:
         constexpr esl_defined_an_interior_cell() {}

         struct {
            bare_form_id_t local = 0;
            bare_form_id_t global = 0;
         } form_ids;
   };
}
#include "../_util.undef.h"