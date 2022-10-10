#pragma once
#include <type_traits>
#include "./all_classes.h"
#include "./scene_limits.h"

#include "./concepts/has_frame_culling_data.h"
#include "./concepts/has_frame_drawing_data.h"

namespace vulkanDK::scene_entities {
   using all_types_with_frame_culling_data = all_types::filter_types<[]<typename T>() -> bool {
      return concepts::has_frame_culling_data<T>;
   }>;
   using all_types_with_frame_drawing_data = all_types::filter_types<[]<typename T>() -> bool {
      return concepts::has_frame_drawing_data<T>;
   }>;
   using all_types_that_are_drawn = all_types::filter_types<[]<typename T>() -> bool {
      return T::is_drawn;
   }>;
   using all_types_with_coalesced_vibs = all_types::filter_types < []<typename T>() -> bool {
      return T::is_drawn && T::coalesced_vib_settings.enabled;
   } > ;
   using all_types_with_fixed_length_coalesced_vibs = all_types::filter_types<[]<typename T>() -> bool {
      return all_types_with_coalesced_vibs::contains_type<T> && T::coalesced_vib_settings.is_fixed_size();
   }>;
   using all_types_with_variable_length_coalesced_vibs = all_types::filter_types < []<typename T>() -> bool {
      return all_types_with_coalesced_vibs::contains_type<T> && !T::coalesced_vib_settings.is_fixed_size();
   } > ;
}