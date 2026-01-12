#pragma once
#include "./audio.h"

namespace ui::types::regions::generable_content {
   constexpr bool audio::empty() const noexcept {
      if (this->music_type)
         return false;
      if (!this->ambient_sounds.empty())
         return false;
      for (auto& item : this->ambient_sounds)
         if (item.sound)
            return false;
      return true;
   }
}