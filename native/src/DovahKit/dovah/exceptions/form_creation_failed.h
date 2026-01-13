#pragma once
#include <cstdint>
#include <stdexcept>
#include "../form_types.h"
#include "./enums/form_creation_error_code.h"

namespace dovah {
   class form_creation_request;
   class form_stub;
}

namespace dovah::exceptions {
   class form_creation_failed : public std::runtime_error {
      public:
         using error_code = form_creation_error_code;

      public:
         form_creation_failed(error_code ec, const form_creation_request&);

         const error_code code;

         struct {
            form_stub* none_stub = nullptr;

            form_type requested_form_type = form_type::none;
            struct {
               int32_t x = 0;
               int32_t y = 0;
            } requested_grid_coords;
            form_stub* requested_parent = nullptr;

            // helpers for form duplication errors:
            size_t form_ids_needed  = 1;
            size_t form_ids_missing = 0;
            size_t failure_count    = 1;
         } details;
   };
}