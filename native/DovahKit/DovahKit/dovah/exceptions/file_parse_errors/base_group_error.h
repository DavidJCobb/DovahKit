#pragma once
#include "../file_parse_error.h"
#include "dovah/tes/group_header.h"

namespace dovah::exceptions::file_parse_errors {
   class base_group_error : public file_parse_error {
      public:
         base_group_error(
            const tes::file&         file,
            size_t                   group_offset,
            const tes::group_header& group_header
         ) :
            file_parse_error(file),
            group_offset(group_offset),
            group_header(group_header)
         {}

      public:
         size_t            group_offset; // offset of the group header within the file as a whole
         tes::group_header group_header;
   };
}