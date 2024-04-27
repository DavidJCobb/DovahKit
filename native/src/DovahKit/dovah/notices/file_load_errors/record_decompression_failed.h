#pragma once
#include "./base_record_load_error.h"

#include "../_util.define.h"
namespace dovah::notices::file_load_errors {
   class record_decompression_failed : public base_record_load_error {
      public:
         MAKE_ERROR_OVERLOADS;

      public:
         enum class problem_code {
            claimed_size_is_too_huge_to_even_try,
            data_larger_than_expected, // Z_BUFFER_ERROR
            data_corrupt_or_incomplete, // Z_DATA_ERROR
            uncompressed_data_is_not_of_declared_size, // decompression worked but size isn't what we expected
            zlib_memory_error, // Z_MEM_ERROR
            zlib_unknown_error,
         };

      public:
         constexpr record_decompression_failed(problem_code p) : problem(p) {}

         problem_code problem;
   };
}
#include "../_util.undef.h"