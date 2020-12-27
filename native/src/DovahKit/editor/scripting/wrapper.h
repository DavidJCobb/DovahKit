#pragma once
#include <array>
#include <cstdint>
#include "../../helpers/eight_cc.h"
#include "../../dovah/form_stub.h"
#include "../../../Lua/lua.hpp"
#include "classes.h"
#include "util.h"

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
         static constexpr int part_count = 5;
         //
         static luastackchange_t __gc(lua_State* L);
         //
      public:
         //
         // ONLY use these when CREATING a wrapper:
         //
         void append_part(part_type_t signature, uint32_t index = 0);
         void append_part(part_type_t signature, const std::string& name);
         void append_part(part_type_t signature, const char* name);
         void remove_part();
         void into_collection(uint32_t index); // asserts if (is_collection) is false
         void into_collection(const char* name); // asserts if (is_collection) is false

         struct part {
            part_type_t signature = 0;
            uint32_t    index     = 0; // operator== only checks this if the name is empty
            std::string name; // TODO: remove or limit support for this; it prevents us from deleting entries from sequential collections
            //
            bool operator==(const part& other) const noexcept;
            inline bool operator!=(const part& other) const noexcept { return !(*this == other); }
         };

         wrapper() {}
         wrapper(const wrapper& other) {
            *this = other;
            this->lua_key = LUA_NOREF;
         }

         bool is_equal(const wrapper* other) const noexcept;
         bool is_in_same_collection(const wrapper& other) const noexcept;
         bool innermost_part_matches(const wrapper& other) const noexcept; // only checks index
         bool innermost_part_precedes(const wrapper& other) const noexcept; // only checks index. am I a prior sibling of (other)?
         bool innermost_part_has_index() const noexcept;

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
         uint8_t depth         = 0;     // such that this->parts[this->depth - 1] is the innermost part
         bool    is_collection = false; // if this is (true), then parts[depth] has no index or name but rather identifies the collection itself (i.e. allowing Lua to refer to, say, `shout.words` and not just `shout` and `shout.words[2]`)
         std::array<part, part_count> parts;
         
         inline part& last_part() noexcept {
            if (!this->depth)
               return this->parts[0];
            return this->parts[this->depth - 1];
         }
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
         static const std::initializer_list<luaL_Reg> metatable_methods; // subclasses must override this even if they offer no methods
         static const std::initializer_list<luaL_Reg> metatable_getters; // subclasses must override this even if they offer no getters
         static const std::initializer_list<luaL_Reg> metatable_setters; // subclasses must override this even if they offer no setters

         static constexpr std::initializer_list<luaL_Reg> no_functions = {};
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