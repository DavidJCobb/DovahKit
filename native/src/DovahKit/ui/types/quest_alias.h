#pragma once
#include <cstdint>
#include <QMetaType>

namespace dovah {
   class form_stub;
}

namespace ui::types {
   struct quest_alias {
      static constexpr const uint16_t no_alias_id = 0xFFFF;

      dovah::form_stub* quest    = nullptr;
      uint16_t          alias_id = no_alias_id;

      constexpr bool empty() const { return (quest == nullptr) || (alias_id == no_alias_id); }

      constexpr bool operator==(const quest_alias&) const noexcept = default;
   };
}

// Must be set up at run-time in DovahKitCore:
Q_DECLARE_METATYPE(ui::types::quest_alias)