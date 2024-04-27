#include "./file_load.h"
#include "../files/file_load_order.h"

#include "../notices/base_file_load_warning.h"

namespace dovah::load_order_interfaces {
   void file_load::log_warning(const notices::base_file_load_warning& notice) {
      this->owner._log_warning(notice);
   }
}