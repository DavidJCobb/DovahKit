#pragma once
#include "helpers/vector3.h"
#include "../navmesh.h"
#include "dovahscript/lua_classes/vector3.h"

namespace dovahscript::wrappers {
   struct navmesh_vertex : public wrapper_metatable {
      static constexpr string_list_t superclass_list = { metatable_key, dovahscript::lua_classes::vector3::metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh_vertex";
      static constexpr const char*   class_name      = "navmesh_vertex";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = cobb::vector3<float>;
   };
}