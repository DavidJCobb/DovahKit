#pragma once
#include "dovah/localization/language.h"
#include "./process_params.h"
namespace dovah::load_order_processes::active_file_strings_to_localized {
   class process_pimpl;
}

namespace dovah::load_order_processes::active_file_strings_to_localized {
   class process {
      public:
         process(const process_params& p);
         ~process();

         // May throw.
         void execute();

      protected:
         process_params _params;
         process_pimpl* _pimpl = nullptr;
   };
}