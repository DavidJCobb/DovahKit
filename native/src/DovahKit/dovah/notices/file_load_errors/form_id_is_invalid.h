#pragma once
#include <cstdint>
#include "../base_file_load_error.h"
#include "../../form_types.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class form_id_is_invalid : public base_file_load_error {
      public:
         MAKE_ERROR_OVERLOADS;

      public:
         enum class problem_code {
            unknown,

            missing_master, // this one can only happen if we failed to load a master, which implies that a file was edited between us checking the header and us loading it

            out_of_bounds,

            zero_is_not_allowed,
         };

      public:
         constexpr form_id_is_invalid(problem_code p) : problem(p) {}

         problem_code problem;
         struct {
            uint32_t  local_id = 0;
            form_type type = form_type::none;
         } form;
   };
}
#include "../_util.undef.h"