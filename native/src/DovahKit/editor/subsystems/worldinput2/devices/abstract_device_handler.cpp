#include "./abstract_device_handler.h"
#include <algorithm>
#include "../interruption_check.h"

namespace dovahkit::subsystems::worldinput2::devices {
   const raycast_result_per_key& abstract_device_handler::get_raycast_result(timestamp_t now, const inputs::button& b) {
      for (const auto& item : this->raycast_results) {
         if (item.button == b) {
            return item;
         }
      }
      raycast_result res = static_assert(false, "TODO: Ask Worldinput to ask Worldedit to make a raycast so we can store the results.");
      auto& stored = this->raycast_results.emplace_back(res);
      stored.when   = now;
      stored.button = b;
      return stored;
   }
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