#pragma once
#include <array>
#include <cstdint>

namespace dovahkit::subsystems::game_localized_strings {
   // pairs are narrow-encoded codepoint to Unicode codepoint
   using narrow_character_set = std::array<uint16_t, 256>;
}