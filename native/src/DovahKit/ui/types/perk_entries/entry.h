#pragma once
#include <variant>
#include "./entry_point_entry.h"
#include "./quest_entry.h"
#include "./spell_entry.h"
namespace dovah::loaded_forms {
   namespace structs {
      struct perk_effect;
   }
   class Perk;
}

namespace ui::types::perk_entries {
   struct entry {
      constexpr bool operator==(const entry&) const noexcept = default;
      using quest_entry = perk_entries::quest_entry;
      using spell_entry = perk_entries::spell_entry;
      using entry_point_entry = perk_entries::entry_point_entry;

      uint8_t priority = 0;
      uint8_t rank     = 0;
      std::variant<
         quest_entry,
         spell_entry,
         entry_point_entry
      > data;

      static std::vector<entry> pull_list_from_backend(const dovah::loaded_forms::Perk&);

      void append_into_backend(dovah::loaded_forms::Perk&) const;

      // Returns `true` if any references were severed, or `false` otherwise.
      bool sever_references_to(const dovah::form_stub&);
   };
}