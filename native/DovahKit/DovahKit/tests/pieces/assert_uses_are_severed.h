#pragma once
#include <array>
#include "dovah/form_stub.h"
#include "./assert_even_in_release.h"

namespace dovahkit::tests::pieces {
   // Assert that `former_user` no longer has uses of any of the forms in 
   // `former_used_list`.
   template<size_t S>
   inline void assert_uses_are_severed(const dovah::form_stub& former_user, const std::array<dovah::form_stub*, S>& former_used_list) {
      for (auto* former_used : former_used_list) {
         for (auto& [_, entry] : former_user.outbound) {
            assert_even_in_release(entry.other != former_used && "Use was not severed (still outbound from user)");
         }
         for (auto& [_, entry] : former_used->inbound) {
            assert_even_in_release(entry.other != &former_user && "Use was not severed (still inbound to used)");
         }
      }
   }
}