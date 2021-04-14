#pragma once
#include <array>
#include <cstdint>
#include <QWidget>
#include "../../helpers/eight_cc.h"
#include "../../dovah/form_stub.h"
#include "../../../Lua/lua.hpp"
#include "classes.h"
#include "util.h"

class ObservableStandardItemModelObserver;

namespace editor_script {
   enum class wrapper_type {
      undefined,
      form_data, // a form    or some object within a form
      ui,        // a QWidget or some data   within a QWidget
      ui_model_item,
   };

   using part_type_t = cobb::eight_cc; // signature, e.g. 'FormRoot'
   namespace wrapper_part_types {
   }
   // see wrapper_util.h for stuff related to this

   class wrapper final {
      public:
         static constexpr int part_count = 5;
         
         static luastackchange_t __gc(lua_State* L);
         
      protected:
         //
         // Teardown tasks should be performed here, not in the destructor. The wrapper system works by 
         // having Lua APIs create a wrapper on the stack and pass it to the VM to then be copied into 
         // the VM internals; the wrapper on the stack is then destroyed, but it's the wrapper in the VM 
         // that actually matters, and it's only the latter that should run teardown tasks.
         //
         void teardown();

      public:
         //
         // ONLY use these when CREATING a wrapper:
         //
         void append_part(part_type_t signature, uint32_t index = 0);
         void remove_part();
         void into_collection(uint32_t index); // asserts if (is_collection) is false

         struct part {
            part_type_t signature = 0;
            uint32_t    index     = 0;
            bool        noncontiguous = false;
            //
            bool operator==(const part& other) const noexcept;
            inline bool operator!=(const part& other) const noexcept { return !(*this == other); }
         };

         wrapper() {}
         wrapper(const wrapper& other) {
            *this = other;
            this->lua_key = LUA_NOREF;
         }
         ~wrapper();

         //
         // There are certain objects that we actually *don't* want to provide to scripts, like none-stubs 
         // and GMST form stubs. It's easiest to just check for that stuff at the very last possible second, 
         // when actually sending (or not) the wrapper to Lua.
         //
         bool should_expose_to_script() const noexcept;

         bool is_descendant_of(const wrapper& other) const noexcept;
         bool is_equal(const wrapper* other) const noexcept;
         bool is_in_same_collection(const wrapper& other) const noexcept;

         void load_form();

         //
         // Call these functions before and after modifying data on a form, to ensure that DovahKitCore 
         // emits the correct signals to the program UI and to ensure that the form stub is properly 
         // flagged as edited.
         //
         void before_edit();
         void after_edit();

         //
         // Emits a Lua error if the wrapped form's type isn't the expected type. You should set the 
         // "loose" argument to true if allowing a mismatched type would be technically incorrect but 
         // wouldn't break the file, or false if it would break the file. As an example, setting a 
         // shout's first word to point to an ACTI would be incorrect, but the file would still be 
         // readable; however, setting the owner of an inventory item to something that is not a FACT 
         // or NPC_ would actually make it impossible to read the next four bytes of the inventory 
         // data, as those depend on the form type.
         //
         // We may at some point in the future allow scripts to disable "loose" form type errors. 
         // Maybe.
         //
         void error_if_wrong_form_type(lua_State* L, int arg_index, dovah::form_type_t, bool loose = true);

         template<typename c> c* get_loaded_form_data() {
            this->load_form();
            return (c*)(dovah::loaded_forms::Form*)this->form;
         }

         wrapper_type type = wrapper_type::undefined;
         int lua_key = LUA_NOREF;
         //
         dovah::form_stub* stub = nullptr;
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> form;
         uint8_t depth         = 0;     // such that this->parts[this->depth - 1] is the innermost part
         bool    is_collection = false; // if this is (true), then parts[depth] has no index or name but rather identifies the collection itself (i.e. allowing Lua to refer to, say, `shout.words` and not just `shout` and `shout.words[2]`)
         std::array<part, part_count> parts;
         //
         QWidget* widget = nullptr;
         //
         ObservableStandardItemModelObserver* model_observer = nullptr;
         
         inline part& last_part() noexcept {
            if (!this->depth)
               return this->parts[0];
            return this->parts[this->depth - 1];
         }

         //
         // Gets the pertinent pointer for the wrapper. Lua can use this pointer as a "light userdata," 
         // to serve as a unique key for the wrapper. This, of course, means that the pointer should 
         // never be used if it's nullptr.
         //
         inline void* get_pertinent_pointer() const noexcept {
            assert(this->type != wrapper_type::undefined && "Why are you trying to get the pertinent pointer for a wrapper before it's been properly configured?");
            switch (this->type) {
               case wrapper_type::form_data:
                  return this->stub;
               case wrapper_type::ui:
                  return this->widget;
               case wrapper_type::ui_model_item:
                  return this->model_observer;
            }
            return nullptr;
         }

         int8_t depth_of(const cobb::eight_cc&) const noexcept;
         bool is_collection_at_depth(uint8_t) const noexcept;
   };

   struct wrapper_metatable {
      //
      // Struct for defining metatable names and methods at compile-time.
      //
      private:
         wrapper_metatable() = delete;
      public:
         static constexpr const char* superclass_key = nullptr;
         static constexpr const char* metatable_key  = "dovah.classes.!base";
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