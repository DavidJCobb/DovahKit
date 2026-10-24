#pragma once
#include "dovah/tes/group_header.h"
#include "dovah/types/bare_form_id.h"
#include "../file_parse_error.h"
namespace dovah {
   class form_stub;
}

namespace dovah::exceptions::file_parse_errors {
   class topic_child_group_specifies_bad_topic_id : public file_parse_error {
      public:
         enum class problem_type {
            form_does_not_exist,
            form_is_not_a_topic,
            form_id_is_zero,
            record_id_is_out_of_bounds,
            record_id_invalid, // fallback
         };

      public:
         topic_child_group_specifies_bad_topic_id(
            const tes::file&         file,
            size_t                   group_offset,
            const tes::group_header& group_header,
            problem_type             problem,
            bare_form_id             topic_record_id,
            form_stub*               topic_stub
         ) :
            file_parse_error(file),
            group({
               group_offset,
               group_header,
            }),
            topic({
               topic_record_id,
               topic_stub,
            }),
            problem(problem)
         {}

      public:
         struct {
            size_t            offset;
            tes::group_header header;
         } group;
         struct {
            bare_form_id record_id;
            form_stub*   stub = nullptr;
         } topic;
         problem_type problem;
   };
}