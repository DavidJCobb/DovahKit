#pragma once
#include <cstdint>
#include "../../../Lua/lua.hpp"
#include "../classes.h"

namespace dovah {
   class form_stub;
}

namespace editor_script::classes {
   enum class userdata_base_type {
      generic,
      form_data,
      ui,
   };

   class _base {
      protected:
         virtual bool _is_equal_impl(const _base* other) const noexcept = 0;
      public:
         static constexpr char* superclass_key = nullptr;
         static constexpr char* metatable_key  = "dovah.classes._base";
         static luaL_Reg metatable_methods[];

         _base();
         ~_base();

         bool is_equal(const _base* other) const noexcept;

         userdata_base_type type = userdata_base_type::generic;
         uint32_t refcount = 0;
         //
         dovah::form_stub* stub = nullptr;
   };

   template<typename T> T* class_from_stack(lua_State* L, int pos) noexcept {
      auto* ptr = (T**) editor_script::cast_to_class(L, pos, T::metatable_key);
      if (!ptr)
         return nullptr;
      return *ptr;
   }

   template<typename T> void define_userdata_class(lua_State* L) noexcept {
      if (!T::metatable_key)
         return;
      if (is_class_defined(L, T::metatable_key))
         return;
      define_class(L, T::metatable_key, T::superclass_key, T::metatable_methods);
   }
}