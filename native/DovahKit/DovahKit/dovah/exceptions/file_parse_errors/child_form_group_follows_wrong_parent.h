#pragma once
#include "dovah/tes/group_header.h"
#include "../file_parse_error.h"
namespace dovah {
   class form_stub;
}

namespace dovah::exceptions::file_parse_errors {
   //
   // A parent form should be followed immediately by the GRUP that encapsulates 
   // its child forms. This error is thrown if such a GRUP specifies a parent 
   // record ID that doesn't match the immediate preceding record. (By implication 
   // that also means it happens if the GRUP doesn't follow any preceding record.)
   //
   class child_form_group_follows_wrong_parent : public file_parse_error {
      public:
         child_form_group_follows_wrong_parent(
            const tes::file&         file,
            size_t                   group_offset,
            const tes::group_header& group_header,
            form_stub*               expected_parent_stub // i.e. the stub generated from the preceding record
         ) :
            file_parse_error(file),
            group({
               group_offset,
               group_header,
            }),
            expected_parent_stub(expected_parent_stub)
         {}

      public:
         struct {
            size_t            offset;
            tes::group_header header;
         } group;
         form_stub* expected_parent_stub;
   };
}