#pragma once
#include <concepts>
#include <type_traits>
#include <utility> // std::pair
#include "lua.h"
#include "./impl/push_value.h"
namespace dovah {
   namespace loaded_forms {
      class Form;
   }
   class form_stub;
}
namespace dovahscript {
   class wrapper;
}

namespace dovahscript::api_helpers::native_lists {
   struct member_function_spec;

   namespace impl {
      template<typename Spec>
      concept pushes_value_directly = requires {
         typename Spec::value_stored_type;
         requires requires(lua_State* L, const typename Spec::value_stored_type& v) {
            { Spec::push_value(L, v) } -> std::same_as<int>;
         };
      };
      template<typename Spec>
      concept pushes_value_as_subobject_wrapper = requires {
         requires requires(lua_State* L, wrapper& collection, size_t zero_based_item_index) {
            { Spec::push_value(L, collection, zero_based_item_index) } -> std::same_as<int>;
         };
      };

      template<typename Spec>
      concept values_are_subobjects = requires {
         requires !std::is_same_v<typename Spec::value_wrapper_type, void>;
      };

      template<typename Spec>
      concept is_a_spec = requires {
         requires std::is_base_of_v<member_function_spec, Spec>;
         requires !std::is_same_v<typename Spec::collection_wrapped_type, void>;
         requires !std::is_same_v<typename Spec::value_stored_type,       void>;
         requires !std::is_same_v<typename Spec::value_working_type,      void>;

         requires requires(lua_State* L) {
            { Spec::pull_collection(L) } -> std::same_as<wrapper&>;
         };
         requires (values_are_subobjects<Spec> ? pushes_value_as_subobject_wrapper<Spec> : (pushes_value_directly<Spec> || value_type_is_default_pushable<Spec>));
      };

      // optional static member functions:
      namespace get_collection_length {
         template<typename Spec>
         concept present = requires {
            { Spec::get_collection_length };
         };

         template<typename Spec>
         concept valid = requires {
            requires present<Spec>;
            requires requires(lua_State* L) {
               { Spec::get_collection_length(L) } -> std::same_as<int>;
            };
         };
         
         template<typename Spec>
         concept defaultable = requires {
            requires !present<Spec>;
            typename Spec::collection_wrapped_type;
            requires requires(const typename Spec::collection_wrapped_type& list) {
               { list.size() } -> std::convertible_to<lua_Integer>;
            };
         };
      }
      namespace get_item_by_index {
         template<typename Spec>
         concept present = requires {
            { Spec::get_item_by_index };
         };

         template<typename Spec>
         concept valid = requires {
            requires present<Spec>;
            requires requires(lua_State* L) {
               { Spec::get_item_by_index(L) } -> std::same_as<int>;
            };
         };
         
         template<typename Spec>
         concept defaultable = requires {
            requires !present<Spec>;
            typename Spec::collection_wrapped_type;
            typename Spec::value_stored_type;
            requires requires(const typename Spec::collection_wrapped_type& list, size_t i) {
               { list[i] } -> std::same_as<const typename Spec::value_stored_type&>;
            };
         };
      }
      namespace initialize_value {
         template<typename Spec>
         concept present = requires {
            { Spec::initialize_value };
         };
         template<typename Spec>
         concept valid = requires {
            requires present<Spec>;
            { Spec::initialize_value() } -> std::same_as<typename Spec::value_working_type>;
         };
      }
      namespace pull_value {
         template<typename Spec>
         concept present = requires {
            { Spec::pull_value };
         };
         template<typename Spec>
         concept valid = requires {
            requires present<Spec>;
            requires requires(lua_State* L, int i) {
               { Spec::pull_value(L, i) } -> std::same_as<typename Spec::value_working_type>;
            };
         };
      }
      namespace store_value {
         template<typename Spec>
         concept present = requires {
            { Spec::store_value };
         };
         template<typename Spec>
         concept valid = requires {
            requires present<Spec>;
            typename Spec::value_stored_type;
            typename Spec::value_working_type;
            requires (
               std::is_invocable_v<decltype(Spec::store_value), const typename Spec::value_working_type&, typename Spec::value_stored_type&>
            || std::is_invocable_v<decltype(Spec::store_value), const typename Spec::value_working_type&, typename Spec::value_stored_type&, dovah::loaded_forms::Form&>
            );
         };
      }
      namespace unwrap_collection {
         template<typename Spec>
         concept present = requires {
            { Spec::unwrap_collection };
         };

