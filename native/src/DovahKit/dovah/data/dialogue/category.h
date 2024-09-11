#pragma once
#include <cstdint>
#include <string_view>

namespace dovah::dialogue {
   enum class category : uint8_t {
      topic, // Player Dialogue
      favor_dialogue, // NOT the same as `favors`
      scene,
      combat,
      favors,
      detection,
      service,
      miscellaneous
   };

   constexpr std::string_view internal_name_for_category(category);
}

#include "./category.inl"