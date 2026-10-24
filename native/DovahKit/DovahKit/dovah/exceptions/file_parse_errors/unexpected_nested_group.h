#pragma once
#include "dovah/tes/group_header.h"
#include "../file_parse_error.h"

namespace dovah::exceptions::file_parse_errors {
   //
   // Thrown when groups are nested improperly. This includes:
   // 
   //  - A child group is found within some group of interest that shouldn't have any 
   //    child groups.
   // 
   //  - The multiple layers of child groups used to wrap a cell's child forms have 
   //    inconsistent "parent cell ID" values from one another.
   //
   class unexpected_nested_group : public file_parse_error {
      public:
         unexpected_nested_group(
            const tes::file&         file,
            size_t                   parent_group_offset,
            const tes::group_header& parent_group_header,
            size_t                   nested_group_offset,
            const tes::group_header& nested_group_header
         ) :
            file_parse_error(file),
            parent_group({
               parent_group_header,
               parent_group_offset
            }),
            nested_group({
               nested_group_header,
               nested_group_offset
            })
         {}

      public:
         struct {
            tes::group_header header;
            size_t            offset;
         } parent_group;
         struct {
            tes::group_header header;
            size_t            offset;
         } nested_group;
   };
}