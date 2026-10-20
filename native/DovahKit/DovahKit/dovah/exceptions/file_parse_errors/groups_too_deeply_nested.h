#pragma once
#include <vector>
#include "../file_parse_error.h"

namespace dovah::exceptions::file_parse_errors {
   class groups_too_deeply_nested : public file_parse_error {
      public:
         groups_too_deeply_nested(
            const tes::file& file,
            std::vector<size_t>&& group_offsets
         ) :
            file_parse_error(file),
            group_offsets(group_offsets)
         {}

      public:
         std::vector<size_t> group_offsets;
   };
}