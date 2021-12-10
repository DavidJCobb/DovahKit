#include "editor_mode.h"

namespace DK3D::binds::nodes {
   bool editor_mode::check_still_active(DK3DInputHandler&) const { return false; }
}