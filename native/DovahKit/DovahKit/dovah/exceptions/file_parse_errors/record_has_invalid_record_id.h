#pragma once
#include "./base_record_error.h"

namespace dovah::exceptions::file_parse_errors {
   class record_has_invalid_record_id : public base_record_error {
      public:
         enum class problem_type {
            out_of_bounds,
            zero_is_not_allowed,
         };

      public:
         record_has_invalid_record_id(
            const tes::file&          file,
            size_t                    record_offset,
            const tes::record_header& record_header,
            problem_type              problem
         ) :
            base_record_error(file, record_offset, record_header),
            problem(problem)
         {}

         problem_type problem;
   };
}