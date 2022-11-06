#include "editor_mode.h"

namespace dovahkit::subsystems::worldinput::binds::nodes {
   bool editor_mode::check_still_active(core&) const { return false; }
}