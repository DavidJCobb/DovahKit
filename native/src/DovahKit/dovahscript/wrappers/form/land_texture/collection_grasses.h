#pragma once
#include <string_view>
#include "helpers/eight_cc.h"
namespace dovahscript {
   class wrapper;
}
struct lua_State;

namespace dovahscript::wrapper_likes::native_lists {
   struct land_texture_grasses {
      land_texture_grasses() = delete;

      public:
         static constexpr const std::string_view class_name    = "native_list<grass>";
         static constexpr const std::string_view metatable_key = "native_list<dovah.classes.land_texture.grasses>"; 
         static constexpr const cobb::eight_cc   signature     = "LTexGras";

      public:
         static void define_metatable(lua_State*);
         static int push(lua_State*, const wrapper& parent);
   };
}