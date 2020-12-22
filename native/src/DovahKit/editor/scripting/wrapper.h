#pragma once
#include <cstdint>
#include "../../helpers/eight_cc.h"
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

   using part_type_t = cobb::eight_cc; // signature, e.g. 'FormRoot'
   namespace wrapper_part_types {
   }

   extern bool part_type_uses_name_key(part_type_t);

   class wrapper {
      public:
         static wrapper* wrap_form(dovah::form_stub* s) {
            auto* instance = new wrapper;
            instance->type = wrapper_type::form_data;
            instance->stub = s;
            return instance;
         }

         struct part {
            part_type_t signature = 0;
            union {
               uint32_t    index = 0;
               const char* name;
            };
            //
            bool operator==(const part& other) const noexcept;
            inline bool operator!=(const part& other) const noexcept { return !(*this == other); }
         };

         bool is_equal(const wrapper* other) const noexcept;

         wrapper_type type = wrapper_type::generic;
         int lua_key = LUA_NOREF;
         //
         dovah::form_stub* stub = nullptr;
         uint8_t depth = 0;
         bool    is_collection = false;
         part    parts[5];
   };

   struct wrapper_metatable {
      //
      // Struct for defining metatable names and methods at compile-time.
      //
      private:
         wrapper_metatable() = delete;
      public:
         static constexpr char* superclass_key = nullptr;
         static constexpr char* metatable_key  = nullptr;
         //static luaL_Reg metatable_methods[]; // subclasses must define this, too
   };

   template<typename T> wrapper* wrapper_from_stack(lua_State* L, int pos) noexcept {
      auto* ptr = (wrapper**) editor_script::cast_to_class(L, pos, T::metatable_key);
      if (!ptr)
         return nullptr;
      return *ptr;
   }
   template<typename T> wrapper& get_wrapper_for_thiscall(lua_State* L, int pos = 1) noexcept {
      auto* pself = (wrapper**) editor_script::cast_to_class(L, pos, T::metatable_key);
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