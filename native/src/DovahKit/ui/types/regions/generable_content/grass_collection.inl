#pragma once
#include "./grass_collection.h"

namespace ui::types::regions::generable_content {
   constexpr bool grass_collection::empty() const noexcept {
      if (this->grasses.empty())
         return true;
      for (auto& item : this->grasses)
         if (item.grass && item.land_texture)
            return false;
      return true;
   }
}