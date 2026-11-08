#pragma once
#include <array>
#include "dovah/form_stub.h"
#include "dovah/localization/use_node.h"
#include "dovah/tes/file.h"
#include "../types/localized_string_descriptor.h"
#include "./assert_even_in_release.h"

namespace dovahkit::tests::pieces {
   template<size_t S>
   inline void assert_uses_strings(
      const dovah::form_stub& user,
      const std::array<types::localized_string_descriptor, S>& strings
   ) {
      const auto string_uses = user.get_string_uses();
      for (const auto& string : strings) {
         const auto& [tes_filename, category, id] = string;

         bool found = false;
         for (const auto* use_node : string_uses) {
            assert_even_in_release(!!use_node);
            auto& used = use_node->used;
            if (used.id != id)
               continue;
            if (used.category != category)
               continue;
            if (used.tes_file->path().filename().string() != tes_filename)
               continue;
            found = true;
         }
         assert_even_in_release(!!found && "String is not used");
      }
   }
}