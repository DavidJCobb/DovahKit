#pragma once
#include "../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc papyrus_root = "PapyRoot";
}

namespace editor_script::wrappers {
   struct papyrus_root : public wrapper_metatable {
      static constexpr char* superclass_key = metatable_key;
      static constexpr char* metatable_key  = "dovah.classes.papyrus_root";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr char* script_collection_key = "collection<dovah.classes.papyrus_root.scripts>";
      static void build_collection_metatables(lua_State* L);

      using wrapped_t = dovah::loaded_forms::components::papyrus::script_data;
      static wrapped_t* unwrap(wrapper& w, bool must_be_end);
      static wrapped_t* unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth);
   };
}