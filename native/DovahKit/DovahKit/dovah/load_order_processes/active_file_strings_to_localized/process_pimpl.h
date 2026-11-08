#pragma once
#include "lu/passkey.h"
#include "dovah/localization/category.h"
#include "dovah/types/managed_localized_string_use.h"
#include "./process_params.h"
#include "./string_to_id_maps.h"
namespace dovah {
   namespace load_order_processes::active_file_strings_to_localized {
      class process;
   }
   class form_stub;
   class loaded_game_setting;
}

namespace dovah::load_order_processes::active_file_strings_to_localized {
   class process_pimpl {
      public:
         process_pimpl(lu::passkey<process>, const process_params&);

      public:
         const process_params params;

      public:
         void convert_localized_string(
            string_to_id_maps&,
            form_stub& containing_form,
            localization::category,
            _base_managed_localized_string_use&
         );

         void convert_localized_string(
            string_to_id_maps&,
            loaded_game_setting& containing_form
         );
   };
}