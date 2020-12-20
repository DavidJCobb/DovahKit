#include "_build_metatables.h"
#include "form.h"

namespace editor_script::classes {
   void build_all_userdata_class_metatables(lua_State* L) {
      define_userdata_class<_base>(L);
      define_userdata_class<form>(L);
   }
}