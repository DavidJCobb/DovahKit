#pragma once
#include <cstdint>
#include <limits>

namespace dovah {
   enum class face_tint_type : uint16_t {
      none,
      lip_color,
      cheek_color_upper,
      eyeliner,
      eyeshadow_upper,
      eyeshadow_lower,
      skin_tone,
      facepaint,
      laugh_lines,
      cheek_color_lower,
      nose,
      chin,
      neck,
      forehead,
      dirt,
      unknown_16,
   };

   using face_tint_index_type = uint16_t;
   
   // In serialized ActorBase data, this appears to be used to indicate that 
   // a tint layer doesn't use a color preset. However, the Creation Kit also 
   // avoids using ID 0, and there seem to be some cases where 0 is treated 
   // like "none."
   //
   // Prefer `index_of_no_face_tint` for deciding what index to serialize when 
   // you want to serialize "none." Prefer `face_tint_index_is_none(...)` for 
   // checking if an index is "none."
   constexpr const face_tint_index_type index_of_no_face_tint = std::numeric_limits<face_tint_index_type>::max();

   constexpr const bool face_tint_index_is_none(face_tint_index_type i) {
      return i == 0 || i == index_of_no_face_tint;
   }
}