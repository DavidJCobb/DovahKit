#pragma once
#include "lu/enums/enum_flags.h"
#include "dovah/localization/language.h"
namespace dovah {
   class active_load_order;
}

namespace dovah::load_order_processes::active_file_strings_to_localized {
   struct process_params {
      active_load_order& active_load_order;
      lu::enum_flags<localization::language, localization::language_count> languages;
   };
}