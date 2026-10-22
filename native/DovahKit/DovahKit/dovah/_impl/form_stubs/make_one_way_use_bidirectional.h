#pragma once
#include "dovah/_passkeys/build_use_info_during_load.h"
#include "dovah/form_stub.h"

namespace dovah::impl::_form_stubs {
   inline void make_one_way_use_bidirectional(_passkeys::build_use_info_during_load, form_stub& user, form_stub& used) {
      const auto& use_for_user = user.outbound[used.form_id];
      auto&       use_for_used = used.inbound[user.form_id];
      use_for_used.other     = &user;
      use_for_used.refcount += use_for_user.refcount;
      use_for_used.flags    |= use_for_user.flags;
   }

   inline void make_all_one_way_uses_bidirectional(_passkeys::build_use_info_during_load, form_stub& user) {
      for (auto& [_, use_for_user] : user.outbound) {
         if (form_stub* used = use_for_user.other) {
            auto& use_for_used = used->inbound[user.form_id];
            use_for_used.other     = &user;
            use_for_used.refcount += use_for_user.refcount;
            use_for_used.flags    |= use_for_user.flags;
         }
      }
   }
}