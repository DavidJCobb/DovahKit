#pragma once
#include "../../../helpers/singleton.h"

namespace dovahscript::core::subsystems {
   class permissions : cobb::singleton {
      public:
         static permissions& get() {
            static permissions instance;
            return instance;
         }

         static void verify_form_write_permissions();
         static void verify_ui_permissions();

         static bool check_ui_html_permissions();
   };
}
