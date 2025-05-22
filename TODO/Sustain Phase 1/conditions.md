As of 5/10/2024, I've recently rewritten how conditions are handled both on the backend and in helper structs for the frontend. However, there are still improvements that can be made. First, some background and some terms:

* The type system for condition parameters works as follows. We have first the `parameter_underlying_type`, an enum which describes the basic primitive value types: signed integer, unsigned integer, float, form, string, et cetera. Then, we have more detailed `parameter_typeinfo`s, which are full constexpr data structures that describe more specific information.
  
  For example, `parameter_underlying_type::form` means that the parameter's value is a form stub pointer. However, there are multiple typeinfos that are built on this underlying type: `BaseForm`, `InventoryItem`, `VoiceType`, and so on. Condition functions list typeinfos as their argument types; those typeinfos list their underlying types; and we use the underlying types to know what to store for any given parameter.

Currently, a condition parameter looks like this (non-`std` namespaces removed for brevity):

```c++
struct parameter {
   union {
      uint32_t dword = 0;
      float    float32;
      int32_t  integer;
   };
   form_reference_t form;
   std::string      string;
      
   parameter_underlying_type underlying = parameter_underlying_type::none;
};
```

Meanwhile, the "working" struct, intended to make modification by frontends easier, looks like this:

```c++
using working_parameter = std::variant<
   std::monostate,
   uint32_t,   // alias, event, int_unsigned, package_data, quest_stage
   char,       // character
   float,      // float32
   int32_t,    // enumeration, int_signed
   form_stub*, // form
   std::string // string
>;
```

The "working" struct looks cleaner, but in practice, you need to query the containing condition to find the `parameter_underlying_type` for the parameter (given the condition function to which it is a parameter). As indicated by the comments, several of the C++-level types represent multiple `parameter_underlying_type`s; for example, `uint32_t` can be: an unsigned integer; the ID of an alias on the condition's owning quest; the index of a package-data on the condition's owning package; or, if this isn't the first parameter, it can be the ID of a quest stage, belonging to the quest indicated by the previous-sibling parameter.

In other words, it's a "split variant," where you need to consult two pieces of information to identify the meaning of the value inside: the variant's current contained type; and the value of a `parameter_underlying_type` enum. Worse: the enum isn't retained as state *on the parameter*, but rather is deduced from separate state (the condition function ID) on the containing condition. On top of all of this, you still need to know the underlying C++ type with which a `parameter_underlying_type` is being represented: you have to know that enumeration values are stored as `int32_t`s specifically.

An alternative approach would be to use `std::variant` indices: you can have more than one of the same type in a `std::variant`, and then identify the variant variations by index rather than by type. If the indices match the `parameter_underlying_type` enum, then you can just cast it to `size_t`. Better still would be a custom type that wraps `std::variant` and offers more ergonomic access using the `parameter_underlying_type` enum rather than `size_t` or typenames.

## Unifying the split variant

Once we're doing the above refactor for in-memory form data, it'll be more viable to unify all of these approaches, and to have them work consistently between the backend and the frontend. Consider:

