#pragma once
#include <cstdint>
#include "../../../Lua/lua.hpp"
#include "classes.h"

namespace dovah {
   class form_stub;
}

namespace editor_script {
   enum class wrapper_type {
      generic,
      form_data, // a form    or some object within a form
      ui,        // a QWidget or some data   within a QWidget
   };

   class wrapper {
      protected:
         virtual bool _is_equal_impl(const wrapper* other) const noexcept = 0;
      public:
         static constexpr char* superclass_key = nullptr;
         static constexpr char* metatable_key  = nullptr;

         bool is_equal(const wrapper* other) const noexcept;

         wrapper_type type = wrapper_type::generic;
         int lua_key = LUA_NOREF;
         //
         dovah::form_stub* stub = nullptr;
   };

   template<typename T> T* wrapper_from_stack(lua_State* L, int pos) noexcept {
      auto* ptr = (T**) editor_script::cast_to_class(L, pos, T::metatable_key);
      if (!ptr)
         return nullptr;
      return *ptr;
   }
   template<typename T> T& get_wrapper_for_thiscall(lua_State* L, int pos = 1) noexcept {
      auto* pself = (T**) editor_script::cast_to_class(L, pos, T::metatable_key);
      if (pself == nullptr) {
         luaL_error(L, "function called with bad self (expected %s)", T::metatable_key);
      }
      __assume(pself != nullptr);
      auto* self = *pself;
      if (!self) {
         luaL_error(L, "cannot call function; underlying object is missing somehow?", T::metatable_key);
      }
      __assume(self != nullptr);
      return *self;
   }

   template<typename T> void define_wrapper_metatable(lua_State* L) noexcept {
      if (!T::metatable_key)
         return;
      if (is_class_defined(L, T::metatable_key))
         return;
      define_class(L, T::metatable_key, T::superclass_key, T::metatable_methods);
   }
}