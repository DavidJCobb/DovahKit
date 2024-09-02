#pragma once
#include "./layer.h"

namespace ui::types::face_tints {
   constexpr const preset* layer::preset_by_index(dovah::face_tint_index_type n) const {
      for (auto& item : this->presets)
         if (item.index == n)
            return &item;
      return nullptr;
   }
   constexpr preset* layer::preset_by_index(dovah::face_tint_index_type n) {
      return const_cast<preset*>(std::as_const(*this).preset_by_index(n));
   }
}
