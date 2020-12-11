#include "results.h"

namespace dovah::tes_file_reading {
   read_results::read_results() {
      this->error.context = detailed_notice::notice_context::file_load;
   }
   detailed_notice& read_results::add_warning() noexcept {
      auto& notice = this->warnings.emplace_back();
      notice.type    = detailed_notice::notice_type::warning;
      notice.context = detailed_notice::notice_context::file_load;
      return notice;
   }
}