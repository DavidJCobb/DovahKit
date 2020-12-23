#pragma once
#include <cstdint>
#include "../../helpers/eight_cc.h"
#include "../../dovah/form_stub.h"
#include "../../../Lua/lua.hpp"
#include "classes.h"

namespace editor_script {
   enum class wrapper_type {
      generic,
      form_data, // a form    or some object within a form
      ui,        // a QWidget or some data   within a QWidget
   };

   using part_type_t = cobb::eight_cc; // signature, e.g. 'FormRoot'
   namespace wrapper_part_types {
   }
   // see wrapper_util.h for stuff related to this

   class wrapper final {
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
         void load_form();
         void mark_form_as_edited();

         template<typename c> c* get_loaded_form_data() {
            this->load_form();
            return (c*)(dovah::loaded_forms::Form*)this->form;
         }

         wrapper_type type = wrapper_type::generic;
         int lua_key = LUA_NOREF;
         //
         dovah::form_stub* stub = nullptr;
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> form;
         uint8_t depth = 0;
         bool    is_collection = false; // if this is (true), then parts[depth] has no index or name but rather identifies the collection itself (i.e. allowing Lua to refer to, say, `shout.words` and not just `shout` and `shout.words[2]`)
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
         static constexpr char* metatable_key  = "dovah.classes.!base";
         static luaL_Reg metatable_methods[]; // subclasses must override this even if they offer no methods
         static luaL_Reg metatable_getters[]; // subclasses must override this even if they offer no getters
         static luaL_Reg metatable_setters[]; // subclasses must override this even if they offer no setters
   };

   template<typename T> wrapper* wrapper_from_stack(lua_State* L, int pos) noexcept {
      auto* ptr = (wrapper*) editor_script::cast_to_class(L, pos, T::metatable_key);
      return ptr;
   }
   template<typename T> wrapper& get_wrapper_for_thiscall(lua_State* L, int pos = 1) noexcept {
      auto* self = (wrapper*) editor_script::cast_to_class(L, pos, T::metatable_key);
      if (self == nullptr) {
         luaL_error(L, "function called with bad self (expected %s)", T::metatable_key);
      }
      __assume(self != nullptr);
      return *self;
   }

   template<typename T> void define_wrapper_metatable(lua_State* L) noexcept {
      if (!T::metatable_key)
         return;
      if (is_class_defined(L, T::metatable_key))
         return;
      define_class(L, T::metatable_key, T::superclass_key, T::metatable_methods, T::metatable_getters, T::metatable_setters);
   }
}