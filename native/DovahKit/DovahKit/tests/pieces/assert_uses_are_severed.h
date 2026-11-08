#pragma once
#include <array>
#include <cassert>
#include <exception> // std::terminate, for Release
#include "dovah/form_stub.h"

namespace dovahkit::tests::pieces {
   // Assert that `former_user` no longer has uses of any of the forms in 
   // `former_used_list`.
   template<size_t S>
   inline void assert_uses_are_severed(const dovah::form_stub& former_user, const std::array<dovah::form_stub*, S>& former_used_list) {
      for (auto* former_used : former_used_list) {
         for (auto& [_, entry] : former_user.outbound) {
            #if _DEBUG
               assert(entry.other != former_used && "Use was not severed (still outbound from user)");
            #else
               if (entry.other == former_used)
                  std::terminate();
            #endif
         }
         for (auto& [_, entry] : former_used->inbound) {
            #if _DEBUG
               assert(entry.other != &former_user && "Use was not severed (still inbound to used)");
            #else
               if (entry.other == &former_user)
                  std::terminate();
            #endif
         }
      }
   }
}