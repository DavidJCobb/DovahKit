#pragma once
#include "./attached_scripts.h"

namespace dovahkit::subsystems::form_info_cache::cached_data {
   constexpr bool attached_scripts::operator==(const attached_scripts& o) const {
      if (this == &o)
         return true;

      if (this->attached.size() != o.attached.size())
         return false;
      if (this->deleted.size() != o.deleted.size())
         return false;
      if (this->aliases.size() != o.aliases.size())
         return false;

      auto _unordered_eq = [](const auto& list_a, const auto& list_b) -> bool {
         for (auto& a : list_a) {
            bool found = false;
            for (auto& b : list_b) {
               if (a == b) {
                  found = true;
                  break;
               }
            }
            if (!found)
               return false;
         }
         return true;
      };

      if (!_unordered_eq(this->attached, o.attached))
         return false;
      if (!_unordered_eq(this->deleted, o.deleted))
         return false;
      if (!_unordered_eq(this->aliases, o.aliases))
         return false;

      return true;
   }
}