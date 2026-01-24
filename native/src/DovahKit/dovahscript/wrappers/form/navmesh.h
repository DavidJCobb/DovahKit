#pragma once
#include "form.h"

namespace dovah::loaded_forms {
   class Navmesh;
}

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc navmesh_triangle  = "NavmTria";
   inline constexpr cobb::eight_cc navmesh_vertex    = "NavmVrtx";
   inline constexpr cobb::eight_cc navmesh_door_link = "NavmDoor";
   inline constexpr cobb::eight_cc navmesh_edge_link = "NavmEdge";
   inline constexpr cobb::eight_cc navmesh_triangle_cover_edge = "NavmTriC";
}

namespace dovahscript::wrappers {
   struct navmesh : public form {
      static constexpr string_list_t superclass_list = { metatable_key };
      static constexpr const char*   metatable_key   = "dovah.classes.navmesh";
      static constexpr const char*   class_name      = "navmesh";
      static method_list_t metatable_methods;
      static method_list_t metatable_getters;
      static method_list_t metatable_setters;

      using wrapped_type = dovah::loaded_forms::Navmesh;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L);
   };
}