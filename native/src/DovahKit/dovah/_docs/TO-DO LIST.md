

This is a longer-term to-do list.

Also refer to comments in `main.cpp`, though many were written years ago...


# Backend

## General

* Consider altering the subclasses of `form_reference_t` to rely on a template. We can avoid a dependency on the form stub header if we move `use_info_entry` to its own file (and perhaps even move the use info entry flags-mask out of that struct and into its own separate file, too).

* Need to support expanded ESLs: new SSE versions allow files to define new forms in the range [0x001, 0x7FF]. (Are there any specific constraints, e.g. the new forms cannot be of the same type as hardcoded forms with the same IDs?) Ideally we should support loading these forms, but should only ever use these IDs when no other IDs are available within the active file, as new forms with these IDs crash older versions of the game during plug-in load.

## Loading
* `add_hardcoded_forms_to_load_order` in `dovah/forms/factories/hardcoded.h` is only used by the file load process and should be passkeyed somehow.
* `build_hardcoded_form_outbound_refs` in `dovah/forms/factories/hardcoded.h` is only used by the file load process and should be passkeyed somehow.
* `instantiate_hardcoded_form` in `dovah/forms/factories/hardcoded.h` is only used by `form_stub` when loading full form data, and should be passkeyed somehow.

## Saving

* Undelete-and-disable all flagged-as-deleted REFR overrides on save? (If so, make it a configurable option; default `false` in the backend; default `true` in the frontend e.g. INI settings.)

  * NAVM too?

* **The code to flag refs as persistent (when the CK would deem it appropriate) on save should be moved out of the backend so that we can handle/report failure cases appropriately.** We'd still want some sort of backend functionality, so maybe add a "request" object (akin to form creation/deletion/renumbering/etc. requests, etc.) for bulk-flagging refs as persistent. The "request" would check if there are enough available form IDs to create any persistent cells needed for to-be-flagged refs in worldspaces, failing (i.e. not flagging anything) if there aren't. The frontend could then use this "request" object and display appropriate error messages on save (while still saving, of course!) if flagging all refs as persistent is not possible.

## BSA

* Finish moving all BSA-format-related types into `dovah::bsa`, and rename types as appropriate.
  * `dovah::bsa_archive` -> `dovah::bsa::archive`
  * `dovah::bsa_archived_file` -> `dovah::bsa::packed_file_content`

## Compiled Papyrus scripts

* We should bounds-check all count values, i.e. if a script claims to have 6969 properties we should probably double-check that against the remaining filesize to see if that's at all plausible before we just `std::vector::resize` based on it.
  * We *do* already bounds-check reads of simple primitives and throw an exception if we're going to pass EOF, but that won't save us if the program blows up because someone tricks it into pre-allocating tons of memory *before* those reads.

## Miscellaneous

* `dovah/data/actor_values.h`: this should be a `constexpr` list of PODs. Current way we define it is super old and scuffed (singleton, with a constructor wherein a local C array of definitions that gets `memcpy`'d into a `malloc`'d list).

* `dovah::form_type_info` could be made smaller by using a smaller underlying type for its flags-mask and moving the `formType` constant next to it, i.e. we can cut out three bytes of padding per instance.

## Form data

### Components
* Make the helper functions on `object_bounds` `constexpr`.
* `container_entry::condition` is a struct containing a presence bool and a float; change it to `std::optional<float>`.

### ActorBase (NPC_)
* NAM9 (Face Morphs) should not be serialized if all the values are 0.
* NAMA (Face Parts) is not serialized by the CK if the values are unset/default. However, I don't know what the unset/default values are.
* Investigate omitting fields that would be disabled/suppressed by template actor flags; alternatively, have the frontend do that. Main concern with doing it at save time is use info (i.e. unused form uses still *are* form uses and need to be severed bidirectionally).

### Package (PACK)
* Conditions can refer to a piece of package data on the condition's owning package; they refer to that data by index. If the package is edited and its data is rearranged, how do we find and update all referencing conditions?
  * This isn't as much of a concern as it seems like. The only time conditions have owning packages (and therefore the only time they can refer to package data) is when they exist inside of a `PACK` form. As long as the UI for editing `PACK` forms is carefully implemented to update conditions everywhere in the package when package data are added, removed, or reordered, we should be fine.




# Editor subsystems

## Dovahscript

* It'd be worth investigating whether template metaprogramming can cut down on the boilerplate and copying-and-pasting used throughout the Lua API implementations. We could write templates for accessing form properties, which take lambdas or pointers-to-members to perform the requisite access, for example. My only concern is that we might end up annihilating script performance when compiling in Debug, as *nothing* gets inlined; even `__forceinline` is disobeyed.
  * Pointers-to-members would avoid the overhead associated with accessor lambdas, but C++ has a hole in the standard: there's no way to get a pointer-to-member of a nested struct. A template powered by macros and `offsetof` could simulate that capability.
* Once we're done completely redesigning how Papyrus is handled in the backend and UI, we need to redesign how it's handled within Dovahscript. Right now, all code for this is either commented out or in files that have been excluded from the MSBuild project, since all of that code is out of date.
  * Don't forget quest aliases' script APIs! Aliases have their own VMAD bindings that need to be accessible.


