#include "_import_all.h"
#include "benchmark.h"
#include "euler.h"
#include "matrix3x3.h"
#include "quaternion.h"
#include "vector2.h"
#include "vector3.h"

namespace dovahscript::lua_classes {
   extern void import_all(lua_State* L) {
      benchmark::setup(L);
      euler::setup(L);
      matrix3x3::setup(L);
      quaternion::setup(L);
      vector2::setup(L);
      vector3::setup(L);
   }
}