#pragma once
#include "lu/passkey.h"
#include "dovah/localization/category.h"
#include "dovah/types/bare_localized_string_id.h"
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
         string_to_id_maps strings_to_ids; // only used if folding identical strings is enabled

      protected:
         bare_localized_string_id _get_id_for_new_string(localization::category, const std::string&);

      public:
         void convert_localized_string(
            form_stub& containing_form,
            localization::category,
            _base_managed_localized_string_use&
         );

         void convert_localized_string(
            loaded_game_setting& containing_form
         );
   };
}