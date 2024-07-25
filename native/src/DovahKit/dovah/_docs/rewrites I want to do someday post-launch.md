
# Timeline

* **Phase 1: Rewrite the backend for loaded form data.**  
  Loaded form data is handled very messily, and the "working copy" system forces ephemeral copies of form data to have all the same boilerplate as the persistent stuff. This makes it harder and jankier to write UI-related code. The refactor plans described further below should make it significantly easier to write UI-related code, should allow a lot of cleanup of existing UI-related code, and should lead to higher-quality code in general.

  This phase is expected to enable, or make it easier to implement, the following features:

  * Ability to right-click in listviews like ActorBase's relationship list, pick a "New" option, and open a form-editing dialog without a pre-existing stub, wherein a Relationship form is created if you click "OK"
  * Flowchart-like editor for dialogue
  * Cleaner code for editing Papyrus bound scripts
    * Currently, `DKPapyrusBoundScriptListPane` can't auto-commit changes to a script directly to a form-working-copy's VMAD, because `DKBoundScriptListModel` only keeps track of the VMAD itself and not the working-copy form (so it can't use `form_reference_t::set`). This means that form-editing dialogs have to manually commit changes (from the UI to the working-copy VMAD) on save. We *could* have the model track the working-copy form in order to allow auto-committing... *or* once the backend rewrite is done and working copies use bare `form_stub*` fields, it'll then be possible to give `DKPapyrusBoundScriptListPane` the ability to optionally auto-commit directly to a working-copy VMAD.

* **Phase 2: Rewrite the renderer.**  
  The current renderer design is difficult to maintain and not configurable for different use cases. It's not bad at all for, like, the third time I've ever built a renderer and the first time I've ever meaningfully succeeded, but it's not scalable in the ways I need it to be. It's easily the single worst "God object" I've ever written: this is the only time in my entire life that I've had to have *four separate `.cpp` files* for a single header. We need to figure out how best to divide up the renderer's systems and data, and figure out how to specify things like render pass and shader definitions as `constexpr` PODs in order to separate "data" from "code." We also need to make the renderer configurable: right now, it *always* preallocates enough VRAM resources to run the Render Window (i.e. enough to render a large portion of a worldspace), which is beyond excessive for things like previewing a single form or editing an actor's appearance; and this, indeed, is why DovahKit will not ship with a Preview Window.

  This phase is expected to enable:

  * "Preview" window for any form with a 3D model
  * "Preview" pane shown when editing a form's 3D model (including its texture swaps)
  * Editing ActorBase appearances
    * This is something we'll have to actually lock down on initial release, since having users "fly blind" when editing appearances is straight-up not viable at all.
    * A bonus feature we could offer here, once actor editing in general is available, is the ability to let the user draw a separate "custom facepaint" image file that we automatically bake (i.e. alpha-blend) onto an NPC's exported tintmask.
      * ...And a *very far-future, if ever,* idea would be to let the user paint with the mouse directly onto the 3D render of the actor's face, as one can do in Blender.
  * Improvements to the renderer's accuracy (e.g. support for water, EffectShaders, etc.)
  * "Preview" pane for EffectShaders (akin to the unimplemented one in the FO4 CK)

* **Phase 3: Refactor Worldedit.**  
  Worldedit's "tool" system is close to what we want, but conceptually it's not well-organized; see the section below. We should divide this into a two-tiered hierarchy: "tools" with "compositions." Separately from this, we should investigate things like ordered binds (i.e. a flexible list of actions per frame, allowing multiples of the same tool, rather than coalescing/merging/clobbering on a per-tool basis). Potentially in the future that could even enable things like having multiple tools per bind.

  This phase is expected to enable, or make it easier to implement, the following features:

  * Completion of all binds for editing objects
  * Navmesh editing
  * Terrain editing

* **Phase ?: Refactor file loading.**  
  Not sure where in the process to put this one.
  
  `file_load_order` should be renamed to `active_load_order` and made the end product of a load operation, with some temporary "load process" data structure actually handling (and containing the code for) the load operation. Additionally, the classes (and class hierarchy) for actually parsing and loading files is spaghetti. There are plans further below for redesigning all of this.

* **Phase ?: Refactor Dovahscript.**  
  Not sure where in the process to put this one.
  
  Dovahscript currently relies on *lots* of copying and pasting in order to implement the bulk of form access APIs. This means that refactoring how APIs work (e.g. adding new checks or changing internals) will be prohibitively difficult, and adding new APIs involves a lot of boilerplate. Pre-launch, I'm deliberately choosing *not* to fix this, because I don't want to commit to the wrong abstraction or development process. I want to get the APIs mostly feature-complete, if maybe lacking in some robustness and polish, and then -- after all core functionality is working -- look for common patterns and trends, pay careful attention to their edge-cases and exceptions, and *carefully* explore ways to cut down on duplication and boilerplate using metaprogramming.

  This phase is expected to enable, or make it easier to implement, the following features:

  * More ergonomic script APIs, e.g. DovahScript more consistently allowing you to "copy-assign" data sub-structures between forms or form components rather than having to manually copy individual members across data sub-structures.
  * Potentially, changes to the script engine internals
    * Currently, Dovahscript runs on a worker thread to limit the damage that an infinite loop in a script can do. *[EDIT: That isn't the sole benefit, and a worker thread may not be required to achieve it. See Dovahscript's section below.]* I have some pie-in-the-sky ideas for optionally letting Dovahscript run on the main thread, which would make it viable to offer APIs that allow for stronger integration into the editor: think "add-ons" rather than "scripts". (**Do not take this as a promise or even a statement of intent,** but some examples of that kind of integration are attach points for adding scripted UI controls into native windows, and real-time scripted control over the Render Window.) This would require being able to configure the script engine's threading model at run-time, and adding *that* would, in turn, require changes to how basically all form access APIs work -- which, again, would be easier with less code duplication throughout Dovahscript.

# General plans for the backend

* Type-safe flags-masks all over the backend. It's trivial to do this with templates now, so the "non-strict enum inside a struct" trick isn't needed anymore.
  * That said, the "non-strict enum inside a struct" trick allows "subclassing enums" when one wrapper struct subclasses another. Never tried doing that with template-based flags masks before...
    * Constexpr validation of flags, in the context of subclassable/extensible flags masks, would require explicit object parameter support ("deducing `this`"), which is busted in IntelliSense as of this writing (9/18/2023; definitions work; calls show false-positive errors).
* Better access specifiers for `form_stub`'s members. Lots of stuff is public when it should arguably be private with a `constexpr const auto&` getter and maybe some passkeyed non-const accessors.
* Revise `loaded_form_ptr` to be much more ergonomic. Having to get an untyped smart pointer via `form_stub::load` and manually call `ptr_cast` on it is annoying.
* Rename `dovah::loaded_forms` to just `dovah::forms`. We'd still want to call these things "loaded form classes," but naming the namespace that feels redundant; that you have an instance of one of these classes on hand implies that you've loaded it.
* Move `dovah::loaded_forms::components` to `dovah::form_components`.
* Change how form components are handled:
  * Have a loaded form class define an anonymous `components` member struct.
  * Put the relevant loaded form component members inside of that struct.
  * Have a dispatch table that can access a component from any form that actually contains it, accounting for `std::optional<T>` components like destruction data.
    * We already have accessors defined as member functions on `Form`. It'd be nice to have a single accessor templated on the desired component type; however, it'd have to be non-member rather than a member of the base class, e.g. `dovah::component_of<T>(form)` or `dovah::form_components::get<T>(form)`, as whatever form pointer you use needs to be a derived pointer, not a base class pointer.
* Change how forms handle embedded references to other forms:
  * Currently, to clone a form that contains references to other forms, we instantiate an empty destination form (`dst`) and then copy from the source form (`this`) in the latter's `_clone_impl` virtual member function. Copying those references, then, looks like this: `dst->some_ref.set(*dst, this->some_ref)`. It'd be nice if we instead had a dedicated "clone form reference" function that would take as arguments the source form, the destination form, and a pointer-to-member.
    * Bonus points if the pointer-to-member can be to anything: if it's a `std::vector<form_reference_t>`, then we loop over it; if it's a custom struct, it can define its own "clone" function that we'll then invoke via templating.
    * And ditto for helper functions like `copy_form_reference_list`.
    * Ditto for severing outbound references to a target form, and for clearing data, too.
    * Okay, but does C++ support nestable pointers-to-member, i.e. `&SomeStruct::nestedStruct::member` such that you could use that from a `SomeStruct`?
      * As of April 2024, I believe the answer is "extremely no." Perhaps a workaround would be to pass both forms and a lambda which, given a form, returns the desired member?
  * If possible, we should set `form_reference_t::operator=(const form_reference_t&) = delete`. This would significantly reduce the room for error when writing forms' boilerplate.
    * Why did we ever even have `operator=` for that? If it's only used during load, then can we replace it with a passkeyed accessor somehow?
  * In my bitstream classes, I implemented "multi-read" and "multi-write" methods: they used variadic template parameters to allow you to read or write fields in bulk. Can we write helper functions that behave similarly, for the various tasks a form might need to perform (i.e. clearing data, severing outbound references to a to-be-deleted form, cloning data, etc.)?
* Improvements to form extra-data:
  * Replace `template<class E> E* extra_data_list::lookup(extra_data_type et)` with a getter that doesn't take any arguments; use a `constexpr` mapping of extra-data classes to typecodes to know what typecode to look for.

# Specific plans

## Form backend and working copies

We should redesign the backend as follows:

* Split loaded-forms and working copies into totally separate constructs, with only the former integrating and automating the legwork needed to track Use Info
* To facilitate the previous, move form data out of the loaded-form class. So you'd have `dovah::form_data::Activator` as a templated type (with template parameters determining whether we have the Use Info machinery), and then you'd have `dovah::forms::Activator` and `dovah::form_working_copies::Activator` which contain an instance of that data each configured with the appropriate template parameters.

This would allow GUI code to operate on working copies without having to constantly go through the Use Info machinery (which is currently present but disabled on working copies, since in the current design loaded forms and working copies are of the same type).

The namespaces should be:

* **`dovah`**  
  Top-level namespace for the backend.
  * **`form_data`**  
    Namespace for form data classes -- one per form type, each templated on a `form_data_config` value.
  * **`form_components`**  
    Namespace for form component classes, like VMAD or DEST; these are things that forms would multiply-inherit from in Bethesda's codebase, though we use composition in ours. Each component type would be templated on a `form_data_config` value.
  * **`form_fields`**  
    Namespace for common structs that recur across multiple form types, such as the "color" struct used for ACTI/CNAM and others.
  * **`forms`**  
    Namespace for loaded forms.
  * **`form_working_copies`**  
    Namespace for form working copies.

```c++
namespace dovah {
   struct form_data_config {
      bool track_use_info = true;
   };

   namespace form_data {
      template<form_data_config Config>
      class _base {
         public:
            using form_ref = std::conditional_t<
               Config.track_use_info,
               tracked_form_pointer, // called `form_reference_t` in current codebase
               form_stub*
            >;
      };
   }

   namespace forms {
      class Form {
         public:
            struct form_flag {
               form_flag() = delete;
               enum : uint32_t {
                  deleted = 0x00000020, // working with this flag directly is undefined behavior. use the (flagged_as_deleted) flag on (form_stub) instead.
               };
            };

         public:
            //
            // Virtual and non-virtual functions needed for form handling.
            //
      };
   }
   
   // forward-declare
   template<enum form_type FormType, template<form_data_config> class Data>
   class form_working_copy;

   template<enum form_type FormType, template<form_data_config> class Data>
   class loaded_form : public forms::Form {
      public:
         static constexpr const form_type type = FormType;

      protected:
         static constexpr const form_data_config _data_config = {
            .track_use_info = true;
         };

      public:
         form_stub&         stub;
         Data<_data_config> data;

         void commit_working_copy(form_working_copy<FormType, Data>&&);
   };

   template<enum form_type FormType, template<form_data_config> class Data>
   class form_working_copy {
      public:
         static constexpr const form_type type = FormType;

      protected:
         static constexpr const form_data_config _data_config = {
            .track_use_info = false;
         };

      public:
         form_stub& source_stub;
         
         struct {
            //
            // These properties are normally stored within the form stub. 
            // The values here are "working" values: make your changes to 
            // these, and then when you commit the working copy, those 
            // changes will be mirrored onto the stub.
            //
            const uint32_t form_id;
            std::string    editor_id;
            uint32_t       form_flags;
         } record;
         
         Data<_data_config> data;
   };
}
```

And an example of using a specific form with this:

```c++
namespace dovah {
   namespace form_data {
      template<form_data_config Config>
      class Activator : protected _base<Config> {
         public:
            form_ref water_type = {};
            //
            // NOTE: We need to initialize it to {} so that when `form_ref` is a 
            //       pointer, it's initialized to `nullptr`.


            // pretend I could be bothered to list the other members here


            template<form_data_config AltConfig>
            Activator& operator=(const Activator<AltConfig>& src) requires (!Config.track_use_info);
            
            template<form_data_config AltConfig>
            void overwrite_with(const Activator<AltConfig>& src, form_stub& this_form) requires (Config.track_use_info);
      };
   }

   namespace forms {
      class Activator : public loaded_form<form_type::activator, form_data::Activator> {
         public:
            //
            // virtual overrides for serialization funcs here
            //
      }
   }
   namespace form_working_copies {
      using Activator = form_working_copy<form_type::activator, form_data::Activator>;
   }
}
```

### Helpers

My bitstream classes have templated "multi-read" and "multi-write" methods, wherein `stream.read(a, b, c)` will receive `a`, `b`, and `c` as references and read each of them in sequence: for each argument, the stream checks if it's a struct with a `read(stream)` method and if so, invokes that; otherwise, it checks if they're simple primitives and reads them using internal functionality if so.

It would be useful to have helper functions that apply this pattern to the common tasks involved in working with a form's data:

* Clearing the form's data
* Cloning the form's data
  * Syntax for this would be a bit more complicated...
* Severing references to a to-be-deleted form

Alternatively, we could use variadic template parameters and similar tech to define `visit_members` functions on each form-data type:

```c++
//
// Base interface:
//
namespace dovah {
   namespace form_data {
      template<form_data_config Config>
      class _base {
         public:
            using form_ref = std::conditional_t<
               Config.track_use_info,
               form_reference_t,
               form_stub*
            >;

         protected:
            template<typename Functor, typename... Fields>
            constexpr void _exec_per_member(this auto&& self, Functor&& functor, Fields&... fields) {
               ((functor)(fields), ...);
            }
      };
   }

   // Structs that recur across several form types:
   // (These would be in separate files in "real" code.)
   namespace form_fields {
      struct color {
         uint8_t r = 0;
         uint8_t g = 0;
         uint8_t b = 0;
         uint8_t unused = 0;
      };

      template<typename Enum>
      struct flags_mask {
         // pretend i could be bothered to write something that treats an enum 
         // like a mask here
      };
   };
}

//
// Form type:
// (The main thing we care about is the `visit_members` method.)
//
namespace dovah {
   namespace form_data {
      template<form_data_config Config>
      class Activator : protected _base<Config> {
         public:
            struct activator_flag {
               activator_flag() = delete;
               enum type : uint16_t {
                  no_displacement    = 0x0001,
                  ignored_by_sandbox = 0x0002,
               };
            };
            using activator_flags = form_fields::flags_mask<activator_flag::type>;

            // Default color used by the Creation Kit for new Activators:
            static constexpr const form_fields::color default_primitive_color = {
               .r = 204,
               .g =  76,
               .b =  51,
            };

         public:
            struct {
               std::optional<form_components::destruction_data<Config>> destruction_data;
               form_components::keyword_list<Config>  keywords;
               form_components::model_ts<Config>      model;
               form_components::object_bounds<Config> object_bounds;
               form_components::papyrus<Config>       papyrus;
            } components;
            //
            localized_string   name;
            form_fields::color primitive_color = default_primitive_color;
            struct {
               form_ref looping  = {};
               form_ref interact = {};
            } sounds;
            form_ref         water_type       = {};
            form_ref         interact_keyword = {};
            localized_string verb_override;
            activator_flags  flags;

            // pretend i could be bothered to list previously described member functions here

            template<typename Functor>
            constexpr void visit_members(this auto&& self, Functor&& functor) {
               self._exec_per_member(
                  functor,
                  //
                  // Members:
                  //
                  components.destruction_data,
                  components.keywords,
                  components.model,
                  components.object_bounds,
                  components.papyrus,

                  name,
                  primitive_color,

                  sounds.looping,
                  sounds.interact,

                  water_type,
                  interact_keyword,
                  verb_override,
                  flags
               );
            }
      };
   }
}
```

Each form component could be set up with a similar `visit_members` function.

### Measures for data integrity

The reason `form_reference_t` (which we should rename to `tracked_form_pointer` when redoing the backend) is designed as it is &mdash; the reason we endeavor to update Use Info in real time, as opposed to allowing outside code to write fields arbitrarily and then manually call an "update" method &mdash; is to maintain data integrity. Theoretically, the backend Use Info should always match the actual data in the form, because it should be updated in real-time.

In practice, there are some holes in this design, stemming in large part from the fact that I wasn't as fluent in C++ and its design patterns back when I first started work on DovahKit.

* `form_reference_t::operator=` should probably be `delete`d rather than made `protected`. (I don't think we even use it internally; perhaps we did at some point in the past...)

* `form_reference_t::unmanaged_set` should be passkeyed (and underscore-prefixed). I hadn't learned the passkey pattern back when I started. It should perhaps be renamed to `_untracked_set` as well.

* It should not be possible to construct a `form_reference_t` with a value already in place. Any cases where we do that while loading forms should instead use the passkeyed setter.

* Anything along the lines of `std::vector<form_reference_t>`, or `std::vector<T>` where `T` contains, directly or indirectly, a `form_reference_t`, creates a potential hole in data integrity. Nothing is stopping you from simply calling `erase` on that vector and blowing away a `form_reference_t` without updating Use Info.

  The only way I can think of to remedy this while keeping `form_reference_t` more-or-less as-is would be to create custom container implementations that ensure that Use Info is properly tracked on their elements. Notably, the destructors on these containers *should not* perform those sorts of updates because we want to be able to unload the loaded form data. These custom container implementations should support any `T` provided that `T::clear(*loaded_form)` is callable.

  Within form data, these custom containers would be conditional types, i.e. `tracked_form_pointer_vector` for a "real" form and `std::vector<form_stub*>` for a working copy. It's fine for the typenames to be a little wordy, because inside a form data class, they'd be `using`'d as `form_ref_list`, `form_ref_array<Size>`, and so on.

  * We would of course need to be able to handle lists of sub-structures that themselves contain form refs, though. Could define generic containers that call functions like `T::clear` and so on, and then `using` them as `substruct_vector<T>`, `substruct_array<T>`, and so on?

### Conditions

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

In other words, it's a "split variant," where you need to consult two pieces of information to identify the meaning of the value inside: the variant's current contained type; and the value of a `parameter_underlying_type` enum. Worse: the enum isn't retained as state *on the parameter*, but rather is deduced from separate state (the condition function ID) on the containing condition.

#### Unifying the split variant

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

#### Enhancements

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

#### One last sketch...

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

#### Corresponding frontend changes

* Currently, form-editing dialogs are intrinsically tied to form stubs, even when they use a working copy. It'd be really, really nice if it were possible to invoke a form-editing dialog without a stub, such that we start with default data, and such that if the user clicks "OK" rather than "Cancel," we create a new form and write the user's entered data into that form. Use cases for this include things like the relationship list in ActorBase, where in the CK you can right-click and create a new form right from that spot.
  * Bonus points if it's possible for the caller to alter the form data (e.g. when creating a new ActorBase relationship, pre-fill the current actor as the referent in the new Relationship dialog).

## Improve form stub / file handling and loading

### File loading

The data structures for this feel like spaghetti. They're better than they used to be back in DovahKit's very early proof-of-concept stage, but they're still years old and poorly designed. Is there anything we can do to improve them further?

One major conceptual flaw I see (looking into a lot of this machinery on 4/27/2024 while trying to disentangle and rewrite the error handling) is that `file_loader` represents both something which *loads* a file and affords you functions to read from it as a stream, *and* the authoritative "owner" of the actual file data and metadata. (And describing that in reverse: its base class, `basic_reader`, could be a file *or* a view into a file, with this varying from subclass to subclass; and consequently `basic_reader` itself doesn't know how to get information, e.g. the name, about the file it's actually reading, because it doesn't know whether to "ask itself" or ask something else.) This feels wrong.

We should have a `dovah::tes::file` class which holds and "owns" the mapped file, and then all the various "loaders" should act like views into this single authoritative file object.

Some specific ideas, first written down on 4/28/2024:

* **`dovah::tes::file`** as a single TES file, with ownership of a mapped view, and with known information pulled from the header and stored as members. It should not contain or require an owning `file_load_order`. There should not be any functions for actually reading data from the mapped view (e.g. `read` and `unchecked_read`), nor should there be any other "stream" fields like a "current position." Outside code (e.g. `form_stub`s; load processes; etc.) would create a `file_view` (see below) to perform reads.

  * But how, then, do we read the header information so that the `file` can store and report information about itself? Potentially? Have it create and use a throwaway `file_view` (see below) internally.

* **`dovah::load_order_file`** as a subclass of `dovah::tes::file`, which adds an owning `file_load_order&` as well as runtime-specific flags (e.g. "is dummy file for hardcoded forms" and "is dummy file for none-stubs"). This is what a `file_load_order` would store and what `form_stub`s would use in their file list.

  * In the current implementation, `form_stub`s identify their owning load order *via* their files, in order to keep the stubs' memory footprint to a minimum since we have so many of the things around. This is also why we have "dummy files" in the first place: so that `form_stub`s that were never loaded from a "real" file can still reach their owning load order.

* **`dovah::tes::file_view`** as a view into any given `file`. This would be the counterpart to what is currently called `dovah::tes_file_reading::basic_reader`, with state for groups, records, and subrecords, and functions like `load_record_at` and `next_record_or_group`.

  * An advantage of making `file_view` wrap a `file` is that if we encounter a structurally malformed file (e.g. a suspicious record signature, or an ill-formed `XXXX` subrecord), we'll be able to access the file data (most pertinently its name) when we throw an error. Right now, `dovah::tes_file_reading::basic_reader` is actually incapable of providing the filename as diagnostic information in these cases: a `basic_reader` doesn't know if it's a file (`file_loader`) or a view into a file (any other subclass), and consequently, doesn't know how to find its way to *whatever file it's reading* to query that file's name.

  * Perhaps we could even template this and allow some compile-time configuration, e.g. a choice of whether to throw exceptions on invalid data or `assert` correctness instead. (Why would you ever want to `assert` that user-supplied data is well-formed? Because we crawl the files fairly completely during the initial file load, and after that, on-demand form data loads operate on the assumption that the file *definitely is* structurally valid and reads will never throw. Checks for e.g. record signature validity are skipped, and we assert instead of throwing, but this is conditioned on a run-time flag that gets set on the whole file post-file-load. In the new system, a `form_stub` would create disposable `file_view`s for loading each file, and so we may as well move the throw/assert choice to compile-time, no?)

    * We may even want to go the extra mile and have a `record_view`, so that stubs can create views on the stack without burning extra stack space on e.g. machinery to track GRUPs.

  * And as long as we're rebuilding the logic for reading file content, we may as well add support for endian-flipped files, conditioned behind a `constexpr const bool`. Bethesda themselves have this support: if the file header's signature reads as `4SET`, then they know that the file endianness doesn't match the system's native endianness, and they byteswap every value they read.

    * Why condition it behind a `constexpr bool`? Two reasons. First: if we rewrite the file load system, then we'll want to be able to compare benchmarks as directly to the old system as possible to ensure there's no notable perf hit. We'd want to disable any endian-flip support for these initial tests to ensure the branching and similar doesn't impact performance; then enable it later and measure the performance impact.

    * Endian-flip branches should probably be marked as `[[unlikely]]`.

    * We'd have to audit use info and form load code to ensure that no byte-stitching is done in either place, lest any endian-flipped data break there.

    * How would we test this? We don't have any endian-flipped files, and creating one would be very cumbersome.

* **`dovah::tes::record_reader`** and **`dovah::tes::subrecord_reader`** as the interfaces for reading records, i.e. the replacements for `dovah::tes_file_reading::record` and `dovah::tes_file_reading::subrecord`.

  * The `record_reader` interface shouldn't offer any functions for reading arbitrary data (i.e. no `read` or `unchecked_read` functions). Clients that are given access to a record should be required to obey the file structure (i.e. open, read, and close subrecords). There are internals for file parsing that require pulling data from a record, but those functions could be made internal or the relevant reads could otherwise be done manually.

    This would be an improvement over the current design, wherein form loaders, form use info builders, and any custom parses (e.g. the frontend caching subrecords of interest) can just choose not to obey the file structure -- to pluck arbitrary bytes out of a record without bothering to heed subrecord boundaries.

### Form stubs

The `load` function will fail, returning without loading form data, under the following circumstances:

* The stub has no source files
  * Only possible while the stub is being built; `assert` that source files are present instead of failing?
  * Redundantly checked in two parts of the function
* The stub is for a GMST or a none-stub
* Form loading is blocked (i.e. the load order is mid-load or mid-save, and we're not loading as part of that save operation)
* No form loader (i.e. loading the given form type isn't implemented yet)
* Any load failure
  * Unsaved active file (or other cases of the stub's file offset being 0)
    * In the case of an unsaved active file, shouldn't we assert that this isn't the case? It shouldn't be possible to create a form in the active file without creating both the stub and its loaded data (and indeed, `file_load_order` will delete a stub if it fails to create blank loaded-form data for it), so if the stub belongs to an unsaved active file, then it should *always* already have loaded-form data and should early-out at the start of the `form_stub::_load` function.
    * Under what circumstances *can* that file offset be 0? We need to document that, and any other sentinel and uninitialized values within form stubs.
  * `instantiate_hardcoded_form` returns `nullptr`
    * Under what circumstances is this allowed to occur? I assume it's only if we don't have a loaded-form class defined for a given form type, but we should formally document this!
  * `create_blank_loaded_form_by_type` returns `nullptr`
    * Under what circumstances is this allowed to occur? I assume it's only if we don't have a loaded-form class defined for a given form type, but we should formally document this!

As noted in the list above, several of these failure cases should be assertion failures instead; and more rigorous documentation is needed for form stubs, their sentinel values, and when their contents are undefined, too.

There are also cases where a form stub has "unresolved" or "in-progress" values in it. For example, when we create a form stub for parsing a file, we set its form ID to the *file-local* form ID for the record being parsed. When we commit that stub to the load order, we eventually resolve that form ID to a global one and store the resolved form ID on the stub. However, some file load warnings and errors are emitted before form IDs are resolved, and we never get around to resolving form IDs for error reporting; warnings for malformed GMST records are one case that sticks out in my mind. Essentially, this is a case of a "work-in-progress" value being stuck in a field for (possibly) longer than is necessary, with practical downsides that are visible outside of the internals of the load process (i.e. anyone who receives the affected file load warnings/errors has to avoid certain fields on `form_stub` based entirely on implementation details that should be fully hidden away).

### Restructure `file_load_order`

Hm... I don't like its name and I don't like that it's stored in the `dovah/files/` directory.

I think `active_load_order` might be a better name. This would better distinguish it from the general concept of a "load order," while also matching the term "active file" and being clearer about the class's purpose: it holds all of the loaded data associated with a load order; it's the DovahKit counterpart to Bethesda's `TESDataHandler`. We could then repurpose the name "file load order" for the class that we currently call something like "load order normalizer."

One thing I'd really like to do is do a better job of separating out all the machinery related to loading and saving. It'd be nice if `active_load_order` would just retain the loaded data, and defer to temporary data structures for the actual load and save operations &mdash; perhaps something like `dovah::load_order_serialization::load_process` and `dovah::load_order_serialization::save_process`. Passkeys could grant them appropriate access to the `active_load_order` internals.

(PRE-LAUNCH UPDATE: We now use a `save_process`, but it's just a straight-up `friend` of multiple types; and it's used entirely within `file_load_order`, rather than being something you can create externally and then invoke on an active load order. Really, at present it's just a means of moving the save code to other files and splitting it up a bit for organization's sake, rather than being a true refactor. Plus, the machinery for handling files is also messy and so a lot of stuff is still spaghetti. A more complete redesign and rewrite would be needed, with careful consideration given to things like what information needs to be known by what systems, and when, for the purposes of things like error reporting.)

Miscellaneous:

* `form_creation_request::commit` and friends should return a `form_stub&`, so callers don't have to check whether the request succeeded even when it doesn't throw an exception.

## Change `dovah::notices::base_error` and its subclasses into exceptions and throw them directly, instead of wrapping them

Some backstory:

When I first implemented DovahKit's backend -- prototyping it circa December 2019 IIRC, so nearly half a decade ago as I write this -- I didn't know that things like `std::current_exception()` existed: I didn't know that you could catch an exception on a worker thread and then re-throw it on the main thread. As a result, the backend originally didn't use exceptions.

Additionally, one design goal was to reduce heap allocations -- a classic case of optimizing too early, and for the wrong thing. (If DovahKit encounters warnings or errors when loading a file, *who cares* about the perf on signalling them? There shouldn't be that many warnings or errors unless the file is outright malformed; and the bulk of the warnings DovahKit is even capable of signalling come after files initially load, when loading a form's full data on demand.) Did I also avoid exceptions as a bad optimization, based on the widely held misconception that they're "slow?" I don't know.

The "solution" to all of these issues was to use a struct called `detailed_notice`, which contained optional fields for all the information an error could possibly report: filenames, file offsets, form stub pointers, form IDs, the works. Additionally, a `detailed_notice` contained a single error code: a massive enum similar to Win32 error codes. This meant that every warning or error was the same C++ type and the same size: no heap allocation; no polymorphism; and they could be handled uniformly, including by any error-reporting code. This, of course, sucked. It made control flow less clear (already a problem because the file loader was spaghetti; see the "File loading" section above for planned improvements in that regard) and it meant that anything that signals or interprets warning or error codes had to include a single header defining that enum, which is *not great* when you have to add a new warning or error code.

There was one "benefit" of the system, but I basically didn't use it, and never needed it. A `detailed_notice` could, in theory, be stored for later, without having to heap-allocate it. The Log Window was designed to store them, and it used this to identify duplicate errors and avoid displaying the same error multiple times: even if two errors stringified to identical text, if any error information in the `detailed_notice` differed, then they could be told apart. This is moderately valuable when dealing with loading form data on-demand, since the same form may be loaded multiple times and it'd kind of suck to spam the log with duplicate errors. In practice, however, this... doesn't matter? If we're filtering duplicate messages, then all we really care about is the text. And we never used the stored `detailed_notice` objects for anything else, so if anything, they were just wasting memory.

As of April 28, 2024, however, that whole system for signalling warnings and errors has been thrown into the dustbin, replaced with "notice" objects and exceptions. Notice objects come in two flavors: subclasses of `base_warning`, and subclasses of `base_error`. They rely on inheritance, so each kind of warning or error contains fields for just the information that's actually relevant (and unless those fields are `std::optional`, they will *always* be present -- no need to check presence flags before using them). Errors are wrapped in subclasses of `std::exception` and then thrown; when thrown from a worker thread, we carry them to the main thread using `std::exception_ptr` and friends.

This is a massive improvement. Took a week or so and it was well worth the effort. However, it's still not perfect. As a relic of the old system, the "error" notices are not exceptions in themselves; they're basically P.O.D.s, which get wrapped (via `std::unique_ptr`) in an exception when thrown. The "error" notices could in theory be stored, but again, we don't do that. **We really should just turn them into exceptions and throw them directly.** Fortunately, that's a much simpler change than, y'know, replacing the entire `detailed_notice` system in order to get to where we are now. I'm still pushing it to post-launch because I want to just start moving forward again.

# Outside the backend

## Worldedit

### Challenge 1: composition

Currently, all Render Window functions are configured via the UI. The challenge with doing things via the UI is that *compositions* of transformations, rotations, etc., have to be expressable in a GUI form, which in practice means that we have to hardcode every possible combination.

This means that to support moving a selection by fixed distances in directions, or moving by dragging an edit gizmo, for example, we need either one monstrously complicated set of options, or two separate "move selection" tools adapted for each of those scenarios. The ways we're composing reference frames, inputs, and screen state, to get a final vector to move the selection along, are just too different between the two use cases. Rotating a selection is even more complex because in addition to those two cases, we have a third. In <i>Halo: Reach</i>'s Forge controls, rotation is world-relative but the three input axes for it (split across two joysticks) map to camera axes. For any given input axis, the game finds the world axis that is the closest to being parallel to the mapped camera axis, and rotates the selection about that world axis. The result of this is highly intuitive from a UX perspective, but this is yet another way of composing transformations that we have to hardcode.

A GUI simply cannot represent entirely arbitrary compositions of transformations, of vector and rotation primitives, whereas a script can; and so it's tempting to scrap most of Worldedit's tool system in favor of defining a main-thread script API (whether using Lua or something custom-built). However, scripting many of these controls would be *well beyond* 95% of users; the <i>Halo: Reach</i> behavior above, for example, requires not merely a strong understanding of 3D transformations but also a keen enough eye to work out the precise behaviors by testing in <i>Reach</i>. Things like gizmo axis drag would be similarly difficult.

### Challenge 2: conditions

Another problem with the current system is that there's no way to do conditional binds, e.g. "Attempt to move the selection, and only if that movement succeeds, move the camera commensurately," which is the movement behavior in <i>Halo: Reach</i> when an object is selected. There are two reasons for this. The first reason is that we minimize (and often avoid) heap allocation within the control system by relying on a tuple of "requests," one per tool; when the same tool is invoked multiple times within a single frame, we either merge all of its requests into one, or have one supersede all of the others. This in turn means that there can be no ordering between tools &mdash; no way to *know* that one tool was definitely activated before another. The second reason is that there's no way for tools to return any sort of "results," much less accept any such results as input. The only inputs that a tool can accept are the options accompanying the keybind, and in some cases the specific states of the input device (e.g. the direction and magnitude of a joystick or mouse movement).

The current workaround is yet more hardcoded composition: an "also move camera" checkbox on the "move selection" tool, for example.

This is another issue that a scripting language could solve, but forcing end users to work out (and often recreate) the full logic for every single tool and sequence of tools is, again, less than ideal.

### Solutions

"Tool duplication," where the only difference between a handful of tools is what transformations we're composing, could be solved through subcategorization, e.g. "Move Selection > By Gizmo Axis Drag"

The inability to do conditional binds (i.e. move camera if move selection succeeds) without hardcoded composition is a problem, ~~but could be solved in other ways (e.g. ordered operations, at the cost of heap allocation and freeing per frame, rather than merged ones; could pre-allocate, etc., to ease perf burdens)~~. (This doesn't work for "move camera if move selection succeeds," because moving the selection may only *partially* succeed: you may try to move the selection 100 units to the left, but it only goes 70 and then hits some boundary that we can't let it cross e.g. the max coordinate threshold in an interior cell. How do we ferry the amount by which it moved into the "move camera" bind, so that the camera movement matches the selection movement? Again: having tools return results isn't part of the design, much less having one tool take the results of another (i.e. *any other*) tool as input.)

### A note about Worldinput

Worldinput doesn't need a redesign, nor does it need to be replaced with scripting.

I said earlier that GUIs limit the ability to compose things compared to a scripting language. However, the hardcoded composition of inputs offered within Worldinput is already highly flexible thanks to me spending, what, four months? [see next section] on codifying my human intuition into a monstrously complicated set of programmatically enforceable rules. Ditching Worldinput would mean forcing control scheme authors to have to anticipate and account for every single keybind/combination conflict, one by one.

The one thing I don't like about Worldinput is that it's organized around key-ups rather than key-downs. This may make inputs feel less responsive, and it also complicates designing things like distinguishing double-clicks from single-clicks. However, it's the easiest way to distinguish presses from long-presses. I'm tempted to revisit this someday and see what a key-down-oriented system would be like (and whether it'd be an improvement), but that would require writing an entirely new spec and meticulously testing various input cases against it.

(Whether we focus on key-ups or key-downs, there will still be some responsiveness delays wherever press/long-press or press/hold binds conflict. This is fine: responsiveness delays in the case of ambiguous inputs aren't unique to us. To give one example, Windows Explorer has certain cases where a single-click and a double-click will trigger mutually exclusive actions, and they handle this by delaying the single-click action until after the double-click timing window elapses. When a file is already selected, a single-click on its name triggers renaming, while a double-click opens the file.)

#### A history of Worldinput just so I don't forget

Just so I have it in full:

The planning for DovahKit's first system for handling Render Window input, "DK3D," began circa mid-November 2021; implementation efforts began circa December, with my focus alternating between developing input handling and developing DovahKit's 3D renderer. By late February 2022, it was possible to load interior cells with ACTI and STAT forms visible and lit by a single global directional light; and it was possible to fly the camera around in them using a gamepad. (Shadows came in early March.)

Present-day Worldinput has a concept of mapping button combinations to "tools" with options; this basic idea was present back in DK3D. Plans to rename DK3D to "Worldinput" were made in late February 2022, but the renderer remained the primary focus of development. Going solely by where I was keeping planning notes at the time, and *not* by the commit history, only in November 2022 was Worldinput's first incarnation finally reorganized, and at around this time it became the primary focus of development. By December, the notion of storing tool requests in a tuple (coalescing requests of the same type, and avoiding a heap-allocated list that may need to reallocate multiple times) was implemented. Initial GUI design efforts began circa December 2022, with those designs drawing inspiration from AntiMicro's button combo editor.

By January 2023, Worldinput's first incarnation proved fatally flawed:

* In Worldinput's present-day incarnation, control schemes are defined and edited by users as node trees. However, the use of node trees just makes it more convenient to specify button combinations, conditions, and so on. For actual processing, Worldinput flattens all node trees into "bind lists," and processes all of the resulting binds sequentially.
  
  In Worldinput's original design, control schemes were also designed as node trees, but they were processed very differently. Worldinput would keep track of the "current" node, initially the root; when you pressed the keys for a child of the current node, Worldinput would traverse into that child &mdash; becoming blind to everything outside of that node. This was a naive approach to resolving button conflicts, e.g. between S and Ctrl + S (where by being "inside" a node for Ctrl, the "just S" bind would become invisible to Worldinput, being shadowed by any "S" bind inside of the Ctrl node) and between Ctrl + S and Alt + Ctrl + S (similar principle). However, this idea greatly complicated attempts at designing handling for clicking and dragging (only single clicks, to select refs, had been implemented by that time, along with a non-interactive edit gizmo); quote:
  
    "Where all this gets tricky is that if we treat clicking and dragging as a modifier [key/button] and a vector input, then you can't activate any other modifier keys while the operation is in progress. Using Windows Notepad as an example, it'd be as if drag-selecting text prevented you from using the Ctrl + S shortcut."
* It was around this time that I also discovered that some Creation Kit functions (e.g. editing depth bias, scale, etc., by mouse dragging with certain keys held) could be swapped between, seamlessly, by pressing and releasing individual non-modifier keys. That rendered the original node tree concept entirely unsalvageable: these Creation Kit binds would've been utterly impossible to replicate in this system.

These cases helped motivate a redesign of Worldinput (named `worldinput2` until its completion; now the complete and "canonical" "Worldinput") beginning circa early February 2023. Planning happened in earnest (in the form of writing a massive spec) from 2/28/2023 to 5/10/2023 for the bulk of the latest design, with additional work occurring every week or two in September through October 2023.

## UI

* Move `ui::types::log_item` and friends to `dovahkit::subsystems::message_log::...`.

* Investigate replacing `FormSignatureCombobox` and `FormsOfTypeCombobox` with `DKFormPicker`. Where are they used? I know offhand they're used in the Cell View window, and I don't remember if that's because they pre-date the old `FormPicker` or because I wanted a non-async control for simplicity's sake.

  Ideally we'd add a "sync/async" option to `DKFormPicker`, with it filling async by default, and then have sync versions on Cell View and friends just so we have fewer changes to test.

* Investigate adding icons for all of the form types. Investigate having these icons show up in the Object Window.

* `DKGenericListModel` isn't half bad as-is, but for a few issues. We should write a replacement.

  * Because virtual functions like `moveRows` are implemented *and* are used by helper functions that are meant to be exposed to subclasses, there's no (good) way for subclasses to lock those off from the outside world. Perhaps we should have `static constexpr const bool`s for configuration, e.g. `allow_outside_callers_to_move_rows`, and then have the virtual functions conditionally (i.e. based on the configs) wrap private functions that always work.

  * It'd be nice if we could add optional functionality (to be enabled by the subclasses) for self-sorting and self-filtering models, using behavior similar to what we have in `DKRefsInCellModel`.

  * The model only supports rows. I can't think of any use cases where we'd want exclusively columns, but having a `constexpr` `Qt::Orientation` value wouldn't be a terrible idea.

### Object Window

* The code for this is pretty messy just in general. Tempted to do a full rebuild but it's not worth the devtime right now.

### Form UIs

* **Faction:** When closing the dialog via the "OK" button, check if there are multiple reactions to the same faction, or multiple ranks with the same ID. Warn the user that we'll coalesce the two (TODO: coalesce ranks) if they continue, and give them the opportunity to cancel closing the dialog and fix the lists.

## Dovahscript

* Do we really *need* to run this on a worker thread in order to allow the user to force-kill scripts that hit a loop?
  
  * No, really, think about it. It's not safe to just terminate a thread willy-nilly, because you'll fail to free resources, fail to release locks, and so on... which is why we *don't*. We use a feature built into the Lua interpreter -- the instruction count hook -- to make the script periodically pause and check if an "abort" bool has been set. The only time Dovahscript would ever *truly* be unresponsive would be if native code ran into an infinite loop or otherwise stalled out due to being told to do something silly, and if that ever happens, then we're boned even with the alternate thread, because IIRC the main thread won't commit to killing Dovahscript until the script thread acknowledges the abort. So why can't the whole script run on the main thread? It'd remove the need for locking when reading form data.

    * Running on the main thread would mean that Dovahscript and the Render Window would be competing for time each tick.

    * Even with the instruction hook, Lua operations could potentially still "run hot" enough to decrease UI responsiveness on the main thread. This is pure speculation but perhaps that could interfere with the user actually carrying out the UI interactions needed to force-kill an especially busy and processing-heavy script?

### Form accessors and boilerplate

* To the fullest extent possible, accessors to form properties should rely on templates to reduce copy-and-paste boilerplate.

* Getters and setters for a form's parent should rely on templates to reduce boilerplate. Currently, each form capable of having parents (`cell`, `objectreference`, `topic_info`) has to define its own `parent` getters and setters.

  * A complication with this is that the accessors have different names: `cell.parent_world`; `objectreference.parent_cell`; and `topic_info.parent`. (There's also `topic_info.parent_quest`, but that just accesses the parent DIAL form and invokes a getter on that.) Plus, setters aren't always available e.g. I don't think you can set `cell.parent_world`.

### Main thread context ("add-ons")

If we make it possible to run scripts on the main thread at least conditionally, then that would potentially allow us to improve integration between Lua scripts and the rest of the editor. For example, it might be possible to trigger a Lua script from within the Render Window, or have a Lua script "pilot" the Render Window for the user. We could potentially even go the extra mile and add hooks for extending the native GUI with Lua, though since the native GUI is, uh, native, we would obviously only be able to offer the things it *occurs* to us to offer. **This would be the difference between "scripts" and "add-ons."**

Most form windows have bespoke designs, and none of them have menubars, toolbars, or status bars, so it's hard to really envision any points where Lua scripts could extend the windows. Even if we gate Lua GUIs behind a button or something similar, where would we put those buttons?

One potential extension hook would be context menus for forms in the Object Window: allowing Lua add-ons to add context menu entries and handlers for different forms, form types, and so on, which could open Lua-powered dialogs or just run Lua scripts silently in the background.

I'd need a *lot* of detailed use cases for Lua add-ons before I even begin to design a system for them, because committing to the wrong abstraction -- in something meant to be a platform for user-end extensibility -- would be a devastating mistake here.