```c++
template<form_data_config Config>
class parameter {
   public:
      using form_ref = std::conditional_t<
         Config.track_use_info,
         tracked_form_pointer, // called `form_reference_t` in current codebase
         form_stub*
      >;

   private:
      using _map_entry = cobb::type_containers::map_v_to_t_entry;

   protected:
      // NOTE: This helper type doesn't exist yet. It would map values to types.
      using map_types_to_enum = cobb::type_containers::map_v_to_t<
         _map_entry<parameter_underlying_type::none,         uint32_t>,
         _map_entry<parameter_underlying_type::alias,        uint32_t>,
         _map_entry<parameter_underlying_type::character,    char>,
         _map_entry<parameter_underlying_type::float32,      float>,
         _map_entry<parameter_underlying_type::int_signed,   int32_t>,
         _map_entry<parameter_underlying_type::int_unsigned, uint32_t>,
         _map_entry<parameter_underlying_type::form,         form_ref>,
         _map_entry<parameter_underlying_type::package_data, uint32_t>,
         _map_entry<parameter_underlying_type::string,       std::string>,
         _map_entry<parameter_underlying_type::quest_stage,  uint32_t>
      >;

  public:
      template<parameter_underlying_type Key>
      using value_type_for = map_types_to_enum::value_type_for<Key>;

   protected:
      union {
         char        character;
         uint32_t    dword    = 0;
         int32_t     integer;
         float       float32;
         form_ref    form;
         std::string string;
      } _value;
      parameter_underlying_type _raw_type = parameter_underlying_type::none;

      template<parameter_underlying_type Requested>
      const value_type_for<Requested>* _pointer() const {
         return _static_cast<const value_type_for<Requested>*>(this->_value);
      }

      constexpr void _clear() {
         if (this->_raw_type == parameter_underlying_type::none)
            return;
         map_types_to_enum::for_each_until_true([this]<auto Key, typename ValueType>() {
            if (Key == this->_raw_type) {
               std::destroy_at(_pointer<ValueType>());
               return true;
            }
            return false;
         });
         this->_raw_type = parameter_underlying_type::none;
      }

   public:
      constexpr ~parameter() {
         this->_clear();
      }

      template<parameter_underlying_type Requested>
      constexpr bool is() const noexcept {
         return this->_raw_type == Requested;
      }

      template<parameter_underlying_type Requested>
      constexpr const value_type_for<Requested>& get() const {
         if (!is<Requested>())
            throw exceptions::bad_condition_parameter_access(Requested, _raw_type);
         return *_pointer<Requested>();
      }
      template<parameter_underlying_type Requested>
      constexpr value_type_for<Requested>& get() {
         return const_cast<value_type_for<Requested>&>(std::as_const(*this).get<Requested>());
      }

      template<parameter_underlying_type Requested>
      constexpr void set(const value_type_for<Requested>& v) {
         if (!is<Requested>())
            this->_clear();
         this->_raw_type = Requested;
         construct_at(_pointer<Requested>(), v);
      }
};
```

(Methods omitted for brevity include `operator==`, copy and move functions, and so on.)

Usage would look like this:

```
parameter& param = ...;

if (param.is<parameter_underlying_type::float32>()) {
   float f = param.get<parameter_underlying_type::float32>();
}
param.set<parameter_underlying_type::int_signed>(5);
```

One could potentially even add a `convert_or_clear` operator which, given a requested type, would convert the current value to that type if possible or reset the parameter to an empty/zero value of the requested type otherwise. This would mainly handle conversion between the various numeric types (`float`, `int_signed`, `int_unsigned`).

## Enhancements

Now, the exact class proposed above isn't perfect. We're still acting in terms of the `parameter_underlying_type`, so any two condition parameter types with the same underlying-type would be considered interconvertible. This is most problematic when dealing with enumerations and form stubs: two enumeration types will have different sets of values, and should not be considered interconvertible; and form stubs should be validated against the typeinfo's allowed form types.

There is a way around this. Consider:

```c++
template<form_data_config Config>
class parameter {
   //
   // Non-data members omitted for brevity.
   //
   protected:
      union {
         char        character;
         uint32_t    dword    = 0;
         int32_t     integer;
         float       float32;
         form_ref    form;
         std::string string;
      } _value;

      parameter_underlying_type _raw_type = parameter_underlying_type::none;

      // Index into the `conditions::all_parameter_types` array.
      uint8_t _typeinfo_id = 0;
};
```

We'd have to either include the "all typeinfos" array *or* make the setter (and any "convert-or-clear" function) non-`constexpr`, but this gives us the means to enforce per-typeinfo constraints when setting a parameter's value.

(This does have some limitations. There are two cases where the values allowed in a condition's second parameter depend on the value current present in its first parameter: certain VATS enums; and the quest stage parameter type, where the first parameter is the quest and the second parameter is one of its stages. We have the VATS enums abstracted as "union typeinfos," while the quest stage case is hardcoded into the places where we actually enforce it (which isn't everywhere, as that would require loading the quest's full data into memory). If we want to enforce full data integrity, then we could offer a `set` function that takes a non-const pointer to the next parameter, so that if the next parameter is of a "dependent" type, we can update its own value in response to the current parameter's value changing.)

