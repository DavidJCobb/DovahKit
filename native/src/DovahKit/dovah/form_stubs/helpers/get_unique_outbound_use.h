#pragma once
#include "../../form_stub.h"
#include "../../use_info_entry.h"

namespace dovah::form_stub_helpers {
   template<use_info_entry::flag::type Flag>
   form_stub* get_unique_outbound_use(const form_stub& user) {
      for (auto& pair : user.outbound) {
         auto& entry = pair.second;
         if (entry.flags & Flag)
            return entry.other;
      }
      return nullptr;
   }
}