#include "./file_load.h"
#include "dovah/notices/base_file_load_warning.h"

namespace dovah::load_order_interfaces {
   void file_load::log_warning(const notices::base_file_load_warning& warning) {
      if (this->callbacks.on_warning)
         (this->callbacks.on_warning)(warning);
   }
}