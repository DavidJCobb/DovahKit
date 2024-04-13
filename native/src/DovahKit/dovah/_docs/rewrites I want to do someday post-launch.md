
# Timeline

* **Phase 1: Rewrite the backend for loaded form data.**  
  Loaded form data is handled very messily, and the "working copy" system forces ephemeral copies of form data to have all the same boilerplate as the persistent stuff. This makes it harder and jankier to write UI-related code. The refactor plans described further below should make it significantly easier to write UI-related code, should allow a lot of cleanup of existing UI-related code, and should lead to higher-quality code in general.

  This phase is expected to enable, or make it easier to implement, the following features:

  * Flowchart-like editor for dialogue
  * Cleaner code for editing Papyrus bound scripts
    * Currently, `DKPapyrusBoundScriptListPane` can't auto-commit changes to a script directly to a form-working-copy's VMAD, because `DKBoundScriptListModel` only keeps track of the VMAD itself and not the working-copy form (so it can't use `form_reference_t::set`). This means that form-editing dialogs have to manually commit changes (from the UI to the working-copy VMAD) on save. We *could* have the model track the working-copy form in order to allow auto-committing... *or* once the backend rewrite is done and working copies use bare `form_stub*` fields, it'll then be possible to give `DKPapyrusBoundScriptListPane` the ability to optionally auto-commit directly to a working-copy VMAD.

* **Phase 2: Rewrite the renderer.**  
  The current renderer design is difficult to maintain and not configurable for different use cases. It's not bad at all for, like, the third time I've ever built a renderer and the first time I've ever meaningfully succeeded, but it's not scalable in the ways I need it to be. It's easily the single worst "God object" I've ever written: this is the only time in my entire life that I've had to have *four separate `.cpp` files* for a single header. We need to figure out how best to divide up the renderer's systems and data, and figure out how to specify things like render pass and shader definitions as `constexpr` PODs in order to separate "data" from "code." We also need to make the renderer configurable: right now, it *always* preallocates enough VRAM resources to run the Render Window (i.e. enough to render a large portion of a worldspace), which is beyond excessive for things like previewing a single form or editing an actor's appearance.

  This phase is expected to enable:

  * "Preview" window for any form with a 3D model
  * "Preview" pane shown when editing a form's 3D model (including its texture swaps)
  * Editing ActorBase appearances
    * This is something we'll have to actually lock down on initial release, since having users "fly blind" when editing appearances is straight-up not viable at all.
    * A bonus feature we could offer here, once actor editing in general is available, is the ability to let the user draw a separate "custom facepaint" PNG file that we automatically bake (i.e. alpha-blend) onto an NPC's exported tintmask.
  * Improvements to the renderer's accuracy (e.g. support for water, EffectShaders, etc.)
  * "Preview" pane for EffectShaders (akin to the unimplemented one in the FO4 CK)

* **Phase 3: Refactor Worldedit.**  
  Worldedit's "tool" system is close to what we want, but conceptually it's not well-organized; see the section below. We should divide this into a two-tiered hierarchy: "tools" with "compositions." Separately from this, we should investigate things like ordered binds (i.e. a flexible list of actions per frame, allowing multiples of the same tool, rather than coalescing/merging/clobbering on a per-tool basis). Potentially in the future that could even enable things like having multiple tools per bind.

  This phase is expected to enable, or make it easier to implement, the following features:

  * Completion of all binds for editing objects
  * Navmesh editing
  * Terrain editing

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

* `dovah`  
  Top-level namespace for the backend.
  * `form_data`  
    Namespace for form data classes -- one per form type, each templated on a `form_data_config` value.
  * `form_components`  
    Namespace for form component classes, like VMAD or DEST; these are things that forms would multiply-inherit from in Bethesda's codebase, though we use composition in ours. Each component type would be templated on a `form_data_config` value.
  * `form_fields`  
    Namespace for common structs that recur across multiple form types, such as the "color" struct used for ACTI/CNAM and others.
  * `forms`  
    Namespace for loaded forms.
  * `form_working_copies`  
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

  Within form data, these custom containers would be conditional types, i.e. `tracked_form_pointer_list` for a "real" form and `std::vector<form_stub*>` for a working copy.

## Get rid of notice codes because they suck and are bad

DovahKit reports all backend warnings and errors via an enum called `dovah::notice_code` and a struct called `dovah::detailed_notice`. The notice codes are similar to WinAPI error codes, and the "detailed notice" struct contains fields for every possible piece of error information that a warning or error could provide.

This sucks. We should instead design this similarly to exceptions -- not in the sense of them being thrown, but rather in the sense of:

* Heap-allocating all "notices"
* Having a common base class
* Having subclasses for specific notice types

This would remove the dependency on a massive enum (such that anything that emits a notice has to be recompiled if we ever edit the list of possible notices) and would also make most notice objects smaller, at the cost of some memory locality for lists of notices. We shouldn't be emitting *that* many notices during the load process, *especially* since we lazy-load forms (and form loading is where the bulk of possible notices during any sort of loading would come from), so taking a few trips to the heap for emitting notices shouldn't slow things down to any noticeabe degree.

### Analysis

Within the backend, it looks like all errors pass through these spots:

* `file_load_order::_log_load_warning` and `file_load_order::_log_save_warning` ferry the passed-in `detailed_notice` to whatever (optional) callbacks have been stored on the load order, though only if the notice isn't empty.
* The classes within `load_order_interfaces` mainly exist as conduits to ferry `detailed_notice`s to the "log warning" functions. There are two additional behaviors, though:
  * `load_order_interfaces::file_load::log_load_warning` stores the warning in a list (if available) on the `file_load_order`'s "save/load state," before invoking the callback.
  * `load_order_interfaces::file_load::log_load_error` doesn't pass the error to any callback; instead, the error is stored on the `file_load_order`'s "save/load state" as an error.

**The best plan for this task, then, would be:**

1. Add member functions to `DovahKitCore` that can take a subclassable "warning" object and a subclassable "error" object. The base classes should be polymorphic (i.e. virtual destructor) both to avoid potential leaks and to allow for `dynamic_cast` and `typeid(instance_ref)`.
1. Add logging callbacks and helper functions to `file_load_order` which work with these new types. Add the requisite functionality to `DovahKitCore` and an overload to `editor_helpers::warning_or_error_to_string` to support handling the new types outside of the backend.
1. Replace all warnings emitted by form loaders and form component loaders with dedicated warning structs. The overwhelming majority of these will be "warn if wrong type," where some given subrecord is pointed at a form of the wrong type (e.g. an Activator specifying a Quest as its water type; that kind of thing), so in practice we can knock out 196 emitted warnings with just a handful of warning types.
   * Any notice codes that are converted from notice-code/detailed-notice to dedicated warning classes can also be removed from the code that prints warnings to the UI, which will be a modest but helpful additional reduction in the number of `detailed_notice` search hits.
1. Modify the `load_order_interfaces` types to support logging the new warning types. Don't add anything for hard errors just yet.
   * Once that's done, we can then start looking for calls to the detailed-notice logging functions and replacing those one by one.
1. Investigate each case where `detailed_notice` is used for hard errors, and look into replacing those. This will be a bit of a challenge since `detailed_notice` can be empty, and several processes store a `detailed_notice` member and only track whether they've encountered an error based on whether that member is still empty. We'll have to handle these case-by-case.

Statistics on `detailed_notice`:

* 684 search hits for the classname across all H and CPP files in the project (using Notepad++ to search)
* Many hits outside of form loaders and form component loaders come from pieces of code setting flags on `detailed_notice` to indicate what informational values are present. The actual number of times we *use* the class will be quite a bit lower.
* 67 hits are just the class definition itself
* 66 hits from `file_load_order` (header and CPP)
* 196 hits come from warnings emitted by form loaders or form component loaders
* 103 hits come from the function that converts `detailed_notice` instances to UI-printable strings, since many error messages check flags to see if certain information is present. (This is something else we can remove the need for by using class hierarchies and inheritance for warnings: a warning of a given type can *require* certain pertinent information to be present, such that its presence need not be checked for.)
* 26 more hits come from UI code to display errors and warnings (e.g. during load, or for the log list window).
* 82 hits come from Qt MOC.
  * 8 hits come from Qt MOC for DovahKitCore, i.e. where `detailed_notice` is a signal parameter.
  * 29 hits come from Qt MOC for `detailed_notice_dispatcher`, which IIRC is used to dispatch notices across threads.
  * 4 hits come from Qt MOC for the log list view.
  * The above are for Debug; duplicates exist for Release.
* 23 hits come from `nifDK`. The NIF loading code uses the same design pattern, but a different class: `nifDK::detailed_notice`. These are used exclusively to report load errors, and never warnings.
  * 1 hit comes from the class definition itself.
  * 11 hits come from the code for loading NIF files (i.e. the `file` class), which tracks whether it's run into an error via a potentially-empty `nifDK::detailed_notice` member.
  * 11 more hits from the NIF `file_reader`, which also has an "error" member.
* That leaves 121 miscellaneous uses in the backend.

Specific places where notice codes are used:

* Data member `basic_reader::last_error`, set by some member functions on the class.
* Data member `file_writer::error`, set internally when errors occur during saving.
  * Related: `tes_file_writing::write_results::error`.
* `file_load_order_normalizer::add(...)`, as a mechanism for returning an error with basic error information. The error is written to via a `detailed_notice&` argument. Notably, this function is recursive, since it has to handle dependencies of dependencies and so on, so it also checks whether the passed-in argument is *already* a logged error.
* `file_header_reader::load`, where an optional pointer-type out-argument is used to signal errors when reading a file header (e.g. filesystem problem; malformed file).
* Logging warnings when a form's full data is invalid; done by each individual form loader.

# Outside the backend

## Worldedit

### Challenge 1: composition

Currently, all Render Window functions are configured via the UI. The challenge with doing things via the UI is that *compositions* of transformations, rotations, etc., have to be expressable in a GUI form, which in practice means that we have to hardcode every possible combination.

This means that to support moving a selection by fixed distances in directions, or moving by dragging an edit gizmo, for example, we need either one monstrously complicated set of options, or two separate "move selection" tools adapted for each of those scenarios. The ways we're composing reference frames, inputs, and screen state, to get a final vector to move the selection along, are just too different between the two use cases. Rotating a selection is even more complex because in addition to those two cases, we have a third. In <i>Halo: Reach</i>'s Forge controls, rotation is world-relative but the three input axes for it (split across two joysticks) map to camera axes. For any given input axis, the game finds the world axis that is the closest to being parallel to the mapped camera axis, and rotates the selection about that world axis. The result of this is highly intuitive from a UX perspective, but this is yet another way of composing transformations that we have to hardcode.

A GUI simply cannot represent entirely arbitrary compositions of transformations, of vector and rotation primitives, whereas a script can; and so it's tempting to scrap most of Worldedit's tool system in favor of defining a main-thread script API (whether using Lua or something custom-built). However, scripting many of these controls would be *well beyond* 95% of users; the <i>Halo: Reach</i> behavior above, for example, requires not merely a strong understanding of 3D transformations but also a keen enough eye to work out the precise behaviors by testing in <i>Reach</i>. Things like gizmo axis drag would be similarly difficult.

### Challenge 2: conditions

Another problem with the current system is that there's no way to do conditional binds, e.g. "Attempt to move the selection, and only if that movement succeeds, move the camera commensurately," which is the movement behavior in <i>Halo: Reach</i> when an object is selected. This is because we minimize (and often avoid) heap allocation within the control system by relying on a tuple of "requests," one per tool; when the same tool is invoked multiple times within a single frame, we either merge all of its requests into one, or have one supersede all of the others. This in turn means that there can be no ordering between tools &mdash; no way to *know* that one tool was definitely activated before another.

The current workaround is yet more hardcoded position: an "also move camera" checkbox on the "move selection" tool, for example.

This is another issue that a scripting language could solve, but forcing end users to work out (and often recreate) the full logic for every single tool and sequence of tools is, again, less than ideal.

### Solutions

"Tool duplication," where the only difference between a handful of tools is what transformations we're composing, could be solved through subcategorization, e.g. "Move Selection > By Gizmo Axis Drag"

The inability to do conditional binds (i.e. move camera if move selection succeeds) without hardcoded composition is a problem, but could be solved in other ways (e.g. ordered operations, at the cost of heap allocation and freeing per frame, rather than merged ones; could pre-allocate, etc., to ease perf burdens).

### A note about Worldinput

Worldinput doesn't need a redesign, nor does it need to be replaced with scripting.

I said earlier that GUIs limit the ability to compose things compared to a scripting language. However, the hardcoded composition of inputs offered within Worldinput is already highly flexible thanks to me spending, what, four months? [1] on codifying my human intuition into a monstrously complicated set of programmatically enforceable rules. Ditching Worldinput would mean forcing control scheme authors to have to anticipate and account for every single keybind/combination conflict, one by one.

#### A history of Worldinput just so I don't forget

Just so I have it in full:

The planning for DovahKit's first system for handling Render Window input, "DK3D," began circa mid-November 2021; implementation efforts began circa December, with my focus alternating between developing input handling and developing DovahKit's 3D renderer. By late February 2022, it was possible to load interior cells with ACTI and STAT forms visible and lit by a single global directional light; and it was possible to fly the camera around in them using a gamepad. (Shadows came in early March.)

Present-day Worldinput has a concept of mapping button combinations to "tools" with options; this basic idea was present back in DK3D. Plans to rename DK3D to "Worldinput" were made in late February 2022, but the renderer remained the primary focus of development. Going solely by where I was keeping planning notes at the time, and *not* by the commit history, only in November 2022 was Worldinput's first incarnation finally reorganized, and at around this time it became the primary focus of development. By December, the notion of storing tool requests in a tuple was implemented. Initial GUI design efforts began circa December 2022, with those designs drawing inspiration from AntiMicro's button combo editor.

By January 2023, Worldinput's first incarnation proved fatally flawed:

* In Worldinput's present-day incarnation, control schemes are defined and edited by users as node trees. However, the use of node trees just makes it more convenient to specify button combinations, conditions, and so on. For actual processing, Worldinput flattens all node trees into "bind lists," and processes all of the resulting binds sequentially.
  
  In Worldinput's original design, control schemes were also designed as node trees, but they were processed very differently. Worldinput would keep track of the "current" node, initially the root; when you pressed the keys for a child of the current node, Worldinput would traverse into that child &mdash; becoming blind to everything outside of that node. This was a naive approach to resolving button conflicts, e.g. between S and Ctrl + S (where by being "inside" a node for Ctrl, the "just S" bind would become invisible to Worldinput, being shadowed by any "S" bind inside of the Ctrl node) and between Ctrl + S and Alt + Ctrl + S (similar principle). However, this idea greatly complicated attempts at designing handling for clicking and dragging (only single clicks, to select refs, had been implemented by that time, along with a non-interactive edit gizmo); quote:
  
    "Where all this gets tricky is that if we treat clicking and dragging as a modifier [key/button] and a vector input, then you can't activate any other modifier keys while the operation is in progress. Using Windows Notepad as an example, it'd be as if drag-selecting text prevented you from using the Ctrl + S shortcut."
* It was around this time that I also discovered that some Creation Kit functions (e.g. editing depth bias, scale, etc., by mouse dragging with certain keys held) could be swapped between, seamlessly, by pressing and releasing individual non-modifier keys. That rendered the original node tree concept entirely unsalvageable: these Creation Kit binds would've been utterly impossible to replicate in this system.

These cases helped motivate a redesign of Worldinput (named `worldinput2` until its completion; now the complete and "canonical" "Worldinput") beginning circa early February 2023. Planning happened in earnest (in the form of writing a spec) from 2/28/2023 to 5/10/2023 for the bulk of the latest design, with additional work occurring every week or two in September through October 2023.

## UI

* Investigate replacing `FormSignatureCombobox` and `FormsOfTypeCombobox` with `DKFormPicker`. Where are they used? I know offhand they're used in the Cell View window, and I don't remember if that's because they pre-date the old `FormPicker` or because I wanted a non-async control for simplicity's sake.

  Ideally we'd add a "sync/async" option to `DKFormPicker`, with it filling async by default, and then have sync versions on Cell View and friends just so we have fewer changes to test.

* Investigate adding icons for all of the form types. Investigate having these icons show up in the `DKFormPicker`.

* `DKGenericListModel` isn't half bad as-is, but for a few issues. We should write a replacement.

  * Because virtual functions like `moveRows` are implemented *and* are used by helper functions that are meant to be exposed to subclasses, there's no (good) way for subclasses to lock those off from the outside world. Perhaps we should have `static constexpr const bool`s for configuration, e.g. `allow_outside_callers_to_move_rows`, and then have the virtual functions conditionally (i.e. based on the configs) wrap private functions that always work.

  * It'd be nice if we could add optional functionality (to be enabled by the subclasses) for self-sorting and self-filtering models, using behavior similar to what we have in `DKRefsInCellModel`.

  * The model only supports rows. I can't think of any use cases where we'd want exclusively columns, but having a `constexpr` `Qt::Orientation` value wouldn't be a terrible idea.

### Object Window

* The code for this is pretty messy just in general. Tempted to do a full rebuild but it's not worth the devtime right now.

## Dovahscript

### Form accessors and boilerplate

* To the fullest extent possible, accessors to form properties should rely on templates to reduce copy-and-paste boilerplate.

* Getters and setters for a form's parent should rely on templates to reduce boilerplate. Currently, each form capable of having parents (`cell`, `objectreference`, `topic_info`) has to define its own `parent` getters and setters.

  * A complication with this is that the accessors have different names: `cell.parent_world`; `objectreference.parent_cell`; and `topic_info.parent`. (There's also `topic_info.parent_quest`, but that just accesses the parent DIAL form and invokes a getter on that.) Plus, setters aren't always available e.g. I don't think you can set `cell.parent_world`.