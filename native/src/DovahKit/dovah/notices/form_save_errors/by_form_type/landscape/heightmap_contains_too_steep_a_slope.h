#pragma once
#include <cstdint>
#include <limits>
#include <utility>
#include "../../../base_form_save_error.h"

#include "../../../_util.define.h"
namespace dovah::notices::form_save_errors::by_type::landscape {
   class heightmap_contains_too_steep_a_slope : public base_form_save_error {
      public:
         MAKE_ERROR_OVERLOADS;
      public:
         static constexpr const std::pair<float, float> allowed_height_range = { // min, max
            std::numeric_limits<int8_t>::min() * 8.0F,
            std::numeric_limits<int8_t>::max() * 8.0F
         };

      public:
         constexpr heightmap_contains_too_steep_a_slope(
            form_stub& subject,
            size_t vertex_index_a,
            size_t vertex_index_b,
            float vertex_height_a,
            float vertex_height_b
         )
         :
            base_form_save_error(subject),
            vertex_index_a(vertex_index_a),
            vertex_index_b(vertex_index_b),
            vertex_height_a(vertex_height_a),
            vertex_height_b(vertex_height_b)
         {}

         size_t vertex_index_a;
         size_t vertex_index_b;
         float  vertex_height_a;
         float  vertex_height_b;
   };
}
#include "../../../_util.undef.h"