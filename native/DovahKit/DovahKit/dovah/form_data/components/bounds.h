#pragma once
#include <cstdint>
#include "lu/bytestreams/customization/serialized_bytecount.h"
#include "lu/integral_character_code.h"
#include "lu/math/types/vec3.h"
namespace dovah::tes {
   class subrecord_view;
}

namespace dovah::form_data::components {
   class bounds {
      public:
         static constexpr const lu::four_cc subrecord_signature = 'OBND';

         using point_type = lu::math::vec3<int16_t>;

      public:
         point_type min;
         point_type max;

      public:
         constexpr point_type dimensions() const noexcept {
            return this->max - this->min;
         }
         constexpr bool is_zero() const noexcept {
            return ((int)min.x + min.y + min.z + max.x + max.y + max.z) == 0;
         }
         constexpr float volume() const noexcept {
            auto d = this->dimensions();
            return (float)d.x * (float)d.y * (float)d.z;
         }
         
         bool stream(tes::subrecord_view& subrecord);
         void unchecked_stream(tes::subrecord_view& subrecord);
   };
}

namespace lu::bytestreams {
   template<typename Stream>
   struct serialized_bytecount<Stream, dovah::form_data::components::bounds> {
      private:
         using value_type = dovah::form_data::components::bounds;
      public:
         static constexpr const size_t value = sizeof(value_type::point_type::value_type) * 6;
   };
}
