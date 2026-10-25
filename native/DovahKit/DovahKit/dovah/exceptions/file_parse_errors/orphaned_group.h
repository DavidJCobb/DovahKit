#pragma once
#include "dovah/tes/group_header.h"
#include "../file_parse_error.h"

namespace dovah::exceptions::file_parse_errors {
   // Groups of this type should not be top-level.
   class orphaned_group : public file_parse_error {
      public:
         orphaned_group(
            const tes::file&         file,
            size_t                   group_offset,
            const tes::group_header& group_header
         ) :
            file_parse_error(file),
            group({
               group_header,
               group_offset
            })
         {}

      public:
         struct {
            tes::group_header header;
            size_t            offset;
         } group;
   };
}