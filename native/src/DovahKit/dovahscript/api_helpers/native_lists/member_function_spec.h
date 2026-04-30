#pragma once

namespace dovahscript::api_helpers::native_lists {
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

      /// [Optional] Function which default-constructs a value of the given type. This 
      /// is needed for certain backend types that don't default-construct properly, 
      /// i.e. conditions in form data.
      //static value_working_type initialize_value();

      /// [Optional] Functions which compute some state prior to making insertions, 
      /// and then apply that state to any inserted or overwritten elements. The state 
      /// type can be any type `T`.
      /// 
      /// The `prep_for_insertion` function is invoked with a Lua stack index if the 
      /// `insert` member function was passed a value to insert. The function must 
      /// always return a vector with `count_to_insert` elements inside (we `assert` 
      /// that you do).
      /// 
      /// The `apply_preparations` function will be invoked for each inserted element. 
      /// If the list allows insertions past the end, then this will include implicitly 
      /// created elements between the end of the list and the desired element.
      /// 
      /// A usage example for this is TopicInfo responses, which must have unique IDs 
      /// (separate from their indices, and potentially non-contiguous) within the 
      /// info's response list. These IDs are automatically computed by default, and 
      /// this is the functionality used to compute them.
      //static std::vector<T> prep_for_insertion(const collection_wrapped_type&, size_t count_to_insert, lua_State*, std::optional<int> value_pos);
      //static value_working_type apply_preparations(value_working_type&, T);

      ///
      /// == Functions for pull-and-store ==
      /// 
      /// These functions are used when it's possible to pull a table from Lua, convert 
      /// it to a native object, and retain that native object as a local variable. 
      /// Broadly speaking, that'll be possible if there are no `form_reference_t` or 
      /// other "managed" fields in the object, or if the object is a "working" variation 
      /// of something that relies on a transaction model (e.g. form conditions).
      /// 

      /// Function which pulls a value off of the Lua stack, suitable for insertion 
      /// into the wrapped native list.
      //static value_working_type pull_value(lua_State* L, int pos);

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


      ///
      /// == Functions for validate-and-overwrite ==
      /// 
      /// These functions are used when pull-and-store isn't possible. We validate that a 
      /// Lua-side table can be used to overwrite a value; and then we perform the overwrite 
      /// by reading directly from the table into the object's fields.
      /// 
      /// This is designed to compose well with the "sub-object property helpers" elsewhere 
      /// in the `api_helpers` namespace.
      /// 

      /// Function which validates that a value on the Lua stack can be used to overwrite an 
      /// element in the wrapped list. The former overload is used for an insertion, while 
      /// the latter overload is used for overwriting an existing value.
      //static void validate_value(lua_State* L, int pos, const dovah::loaded_forms::Form&);
      //static void validate_value(lua_State* L, int pos, const dovah::loaded_forms::Form&, const value_stored_type&);

      /// Function which overwrites a stored value with a value from the Lua stack.
      //static void overwrite_value(lua_State* L, int src_pos, value_stored_type&, dovah::loaded_forms::Form&);
   };
}