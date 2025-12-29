#pragma once
#include "../../form_stub.h"

namespace dovah::form_stub_helpers {
   constexpr bool is_used_by(const form_stub& used, const form_stub& user) {
      if (used.inbound.size() < user.outbound.size()) {
         for (const auto& [_, use] : used.inbound)
            if (use.other == &user)
               return true;
      } else {
         for (const auto& [_, use] : user.outbound)
            if (use.other == &used)
               return true;
      }
      return false;
   }
}