(Also, **to maintain data integrity**, any member functions that can change the parameter's type would have to be made private on the tracked-use-info `parameter` class, with the tracked-use-info `condition` class being a `friend` of the tracked-use-info `parameter` class. Arguably, `parameter` should have separate member functions for "set of same type" and "force type and set." Arguably we could even do the same for the "working" parameter type, but **it's especially critical for the tracked-use-info type** because any tracked `form_ref`s in the form data need to be, uh, existent. Like, say you manage to stuff a form-ref into a parameter that should (based on the containing condition's condition function ID) hold a float, *while use info is being tracked*, and then save the file: so we serialize the parameter (how we even do that doesn't matter for this problem), and then if the form unloads and reloads, now we expect a float to be there, so we don't read the parameter as a form, and now we've got a "phantom use" stuck in the form's use info.)

## One last sketch...

```c++
template<form_data_config Config>
class parameter {
   //
   // pretend we have a bunch of the stuff from up above e.g. the k/v map here too
   //
   public:
      using value_variant = std::variant<
         char,
         uint32_t,
         int32_t,
         float,
         form_ref,
         std::string
      >;

   protected:
      union {
         char        character;
         uint32_t    dword    = 0;
         int32_t     integer;
         float       float32;
         form_ref    form;
         std::string string;
      } _value;

      parameter_underlying_type _raw_type = parameter_underlying_type::none;

      // Index into the `conditions::all_parameter_types` array.
      uint8_t _typeinfo_id = 0;

   public:
      // Checks typeinfo without resolving union types.
      const parameter_typeinfo* get_raw_typeinfo() const;

      const parameter_typeinfo* get_typeinfo() const;

   public:
      // Returns `true` if the value of `next` is modified.
      template<parameter_underlying_type Desired>
      bool set_value(const value_type_for<Desired>& v, parameter* next) {
         if (!is<Desired>()) {
            throw exceptions::bad_condition_parameter_modify(Desired, this->_raw_type);
         }
         *_pointer<Requested>() = v;

         if (!next)
            return false;
         auto* next_typeinfo = next->get_raw_typeinfo();
         if (!next_typeinfo || !next->is_union())
            return false;

         auto* this_typeinfo = this->get_typeinfo();
         assert(this->typeinfo != nullptr);
         
         auto* resolved = next_typeinfo->resolve_union_type(*this_typeinfo, v);
         if (resolved == next->get_typeinfo())
            return false;

         // TODO: Don't use 0; use the first entry in `resolved`'s enumeration.
         next->set_value((int32_t)0, nullptr);
         return true;
      };

      template<typename T>
      void set_type(const parameter_typeinfo& typeinfo) {
         //
         // ... if this would change the type, force the value to zero/falsy ...
         //
      }

      template<typename T>
      void set_type_and_value(const parameter_typeinfo& typeinfo, const T& v) {
         if (typeinfo.underlying_type != map_types_to_enum::key_for<T>) {
            throw exceptions::bad_condition_parameter_modify(Desired, this->_raw_type);
         }
         //
         // ... if this would change the type, force the value to zero/falsy ...
         //
      }
};
```

## Corresponding frontend changes

* Currently, form-editing dialogs are intrinsically tied to form stubs, even when they use a working copy. It'd be really, really nice if it were possible to invoke a form-editing dialog without a stub, such that we start with default data, and such that if the user clicks "OK" rather than "Cancel," we create a new form and write the user's entered data into that form. Use cases for this include things like the relationship list in ActorBase, where in the CK you can right-click and create a new form right from that spot.
  * Bonus points if it's possible for the caller to alter the form data (e.g. when creating a new ActorBase relationship, pre-fill the current actor as the referent in the new Relationship dialog).