## Worldedit/Worldinput

* **Strongly consider pushing back full Render Window support until post-launch. Strongly consider disabling the creation of new control schemes in Release builds, and disabling all editing tools in Release builds, in preparation for wholly redesigning input handling in a post-launch update. Alternatively, consider leaving these things in for ship but clearly telegraphing that they may be replaced in a future update (we have the latitude to do that since we plan on launching as a beta).**

  Worldinput simply isn't as scalable as I initially believed it would be; I now believe that a scripting API would be a better way to express the various editing operations that should be possible in the Render Window, and the various ways those operations can be composed and configured. I should prototype Lua script APIs for control schemes such that new control schemes can be implemented as scripts relying entirely on event dispatch. **I should not actually implement this, even as a prototype. I should simply write the scripts for all planned control schemes, see what I want the APIs to look like, and see if I discover any edge-cases that a script API would need to deal with.** Only if writing the scripts seems easier than \*gestures wildly at the existing codebase\* all *this* should I commit to deprecating most of Worldinput and the tool system in favor of a scripted approach, with Worldedit directly exposing APIs to Lua.

  * Notably, creating a scripting API for control schemes means creating a scripting API that runs on the main thread. I should do this carefully, and in such a way that a bad script (e.g. with an infinite loop) doesn't softlock the program (i.e. we shouldn't run these scripts immediately upon program startup or even the user's first GUI interaction).
  
    Ideally, we should be able to limit the script to only responding to events, i.e. it shouldn't even get to run "setup" code until the first event (e.g. the Render Window being focused for the first time), so that a faulty script doesn't immediately hang DovahKit on program startup. The drawback to this idea is that you wouldn't be able to write class definitions, etc., because in Lua those are run-time-executed code.

    Perhaps a custom scripting language? Oh, but that should be *hella* post-launch, if we go that route...

  * Counterpoint: things like computing gizmo axis drag are difficult and the average scripter would be helplessly lost without pre-built compositions. Maximum composability means little when most scripters wouldn't know how to compose common tasks.

    The inability to do conditional binds (i.e. move camera if move selection succeeds) without hardcoded composition is a problem, but could be solved in other ways (e.g. ordered operations, at the cost of heap allocation and freeing per frame, rather than merged ones; could pre-alloc, etc., to ease burden). "Tool duplication" where the only difference is composition could be solved through subcategorization e.g. "Move Selection > By Gizmo Axis Drag."

* The current system for tools is still a bit messy...
  * The `tool_response_tuple` struct should include presence bits for any tools that don't define a `response` type. That way, tools that have no specific response details to offer (e.g. "do this thing, with no parameters" tools, common in debugging) are still representable.
  * Action nodes are capable of having a `nullptr` options-union pointer, which will trigger an assertion failure when the bind list is processed. Can we make it so that an action node will *always* have a valid options pointer, e.g. because it constructs the options-union in its constructor and uses a smart pointer for ownership/overwrites? (It'd have to be a custom smart pointer or a `std::unique_ptr` with a custom deleter, to account for the way we subclass things.)
    * The problem with this is that we don't want to have to include the non-opaque options union header in the action node header and, transitiviely, in every header that ever touches control schemes.
      * But the control scheme header already forward-declares the node types, does it not? So this shouldn't be a problem.
  * We should modify `options_union` to be capable of representing tools that have no `options` struct. Then, we won't need to store the tool ID alongside the `options_union` (which, being a tagged union, already has the tool ID anyway). (We've already indirectly done this by having the tool base class declare an empty `options` type as a band-aid for the above-mentioned assertion failure, but that means that `options_union` now defines dispatch table entries for these no-op structs, which is wasteful. It'd be nice if `options_union`'s type-tag could refer to any tool and we just had functionality in place for tools with no options.)


## Worldinput

### "Pick Reference in Render Window"

* These buttons, existing as part of `DKObjectReferencePicker`, need to actually have their behavior implemented.

  * When this functionality in the Creation Kit is activated, most UI interaction is blocked: the Render Window is forced to the top, and you cannot drag it around or focus other Creation Kit (sub)windows. You can cancel the operation by right-clicking anywhere (which won't focus any windows that aren't already focused, but will open the Render Window context menu if you right-click there) or by focusing a different program. The Esc key doesn't cancel the operation.

### QOL
* Additional program-wide options (i.e. not scoped to any single control scheme) for scaling various editing operations' speeds (e.g. translate selection, rotate selection, etc.) when in the Precision or Boost camera modes. The scalars should be configurable.

### Scancodes instead of/alongside VKs

Worldinput uses Win32 virtual keys rather than keyboard hardware scancodes, and this has some limitations. One is that the AltGr key on international keyboards is detected as `[Ctrl + Alt]` rather than as its own key. In the future, it'd be nice if we allowed the user to specify whether a keybind should map to a virtual key (e.g. "the 'A' key no matter what your layout is") or to a physical key position; this would also allow proper handling of keys like AltGr. A guide for doing this can be found [here](https://blog.molecular-matters.com/2011/09/05/properly-handling-keyboard-input/).

