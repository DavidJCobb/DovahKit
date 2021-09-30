#pragma once
#include "../../lua.h"
#include "../../helpers/rotation.h"

namespace dovahscript::api_helpers {
   extern bool pull_rotation(lua_State*, int index, cobb::euler& result); // assumes that euler input is in degrees; converts to radians
}