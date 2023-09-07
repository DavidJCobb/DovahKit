#include "./abstract_device_handler.h"
#include <algorithm>
#include "editor/subsystems/worldedit/core.h"
#include "../interruption_check.h"

namespace dovahkit::subsystems::worldinput::devices {
   const raycast_result_per_key& abstract_device_handler::get_raycast_result(timestamp_t now, const inputs::button& b) {
      for (const auto& item : this->raycast_results.per_button) {
         if (item.button == b) {
            return item;
         }
      }
      auto& stored = this->raycast_results.per_button.emplace_back(this->get_per_frame_raycast_result());
      stored.when   = now;
      stored.button = b;
      return stored;
   }

   const raycast_result& abstract_device_handler::get_per_frame_raycast_result() {
      auto& rr = this->raycast_results.this_frame;
      if (!rr.has_value()) {
         const auto& worldedit = dovahkit::subsystems::worldedit::core::get();
         //
         rr = worldedit.raycast_at(this->pointer_position.x(), this->pointer_position.y());
      }
      return rr.value();
   }

   void abstract_device_handler::discard_raycast_results_for(const dovah::form_stub& stub) {
      if (auto& list = this->raycast_results.per_button; !list.empty()) {
         //
         // Clear raycast results for any buttons that were released on the previous frame. 
         // (We don't clear a button's results on the frame it's released, because that 
         // would prevent Press and Long Press binds from checking the results when they're 
         // about to activate.)
         //
         list.erase(
            std::remove_if(
               list.begin(),
               list.end(),
               [this, &stub](const auto& result) -> bool {
                  return result.target_info.form == &stub;
               }
            ),
            list.end()
         );
      }
      if (auto& opt = this->raycast_results.this_frame; opt.has_value()) {
         if (opt.value().target_info.form == &stub) {
            opt = {};
         }
      }
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