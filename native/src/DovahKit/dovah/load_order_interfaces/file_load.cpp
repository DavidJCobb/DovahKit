#include "./file_load.h"
#include "../files/file_load_order.h"
#include "../detailed_notice.h"

#include "../notices/base_file_load_warning.h"

namespace dovah::load_order_interfaces {
   void file_load::log_load_warning(detailed_notice& warning) {
      warning.type    = detailed_notice::notice_type::warning;
      warning.context = detailed_notice::notice_context::file_load;
      //
      this->owner._log_load_warning(warning);
   }

   void file_load::log_warning(const notices::base_file_load_warning& notice) {
      this->owner._log_warning(notice);
   }
}