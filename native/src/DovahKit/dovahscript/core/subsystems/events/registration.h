#pragma once
#include <initializer_list>
#include "helpers/singleton.h"
#include "../events.h"

namespace dovahscript::impl::event_registration {
   enum class result {
      success,
      failure,
      no_match,
   };

   class base : cobb::singleton {
      protected:
         using passkey_t = cobb::passkey<dovahscript::core::subsystems::events, base>;

         inline static passkey_t get_passkey() noexcept { return passkey_t(); }
   };
}