         template<typename Spec>
         concept is_bifurcated = requires(wrapper& self) {
            requires present<Spec>;
            typename Spec::collection_wrapped_type;
            { Spec::unwrap_collection(self) } -> std::same_as<std::pair<typename Spec::collection_wrapped_type*, typename Spec::collection_wrapped_type*>>;
         };

         template<typename Spec>
         concept is_single = requires(wrapper & self, size_t i) {
            requires present<Spec>;
            typename Spec::collection_wrapped_type;
            { Spec::unwrap_collection(self) } -> std::same_as<typename Spec::collection_wrapped_type*>;
         };

         template<typename Spec>
         concept valid = is_bifurcated<Spec> || is_single<Spec>;
      }

      template<typename Spec>
      concept is_fully_valid_spec = requires {
         requires is_a_spec<Spec>;
         requires (get_collection_length::valid<Spec> || get_collection_length::defaultable<Spec>);
         requires (get_item_by_index::valid<Spec> || get_item_by_index::defaultable<Spec>);
         requires (!initialize_value::present<Spec> || initialize_value::valid<Spec>);
         requires (!pull_value::present<Spec> || pull_value::valid<Spec>);
         requires (!store_value::present<Spec> || store_value::valid<Spec>);
         requires unwrap_collection::valid<Spec>;
      };
   }

   // A helper type for generating collection APIs via template metaprogramming.
   // 
   // Subclass this, shadow the `using` declarations and static members as appropriate, 
   // and define the functions that are described as needed. Then, you can use the new 
   // subclass (the "spec") as a template parameter to helper templates in this folder 
   // which generate Lua script APIs for you.
   struct member_function_spec {
      member_function_spec() = delete;

      // Type of the native list that you're exposing to Lua.
      using collection_wrapped_type = void;

      // If the list is of sub-objects, the type that defines their wrapper characteristics.
      using value_wrapper_type      = void;

      // Type of the items in the native list (e.g. int, dovah::form_reference_t).
      using value_stored_type       = void;

      // Type of temporaries that can be stored in the list (e.g. int, dovah::form_stub*).
      using value_working_type      = void;

      static constexpr const bool allow_insertions_past_end = false;
      static constexpr const bool allow_removals = true;

      //
      // The functions below must be declared in your spec subclass:
      //

      /// Function which pulls the `self` argument off the Lua stack, type-checks it, 
      /// and returns the native `wrapper` object contained in its userdata.
      //static wrapper& pull_collection(lua_State* L);
      
      /// Function which, given the `wrapper` above, returns a pointer to the native 
      /// list that we're trying to grant Lua access to.
      //static collection_wrapped_type* unwrap_collection(wrapper& self);

      /// [Alternative] For the rare edge-case of collection classes that need to be 
      /// able to expose a bifurcated list to Lua, with it treated as if it were a 
      /// single list. We assume the bifurcated list is split such that the first part 
      /// is immutable and the second part is mutable.
      /// 
      /// If your class handles both normal and bifurcated lists, then when dealing 
      /// with a normal list, return { nullptr, &list }.
      /// 
      /// For a usage example, refer to the condition list API, which exposes both 
      /// ordinary condition lists and the potentially bifurcated ones in TopicInfo.
      /// 
      /// Do not provide both this and the overload listed above. Pick one.
      //static std::pair<collection_wrapped_type*, collection_wrapped_type*> unwrap_collection(wrapper& self);

      /// Function which pulls a value off of the Lua stack, suitable for insertion 
      /// into the wrapped native list.
      //static value_working_type pull_value(lua_State* L, int pos);

      /// [Optional] Function which default-constructs a value of the given type. This 
      /// is needed for certain backend types that don't default-construct properly, 
      /// i.e. conditions in form data.
      //static value_working_type initialize_value();

      /// Function which pushes a collection item onto the Lua stack. Use this signature 
      /// (and not the other one) if this is a collection of sub-objects.
      //static int push_value(lua_State* L, wrapper& collection, size_t zero_based_item_index);

      /// Function which pushes a collection item onto the Lua stack. Use this signature 
      /// (and not the other one) if this is a collection of simple values for which no 
      /// `impl::default_push_value` overload exists. (If that function has an overload 
      /// for your stored type, then you need not define a `push_value` function at all.)
      //static int push_value(lua_State* L, const value_stored_type&);

      /// [Optional] Function which, given a value to store and a place to store it, will 
      /// carry out the assignment as needed. This is only needed if the working type can't 
      /// simply be assigned to the stored type, and if the stored type isn't a form use 
      /// (i.e. `dovah::form_reference_t`).
      //static void store_value(const value_working_type&, value_stored_type&, dovah::loaded_forms::Form&);
   };
}