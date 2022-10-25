#include "rendered_nif.h"

namespace vulkanDK {
   void rendered_nif::_on_background_use_ended() {
      if (this->is_marked_for_delete()) {
         delete this;
      }
   }
   void rendered_nif::delete_when_able() {
      this->multi_thread_state.flags |= loading_flag::marked_for_delete;
      if (!this->is_in_background_use()) {
         delete this;
      }
   }
}