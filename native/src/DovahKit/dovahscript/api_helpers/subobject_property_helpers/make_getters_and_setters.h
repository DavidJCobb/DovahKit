#pragma once
#include <tuple>
#include "helpers/function_traits.h"
#include "helpers/tuples/for_each_nttp_value.h"
#include "lua.h"
#include "dovahscript/core/subsystems/permissions.h"
#include "dovahscript/wrapper.h"
#include "dovahscript/wrappers/base.h"
#include "./is_property_tuple_type.h"
#include "./property_definition.h"
namespace dovah {
   class form_reference_t;
}

namespace dovahscript::api_helpers::subobject_property_helpers {
   namespace impl {
      template<auto UnwrapFunc, const auto& PropertyDefinition>
      concept is_property_unwrapper = requires {
         requires std::is_invocable_v<std::decay_t<decltype(UnwrapFunc)>, lua_State*>;
         requires impl::is_property_definition_specialization<std::decay_t<decltype(PropertyDefinition)>>;
      };
   }

   // 04/20/2026: MSVC does not properly report concept errors on these functions if they are 
   //             free functions; it blindly assumes they're overloads and then chokes on its 
   //             own dumb assumption. Making them static member functions of a struct, and 
   //             moving the concepts/requirements to the struct, works.
   template<typename Wrapper, auto UnwrapFunc, const auto& PropertyDefinition>
      requires (
         std::is_base_of_v<wrapper_metatable, Wrapper>&&
         impl::is_property_unwrapper<UnwrapFunc, PropertyDefinition>
      )
   struct property_apis {
      property_apis() = delete;

      static int property_getter(lua_State* L) {
         auto& subobject = UnwrapFunc(L);
         PropertyDefinition.push(L, PropertyDefinition.access(subobject));
         return 1;
      }
      static int property_setter(lua_State* L) {
         using stored_type = typename std::decay_t<decltype(PropertyDefinition)>::stored_type;
         using value_type  = typename std::decay_t<decltype(PropertyDefinition)>::value_type;

         core::subsystems::permissions::verify_form_write_permissions();
      
         auto& self      = get_wrapper_for_thiscall<Wrapper>(L);
         auto& subobject = UnwrapFunc(L);
         value_type src;
         {
            auto message = PropertyDefinition.check(L, 2);
            if (!message.empty()) {
               luaL_argerror(L, 2, message.data());
            }
            src = PropertyDefinition.pull(L, 2);
         }
         if constexpr (PropertyDefinition.late_check) {
            PropertyDefinition.late_check(L, *self.form, &subobject, src);
         }
         self.before_edit();
         {
            auto& dst = PropertyDefinition.access(subobject);
            if constexpr (std::is_base_of_v<dovah::form_reference_t, stored_type>) {
               dst.set(*self.stub->form, src);
            } else if constexpr (std::is_base_of_v<dovah::localized_string, stored_type> && std::is_same_v<value_type, std::string_view>) {
               dst = std::string(src); // HACK because there's no assignment operator for string-view and I don't wanna recompile the whole program to add one
            } else {
               dst = src;
            }
         }
         self.after_edit();
         return 0;
      }
   };

   // Use the "extra class setup" functionality for wrapper metatables.
   // Class internals take initializer lists rather than doing things cleanly, so we have to do this nonsense.
   // Just as well, though: it means we can define read-only getters the normal way, and define properties in 
   // bulk as "extra."
   template<typename Wrapper, auto UnwrapFunc, const auto& PropertyDefinitionList>
      requires is_property_tuple_type<std::decay_t<decltype(PropertyDefinitionList)>>
   void setup_extra_getters_and_setters(lua_State* L) {
      cobb::tuples::for_each_nttp_value<
         PropertyDefinitionList,
         []<const auto& Definition>(lua_State* L) [[msvc::forceinline]] [[gnu::always_inline]] {
            auto g = &property_apis<Wrapper, UnwrapFunc, Definition>::property_getter;
            auto s = &property_apis<Wrapper, UnwrapFunc, Definition>::property_setter;
            lua_pushcfunction(L, g);
            lua_setfield(L, -3, Definition.name.data());
            lua_pushcfunction(L, s);
            lua_setfield(L, -2, Definition.name.data());
         }
      >(L);
   }
}