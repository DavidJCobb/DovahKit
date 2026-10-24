#pragma once
#include <cstdint>
#include "dovah/tes/group_header.h"
#include "dovah/types/bare_form_id.h"
#include "../file_parse_error.h"
namespace dovah {
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah::exceptions::file_parse_errors {
   class child_form_group_specifies_bad_parent_id : public file_parse_error {
      public:
         enum class problem_type {
            form_does_not_exist,
            form_is_the_wrong_type,
            form_id_is_zero,
            record_id_is_out_of_bounds,
            record_id_invalid, // fallback
         };

      public:
         child_form_group_specifies_bad_parent_id(
            const tes::file&         file,
            size_t                   group_offset,
            const tes::group_header& group_header,
            problem_type             problem,
            form_type                parent_desired_form_type,
            bare_form_id             parent_record_id,
            form_stub*               parent_stub
         ) :
            file_parse_error(file),
            group({
               group_offset,
               group_header,
            }),
            parent({
               parent_desired_form_type,
               parent_record_id,
               parent_stub,
            }),
            problem(problem)
         {}

      public:
         struct {
            size_t            offset;
            tes::group_header header;
         } group;
         struct {
            form_type    desired_form_type;
            bare_form_id record_id;
            form_stub*   stub = nullptr;
         } parent;
         problem_type problem;
   };
}