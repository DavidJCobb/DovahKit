#include "results.h"

namespace dovah::tes_file_writing {
   write_results::write_results() {
      this->error.context = detailed_notice::notice_context::file_save;
   }
   detailed_notice& write_results::add_warning() noexcept {
      auto& notice = this->warnings.emplace_back();
      notice.type    = detailed_notice::notice_type::warning;
      notice.context = detailed_notice::notice_context::file_save;
      return notice;
   }
}