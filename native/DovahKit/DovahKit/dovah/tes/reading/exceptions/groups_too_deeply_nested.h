#pragma once
#include <vector>
#include "./base_file_read_exception.h"

namespace dovah::tes {
   class groups_too_deeply_nested : public base_file_read_exception {
      public:
         groups_too_deeply_nested(
            const file& file,
            std::vector<size_t>&& group_offsets
         ) :
            base_file_read_exception(file),
            group_offsets(group_offsets)
         {}

      public:
         std::vector<size_t> group_offsets;
   };
}