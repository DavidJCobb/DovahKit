#pragma once
#include <string>
#include <string_view>
#include <type_traits>
#include "helpers/function_traits.h"
struct lua_State;

namespace dovahscript::api_helpers::subobject_property_helpers {
   namespace impl {
      template<typename T>
      concept field_access_function = requires {
         typename cobb::function_traits<T>::return_type;
         requires cobb::function_traits<T>::arg_count == 1;
         requires std::is_lvalue_reference_v<typename cobb::function_traits<T>::template arg_type<0>>;
         requires std::is_lvalue_reference_v<typename cobb::function_traits<T>::return_type>;
      };

      template<typename T>
      concept lua_check_function = requires {
         requires std::is_invocable_v<T, lua_State*, int>;
         typename cobb::function_traits<T>::return_type;
         requires (
            std::is_same_v<typename cobb::function_traits<T>::return_type, std::string> ||
            std::is_same_v<typename cobb::function_traits<T>::return_type, std::string_view> ||
            std::is_same_v<typename cobb::function_traits<T>::return_type, const char*>
         );
      };

      template<typename T>
      concept lua_pull_function = requires {
         requires std::is_invocable_v<T, lua_State*, int>;
         typename cobb::function_traits<T>::return_type;
         requires !std::is_same_v<typename cobb::function_traits<T>::return_type, void>;
      };

      template<typename T>
      concept lua_push_function = requires {
         requires cobb::function_traits<T>::arg_count == 2;
         requires std::is_same_v<typename cobb::function_traits<T>::template arg_type<0>, lua_State*>;
      };

      template<
         typename AccessFunc,
         typename CheckFunc,
         typename PullFunc,
         typename PushFunc
      > concept valid_subobject_property_definition = requires {
         requires field_access_function<AccessFunc>;
         requires lua_check_function<CheckFunc>;
         requires lua_pull_function<PullFunc>;

         requires lua_push_function<PushFunc>;
         requires std::is_invocable_v<PushFunc, lua_State*, const std::decay_t<typename cobb::function_traits<AccessFunc>::return_type>&>;
      };
   }
   
   template<typename AccessFunc, typename CheckFunc, typename PullFunc, typename PushFunc>
      requires impl::valid_subobject_property_definition<AccessFunc, CheckFunc, PullFunc, PushFunc>
   struct property_definition {
      public:
         using accessor_function_type  = AccessFunc; // stored_type&  func(subobject_type&);
         using lua_check_function_type = CheckFunc;  // <string-like> func(lua_State*, int stack_pos);
         using lua_pull_function_type  = PullFunc;   // value_type    func(lua_State*, int stack_pos);
         using lua_push_function_type  = PushFunc;   // void          func(lua_State*, const stored_type&);
         using subobject_type          = std::decay_t<typename cobb::function_traits<accessor_function_type>::template arg_type<0>>;
         using value_type              = typename cobb::function_traits<lua_pull_function_type>::return_type;
         using stored_type             = std::decay_t<typename cobb::function_traits<accessor_function_type>::return_type>;

      public:
         std::string_view        name;
         accessor_function_type  access;
         lua_check_function_type check; // return an error string, or empty if the value is well-formed. must keep the Lua stack balanced.
         lua_pull_function_type  pull;  // pull a value out of Lua
         lua_push_function_type  push;  // push a value into Lua

         std::optional<value_type> default_value;
         bool                      treat_nil_as_unchanged = false;
   };

   namespace impl {
      template<typename T>
      concept is_property_definition_specialization = requires {
         typename T::accessor_function_type;
         typename T::lua_check_function_type;
         typename T::lua_pull_function_type;
         typename T::lua_push_function_type;
         requires std::is_base_of_v<
            property_definition<
               typename T::accessor_function_type,
               typename T::lua_check_function_type,
               typename T::lua_pull_function_type,
               typename T::lua_push_function_type
            >,
            T
         >;
      };
   }
}