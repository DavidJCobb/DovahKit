#pragma once
#include "../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace editor_script::wrapper_part_types {
   inline constexpr cobb::eight_cc papyrus_script = "PapyScri";
}

namespace editor_script::wrappers {
   struct papyrus_script : public wrapper_metatable {
      static constexpr const char* superclass_key = metatable_key;
      static constexpr const char* metatable_key  = "dovah.classes.papyrus_script";
      static constexpr const char* class_name     = "papyrus_script";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      using wrapped_t = dovah::loaded_forms::components::papyrus::script_data::script;
      static wrapped_t* unwrap(wrapper& w, bool must_be_end);
      static wrapped_t* unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth);

      static constexpr const char* property_collection_key = "collection<dovah.classes.scripts.properties>";
      static void build_collection_metatables(lua_State* L);
   };
}