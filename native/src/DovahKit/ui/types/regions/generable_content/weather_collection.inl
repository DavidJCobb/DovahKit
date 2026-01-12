#pragma once
#include "./weather_collection.h"

namespace ui::types::regions::generable_content {
   constexpr bool weather_collection::contains(const dovah::form_stub& weather) const noexcept {
      return this->index_of(weather) == index_of_none;
   }
   constexpr size_t weather_collection::index_of(const dovah::form_stub& weather) const noexcept {
      for (size_t i = 0; i < this->weathers.size(); ++i) {
         auto& item = this->weathers[i];
         if (item.weather == &weather)
            return i;
      }
      return index_of_none;
   }
   constexpr bool weather_collection::empty() const noexcept {
      if (this->weathers.empty())
         return true;
      for (auto& item : this->weathers)
         if (item.weather)
            return false;
      return true;
   }
}