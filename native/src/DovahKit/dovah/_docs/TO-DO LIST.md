

Also refer to comments in `main.cpp`, though many were written years ago...


# Backend

## General

* Consider altering the subclasses of `form_reference_t` to rely on a template. We can avoid a dependency on the form stub header if we move `use_info_entry` to its own file (and perhaps even move the use info entry flags-mask out of that struct and into its own separate file, too).

## Saving

* Undelete-and-disable all flagged-as-deleted REFR overrides on save? (If so, make it a configurable option; default `false` in the backend; default `true` in the frontend e.g. INI settings.)

  * NAVM too?

## Form data

### ActorBase (NPC_)
* NAM9 (Face Morphs) should not be serialized if all the values are 0.
* NAMA (Face Parts) is not serialized by the CK if the values are unset/default. However, I don't know what the unset/default values are.
* Investigate omitting fields that would be disabled/suppressed by template actor flags; alternatively, have the frontend do that. Main concern with doing it at save time is use info (i.e. unused form uses still *are* form uses and need to be severed bidirectionally).




# Editor subsystems

## Dovahscript

* It'd be worth investigating whether template metaprogramming can cut down on the boilerplate and copying-and-pasting used throughout the Lua API implementations. We could write templates for accessing form properties, which take lambdas or pointers-to-member to perform the requisite access, for example. My only concern is that we might end up annihilating script performance when compiling in Debug, as *nothing* gets inlined; even `__forceinline` is disobeyed.


## Worldinput

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