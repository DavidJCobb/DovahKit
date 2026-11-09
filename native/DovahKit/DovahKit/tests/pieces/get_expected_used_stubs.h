#pragma once
#include <array>
#include "dovah/form_stub.h"
#include "dovah/types/bare_form_id.h"
#include "./assert_even_in_release.h"

namespace dovahkit::tests::pieces {
   // Assert that all specified form IDs are used by the `user` stub, and return 
   // the form-stub pointers for those IDs.
   template<size_t S>
   std::array<dovah::form_stub*, S> get_expected_used_stubs(const dovah::form_stub& user, const std::array<dovah::bare_form_id, S>& ids) {
      std::array<dovah::form_stub*, S> stubs = {};
      for(size_t i = 0; i < S; ++i) {
         auto id    = ids[i];
         bool found = false;
         assert_even_in_release(id != 0 && "You need to change the `S` template parameter. If it's longer than the array you pass in, zeroes will make it in.");
         for (auto& [entry_id, entry] : user.outbound) {
            if (entry_id == id) {
               stubs[i] = entry.other;
               found    = true;
               break;
            }
         }
         assert_even_in_release(!!found);
      }
      return stubs;
   }
}