### Bind inheritance blocking

It'd be nice if, when defining a modifier node, you could define what upper-level nodes (i.e. nodes outside the modifier) can still be activated, with it defaulting to all of them being activated (the current behavior).

Currently, you can block an outer node from activating by shadowing it with a dummy node inside the modifier (though we don't offer a no-op tool; perhaps we should...). We could automate this. This is super low-prio, though; perhaps not worth implementing at all.




# Frontend

## Features

* When saving a file, if the entered filename already exists (and is not the active file's current name), the user should get a confirmation prompt before overwriting. Make sure this also works when converting across games.

* If the user has any unsaved changes, the main window should show a confirmation prompt on exit. We already override MainWindow::closeEvent; we'll want to do what we need to do in there

  * Checking for unsaved changes is easy: just see if any active file forms are flagged as edited, being sure to ignore non-canonical stubs for singleton forms.

* When converting across games, we should check, pre-save, whether the active file contains any forms or overrides whose types don't exist in the target game. If so, the user should be shown an extra confirmation prompt listing the forms in question. Arguably we should do the same if any such forms in non-active files are used by any forms in the active file.

  * Post-save, the same list of forms should be emitted to the log window, confirming their removal to the user.

* A feature to list all unnecessarily-persistent refs and allow the user to un-flag them in bulk.

## UI polishing

* If the Object Window is filtered to multiple form types, or not filtered at all, then the "New form" context menu item should have a submenu letting the user pick one of the form types to which the Object Window is currently filtered. When the object window isn't filtered, nest this submenu another level, mimicking the categories in the lefthand treeview, so the user doesn't have to scroll through all 120+ form types in the game.

* If the Object Window is filtered to Actors of a given race, then right-clicking and creating a new Actor should set it to that race by default.

* If the Object Window is filtered to female Actors, then right-clicking and creating a new Actor should flag it as female by default.

## Widgets

### CanvasWidget

* Don't abuse Qt's internal object hierarchy for canvas layer ordering. Track canvas layers separately.

  * Audit the `CanvasLayer` widget and its associated classes, and see whether there's any reason for layers and layer groups to be QObject subclasses aside from our abuse of Qt's internal object hierarchy/ownership system. If not, then don't make them be QObjects anymore.

### FormPicker

* When filtering forms under the hood, exterior cells should only be filtered out if they have no editor ID.


# Last-minute verification

Here's ElminsterAU's test procedure for xEdit, as planned for Starfield:

> I still have about 35 record types which haven't been property checked at all yet (over 200 done though), after which comes the test to copy as override every one if the 3.5 million records in Starfield.esm into a new module which has Starfield.esm listed as it's 2nd master (so that the copying must change the stored FormID), making sure all copies show up as ITMs, and finally in game testing with this "everything overridden" esm

We can't run that *exact* procedure for DovahKit and Skyrim because I don't plan on having DovahKit launch with support for every one of Skyrim's 120+ form types. However, we could run this procedure for all supported form types. We'd have to doctor an empty ESP in xEdit to have Skyrim.esm as its second master and a dummy master as the first, but after that, the in-DovahKit test procedure *should* be as simple as using a Lua script to flag as edited every supported form type from Skyrim.esm. Then, we save it, load the game with it, and see if anything breaks. (Could also use xEdit to verify that every record is an ITM, though I don't know how manageable that'll be since Skyrim.esm uses form version numbers other than the most recent whereas IIRC DovahKit will always use the most recent form version; may need to write xEdit scripts to rule out false negatives like those.)

# Post-launch/sustain

## Renderer

### Compile-time configuration

Currently, the renderer doesn't have any compile-time or metaprogrammed configuration. Render passes, shaders, and other constructs exist only in the form of the code which instantiates them at run-time, for example. Scene entity count limits are defined as `constexpr` constants and aren't configurable for individual renderers. Basically, `surface_renderer` is designed for use in the Render Window, and offers no configuration that could be used to adapt it for less intensive uses, e.g. a Preview Window for individual meshes, or a renderer for NPC faces when editing them.

Additionally, because scene entity limits are fixed, we pre-allocate GPU-side buffers large enough to hold everything up to those limits. As of this writing, `surface_renderer` is configured to support up to 32000 meshes in the scene at a time (remember: this is individual meshes within NIFs, not entire NIFs); `rendered_mesh` entities have 112 bytes' worth of shader parameters; each frame-in-flight needs its own copy of these, and we use two frames in flight; so that's (32000 * 112 * 2) = exactly 7KB of VRAM per renderer. Every renderer burns 7KB of VRAM even if it isn't doing anything. That's not a *lot*, but it's more than would be needed for something like a Preview Window.

It'd be nice if I could use C++ templates for different kinds of renderers, with different options and whatnot. I could avoid having to drag absolutely all of the renderer code into a header by using a non-templated base class, with member functions that handle individual actions (e.g. `_instantiate_render_pass(VkRenderPass& out_handle, const render_pass_definition&)`). I could potentially also have an option to let the renderer use dynamic scene entity limits; this would allow, say, a Preview Window to set its limits to exactly the number of entities it would need for the NIF it wants to render.