#include "./form_save.h"
#include "../files/tes_file_writing/file_writer.h"
#include "../files/file_load_order.h"
#include "../detailed_notice.h"

#include "../exceptions/file_save_failed.h"
#include "../notices/base_form_save_error.h"

namespace dovah::load_order_interfaces {
   void form_save::log_save_warning(detailed_notice& warning) {
      warning.type    = detailed_notice::notice_type::warning;
      warning.context = detailed_notice::notice_context::form_save;
      if (!(warning.flags & detailed_notice::flag::has_file_offset)) {
         warning.set_file_offset(this->writer.get_output_position());
      }
      this->owner._log_save_warning(warning);
   }
   void form_save::throw_save_error(const notices::base_form_save_error& error) {
      auto ex = exceptions::file_save_failed(exceptions::file_save_failed::error_code::form_save_failed);
      ex.details.form_save_error.reset((notices::base_form_save_error*)error.clone());
      throw ex;
   }
}