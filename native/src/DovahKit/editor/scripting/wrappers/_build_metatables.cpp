#include "_build_metatables.h"
#include "_all.h"

namespace editor_script {
   void build_all_wrapper_metatables(lua_State* L) {
      using namespace wrappers;
      //
      define_wrapper_metatable<form>(L);
   }
}