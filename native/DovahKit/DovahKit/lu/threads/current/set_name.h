#pragma once
#include <system_error>
#include "../../strings/zstring_view.h"

namespace lu::threads::current {
   extern void set_name(zstring_view);

   extern void set_name(zstring_view, std::error_code&);
}
