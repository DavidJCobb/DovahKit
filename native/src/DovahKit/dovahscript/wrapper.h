#pragma once
#include <array>
#include <cstdint>
#include <QWidget>
#include "../../helpers/lua/error.h"
#include "../../helpers/eight_cc.h"
#include "../../dovah/form_stub.h"
#include "../../lua.h"
#include "core/classes.h"

class  CanvasWidgetEntity;
class  DovahscriptCanvasWidgetLayerData;
class  QButtonGroup;
struct ObservableStandardItemModelObserver;
namespace cobb::qt::ini {
   class Setting;
}
namespace dovahscript {
   class DovahscriptResource;
}

namespace dovahscript {
   enum class wrapper_type {
      undefined,
      form,
      widget,
      model_observer,
      button_group,
      lua_managed_resource,
      canvas_entity,
      canvas_layer_data,
      ini_setting,
   };

   using part_type_t = cobb::eight_cc; // signature, e.g. 'FormRoot'
   namespace wrapper_part_types {
   }

   class wrapper final {
      public:
         static constexpr int part_count = 5;
         
         static int __close(lua_State* L);
         static int __gc(lua_State* L);
         
      protected:
         //
         // Teardown tasks should be performed here, not in the destructor. The wrapper system works by 
         // having Lua APIs create a wrapper on the stack and pass it to the VM to then be copied into 
         // the VM internals; the wrapper on the stack is then destroyed, but it's the wrapper in the VM 
         // that actually matters, and it's only the latter that should run teardown tasks.
         //
         void teardown(bool is_toclose = false);

      public:
         //
         // ONLY use these when CREATING a wrapper:
         //
         void append_part(part_type_t signature, uint32_t index = 0);
         void remove_part();
         void into_collection(uint32_t index); // asserts if (is_collection) is false

         //
         // Should only be called by the internal userdata singleton.
         //
         void _on_pushed();

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
         void error_if_wrong_form_type(lua_State* L, int arg_index, dovah::form_type, bool loose = true);

         template<typename c> c* get_loaded_form_data() {
            this->load_form();
            return (c*)(dovah::loaded_forms::Form*)this->form;
         }

         wrapper_type type    = wrapper_type::undefined;
         int          lua_key = LUA_NOREF;
         union {
            void* pertinent_pointer = nullptr;
            //
            QButtonGroup*                        button_group;
            CanvasWidgetEntity*                  canvas_entity;
            DovahscriptCanvasWidgetLayerData*    canvas_layer_data;
            cobb::qt::ini::Setting*              game_ini_setting;
            DovahscriptResource*                 managed_resource;
            ObservableStandardItemModelObserver* model_observer;
            dovah::form_stub*                    stub;
            QWidget*                             widget;
         };
         //
         dovah::loaded_form_ptr<dovah::loaded_forms::Form> form;
         uint8_t depth         = 0;     // such that this->parts[this->depth - 1] is the innermost part
         bool    is_collection = false; // if this is (true), then parts[depth] has no index or name but rather identifies the collection itself (i.e. allowing Lua to refer to, say, `shout.words` and not just `shout` and `shout.words[2]`)
         std::array<part, part_count> parts;
         
         inline part& last_part() noexcept {
            if (!this->depth)
               return this->parts[0];
            return this->parts[this->depth - 1];
         }

         int8_t depth_of(const cobb::eight_cc&) const noexcept;
         bool is_collection_at_depth(uint8_t) const noexcept;
   };

   template<typename T> wrapper* wrapper_from_stack(lua_State* L, int pos) {
      auto* ptr = (wrapper*) dovahscript::classes::cast_to_class(L, pos, T::metatable_key);
      return ptr;
   }
   template<typename T> wrapper& get_wrapper_for_thiscall(lua_State* L, int pos = 1) {
      auto* self = (wrapper*) dovahscript::classes::cast_to_class(L, pos, T::metatable_key);
      if (self == nullptr)
         cobb::lua::error(L, "function called with bad self (expected %s)", T::metatable_key);
      return *self;
   }
}