#pragma once
#include "lu/integral_character_code.h"
#include "dovah/tes/group_header.h"
#include "../file_parse_error.h"

namespace dovah::exceptions::file_parse_errors {
   class unexpected_nested_group_in_simple_top_group : public file_parse_error {
      public:
      unexpected_nested_group_in_simple_top_group(
            const tes::file&         file,
            size_t                   top_group_offset,
            lu::four_cc              top_group_form_type_signature,
            size_t                   nested_group_offset,
            const tes::group_header& nested_group_header
         ) :
            file_parse_error(file),
            top_group({
               top_group_form_type_signature,
               top_group_offset
            }),
            nested_group({
               nested_group_header,
               nested_group_offset
            })
         {}

      public:
         struct {
            lu::four_cc form_type_signature;
            size_t      offset;
         } top_group;
         struct {
            tes::group_header header;
            size_t            offset;
         } nested_group;
   };
}