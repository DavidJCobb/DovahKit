#include "./abstract_device_handler.h"
#include <algorithm>
#include "../interruption_check.h"

namespace dovahkit::subsystems::worldinput2::devices {
   interruption_check abstract_device_handler::prepare_interruption_check() const {
      interruption_check out = this->_prepare_interruption_check_impl();
      {
         using entry_type = interruption_check::potentially_interrupting_button;
         std::sort(out.buttons.begin(), out.buttons.end(), [](const entry_type& a, const entry_type& b) {
            return a.down_at < b.down_at;
         });
      }
      return out;
   }
}