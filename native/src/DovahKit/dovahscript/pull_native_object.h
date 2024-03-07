#pragma once
#include "../lua.h"
#include "../dovah/core.h"

namespace dovah {
   class form_stub;
}

namespace dovahscript {
   [[nodiscard]] extern dovah::form_stub* pull_form_stub_argument(lua_State*, int arg, dovah::form_type ft = dovah::form_type::none);
}