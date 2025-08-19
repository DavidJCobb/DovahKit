#pragma once
#include <cstdint>
namespace dovah {
   class form_stub;
}

namespace ui::types::perk_entries {
   struct quest_entry {
      constexpr bool operator==(const quest_entry&) const noexcept = default;
      dovah::form_stub* quest = nullptr;
      uint16_t          stage = 0;
   };
}