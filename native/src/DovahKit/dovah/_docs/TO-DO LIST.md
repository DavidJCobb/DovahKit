

This is a longer-term to-do list.

Also refer to comments in `main.cpp`, though many were written years ago...


# Backend

## General

* Consider altering the subclasses of `form_reference_t` to rely on a template. We can avoid a dependency on the form stub header if we move `use_info_entry` to its own file (and perhaps even move the use info entry flags-mask out of that struct and into its own separate file, too).

## Loading
* `add_hardcoded_forms_to_load_order` in `dovah/forms/factories/hardcoded.h` is only used by the file load process and should be passkeyed somehow.
* `build_hardcoded_form_outbound_refs` in `dovah/forms/factories/hardcoded.h` is only used by the file load process and should be passkeyed somehow.
* `instantiate_hardcoded_form` in `dovah/forms/factories/hardcoded.h` is only used by `form_stub` when loading full form data, and should be passkeyed somehow.

## Saving

* Undelete-and-disable all flagged-as-deleted REFR overrides on save? (If so, make it a configurable option; default `false` in the backend; default `true` in the frontend e.g. INI settings.)

  * NAVM too?

## Form data

### Components
* Make the helper functions on `object_bounds` `constexpr`.
* `container_entry::condition` is a struct containing a presence bool and a float; change it to `std::optional<float>`.
* Split `papyrus.h` into a subfolder with different headers for each of the structs used to define an attached ScriptObject.
* Un-nest `script_data::script` and friends; we already have all of it in a namespace i.e. `dovah::loaded_forms::components::papyrus`. Rename `script_data` to `attachment_data` or something.

### ActorBase (NPC_)
* NAM9 (Face Morphs) should not be serialized if all the values are 0.
* NAMA (Face Parts) is not serialized by the CK if the values are unset/default. However, I don't know what the unset/default values are.
* Investigate omitting fields that would be disabled/suppressed by template actor flags; alternatively, have the frontend do that. Main concern with doing it at save time is use info (i.e. unused form uses still *are* form uses and need to be severed bidirectionally).




# Editor subsystems

## Dovahscript

* It'd be worth investigating whether template metaprogramming can cut down on the boilerplate and copying-and-pasting used throughout the Lua API implementations. We could write templates for accessing form properties, which take lambdas or pointers-to-members to perform the requisite access, for example. My only concern is that we might end up annihilating script performance when compiling in Debug, as *nothing* gets inlined; even `__forceinline` is disobeyed.
  * Pointers-to-members would avoid the overhead associated with accessor lambdas, but C++ has a hole in the standard: there's no way to get a pointer-to-member of a nested struct. A template powered by macros and `offsetof` could simulate that capability.


## Worldedit/Worldinput

* The current system for tools is still a bit messy...
  * The `tool_response_tuple` struct should include presence bits for any tools that don't define a `response` type. That way, tools that have no specific response details to offer (e.g. "do this thing, with no parameters" tools, common in debugging) are still representable.
  * Action nodes are capable of having a `nullptr` options-union pointer, which will trigger an assertion failure when the bind list is processed. Can we make it so that an action node will *always* have a valid options pointer, e.g. because it constructs the options-union in its constructor and uses a smart pointer for ownership/overwrites? (It'd have to be a custom smart pointer or a `std::unique_ptr` with a custom deleter, to account for the way we subclass things.)
    * The problem with this is that we don't want to have to include the non-opaque options union header in the action node header and, transitiviely, in every header that ever touches control schemes.
      * But the control scheme header already forward-declares the node types, does it not? So this shouldn't be a problem.
  * We should modify `options_union` to be capable of representing tools that have no `options` struct. Then, we won't need to store the tool ID alongside the `options_union` (which, being a tagged union, already has the tool ID anyway). (We've already indirectly done this by having the tool base class declare an empty `options` type as a band-aid for the above-mentioned assertion failure, but that means that `options_union` now defines dispatch table entries for these no-op structs, which is wasteful. It'd be nice if `options_union`'s type-tag could refer to any tool and we just had functionality in place for tools with no options.)


## Worldinput

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

* If the Object Window is filtered to Actors of a given race, then right-clicking and creating a new Actor should set it to that race by default.

* If the Object Window is filtered to female Actors, then right-clicking and creating a new Actor should flag it as female by default.


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