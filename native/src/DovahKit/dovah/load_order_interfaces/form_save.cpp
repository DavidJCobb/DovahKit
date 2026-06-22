#include "./form_save.h"
#include "../files/tes_file_writing/file_writer.h"
#include "../files/file_load_order.h"

#include "../exceptions/file_save_failed.h"
#include "../notices/base_form_save_error.h"
#include "../notices/base_form_save_warning.h"

namespace dovah::load_order_interfaces {
   const tes_file_writing::write_config& form_save::get_save_config() const noexcept {
      return this->writer.config;
   }

   void form_save::log_save_warning(notices::base_form_save_warning& warning) {
      warning.file_info.file_offset = this->writer.get_output_position();
      this->owner._log_warning(warning);
   }
   void form_save::throw_save_error(const notices::base_form_save_error& error) {
      auto ex = exceptions::file_save_failed(exceptions::file_save_failed::error_code::form_save_failed);
      ex.details.form_save_error.reset((notices::base_form_save_error*)error.clone());
      throw ex;
   }
}