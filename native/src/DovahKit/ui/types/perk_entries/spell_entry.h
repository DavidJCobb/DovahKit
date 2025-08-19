#pragma once
#include <cstdint>
namespace dovah {
   class form_stub;
}

namespace ui::types::perk_entries {
   struct spell_entry {
      constexpr bool operator==(const spell_entry&) const noexcept = default;
      dovah::form_stub* spell = nullptr;
   };
}