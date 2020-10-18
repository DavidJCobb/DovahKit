#include "file_write_error.h"
#include "../notice_code_list.h"

namespace dovah {
   bool file_write_error::has_file_offset() const noexcept {
      switch (this->code) {
         case notice_code::none:
         case notice_code::no_active_file:
         case notice_code::cannot_save_right_now:
         case notice_code::no_filename_specified:
         case notice_code::save_complete_but_reopen_failed:
         case notice_code::forms_out_of_esl_form_id_range:
         case notice_code::too_many_dependencies:
         case notice_code::load_order_would_overflow_into_lights:
         case notice_code::load_order_contains_light_files:
            return false;
      }
      return true;
   }
   bool file_write_error::requires_full_reload() const noexcept {
      switch (this->code) {
         case notice_code::save_complete_but_reopen_failed:
         case notice_code::game_conversion_form_cleanup_failed:
            return true;
      }
      return false;
   }
}