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
            return false;
      }
      return true;
   }
   bool file_write_error::requires_full_reload() const noexcept {
      switch (this->code) {
         case notice_code::save_complete_but_reopen_failed:
            return true;
      }
      return false;
   }
}