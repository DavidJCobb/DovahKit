#pragma once
#include "../../base.h"
#include "../../../wrapper.h"
#include "../../../../dovah/forms/components/papyrus.h"

namespace dovahscript::wrapper_part_types {
   inline constexpr cobb::eight_cc papyrus_property = "PapyProp";
   inline constexpr cobb::eight_cc papyrus_property_array_value = "PapyArrV";
}

namespace dovahscript::wrappers {
   struct papyrus_property : public wrapper_metatable {
      static constexpr const char* superclass_list = { metatable_key };
      static constexpr const char* metatable_key   = "dovah.classes.papyrus_property";
      static constexpr const char* class_name      = "papyrus_property";
      static const std::initializer_list<luaL_Reg> metatable_methods;
      static const std::initializer_list<luaL_Reg> metatable_getters;
      static const std::initializer_list<luaL_Reg> metatable_setters;

      static constexpr bool has_extra_class_setup = true;
      static void extra_class_setup(lua_State* L) noexcept;

      using wrapped_t = dovah::loaded_forms::components::papyrus::script_data::property;
      static wrapped_t* unwrap(wrapper& w, bool must_be_end);
      static wrapped_t* unwrap(wrapper& w, bool must_be_end, uint8_t& next_depth);
   };
}