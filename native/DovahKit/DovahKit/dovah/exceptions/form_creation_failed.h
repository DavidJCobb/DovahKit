#pragma once
#include <cstdint>
#include <optional>
#include <stdexcept>
#include "../requests/create_form.h"
#include "../types/cells/grid_position.h"

namespace dovah {
   class form_stub;
   enum class form_type : uint8_t;
}

namespace dovah::exceptions {
   class form_creation_failed : public std::runtime_error {
      public:
         using error_code = requests::create_form::error_code;

      public:
         form_creation_failed(error_code ec, const requests::create_form&);

         const error_code code;
         struct {
            form_type requested_form_type = {};
            std::optional<types::cells::grid_position> requested_grid_coords;
            form_stub* requested_parent = nullptr;

            // helpers for form duplication errors:
            size_t form_ids_needed  = 1;
            size_t form_ids_missing = 0;
            size_t failure_count    = 1;
         } details;
   };
}