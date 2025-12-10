#pragma once
#include "./node_parent.h"

namespace dovah::datastores::impl::camera_paths {
   constexpr size_t node_parent::index_of_child(const node& n) const noexcept {
      const auto& list = this->children;
      const auto  size = list.size();
      for (size_t i = 0; i < size; ++i)
         if (list[i] == &n)
            return i;
      return index_of_none;
   }
}