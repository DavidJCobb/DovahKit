#include "base.h"

namespace vulkanDK::scene_entities {
   base::base(base&& o) noexcept {
      *this = std::move(o);
   }
   base& base::operator=(base&& o) noexcept {
      std::swap(this->lifetime.life_state, o.lifetime.life_state);
      std::swap(this->lifetime.sync_state, o.lifetime.sync_state);
      return *this;
   }
}