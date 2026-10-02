#pragma once
#include "./file_queue.h"

namespace dovah::tes {
   constexpr bool file_queue::contains(const file_name_type& desired) const {
      return this->index_of(desired) != index_of_none;
   }
   constexpr size_t file_queue::index_of(const file_name_type& desired) const {
      auto it = std::find(this->files.begin(), this->files.end(), desired);
      if (it == this->files.end())
         return index_of_none;
      return std::distance(this->files.begin(), it);
   }
}