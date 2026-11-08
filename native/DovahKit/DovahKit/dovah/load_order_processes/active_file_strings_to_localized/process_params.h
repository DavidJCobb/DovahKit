#pragma once
#include "lu/enums/enum_flags.h"
#include "dovah/localization/language.h"
namespace dovah {
   class active_load_order;
}

namespace dovah::load_order_processes::active_file_strings_to_localized {
   struct process_params {
      active_load_order& active_load_order;

      // If enabled, any two identical non-localized strings will share an ID. This 
      // is not recommended; see documentation. Prefer leaving identical strings as 
      // separate, and then manually merging string IDs afterward for those cases 
      // where you know to a certainty that the results would be acceptable.
      bool merge_identical_strings = false;

      // Languages to produce output for. Languages that aren't flagged will not 
      // have l-strings created for them.
      lu::enum_flags<localization::language, localization::language_count> languages;
   };
}