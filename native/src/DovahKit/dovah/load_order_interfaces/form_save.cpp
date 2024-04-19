#include "./form_save.h"
#include "../files/tes_file_writing/file_writer.h"
#include "../files/file_load_order.h"
#include "../detailed_notice.h"

namespace dovah::load_order_interfaces {
   void form_save::log_save_warning(detailed_notice& warning) {
      warning.type    = detailed_notice::notice_type::warning;
      warning.context = detailed_notice::notice_context::form_save;
      if (!(warning.flags & detailed_notice::flag::has_file_offset)) {
         warning.set_file_offset(this->writer.get_output_position());
      }
      this->owner._log_save_warning(warning);
   }
   void form_save::set_save_error(const detailed_notice& error) {
      if (this->writer.error.is_defined())
         return;
      auto& we = this->writer.error;
      we = error;
      we.type    = detailed_notice::notice_type::error;
      we.context = detailed_notice::notice_context::form_save;
      if (!(we.flags & detailed_notice::flag::has_file_offset)) {
         we.set_file_offset(this->writer.get_output_position());
      }
   }
}