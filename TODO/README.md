
# Sustain plans

I plan on releasing DovahKit as an alpha or beta with core editing functionality implemented for all form types, and then improving it and finishing it post-launch. I'll be putting my long-term plans in this folder, so I can gather them all in one place -- split ideas into separate files by topic.

## General roadmap

* **Phase 1: Rewrite the backend for loaded form data.**  
  Loaded form data is handled very messily, and the "working copy" system forces ephemeral copies of form data to have all the same boilerplate as the persistent stuff. This makes it harder and jankier to write UI-related code. The refactor plans described further below should make it significantly easier to write UI-related code, should allow a lot of cleanup of existing UI-related code, and should lead to higher-quality code in general.
  
  In general, rewriting a software project is a bad idea. In this case, though, a very large amount of the code can just be copied, pasted, and adjusted to match different conventions. In essence, DovahKit does all of the *things* it needs to do, and it does them more-or-less the right *way*; but programming is about not only the things you're doing and the way you're doing them, but also the way you *say* them, the way you *convey them to the machine*, and *that* is where DovahKit goes wrong. Large-scale refactors are risky but [sometimes, you really do need to just rebuild atop more stable foundations and with an updated plan, rather than continuing to extend old structures](http://thecodelesscode.com/case/33).

  This phase is expected to enable, or make it easier to implement, the following features:

  * Ability to right-click in listviews like ActorBase's relationship list, pick a "New" option, and open a form-editing dialog without a pre-existing stub, wherein a Relationship form is created if you click "OK"
  * Flowchart-like editor for dialogue
  * Cleaner code for editing Papyrus bound scripts
    * Currently, `DKPapyrusBoundScriptListPane` can't auto-commit changes to a script directly to a form-working-copy's VMAD, because `DKBoundScriptListModel` only keeps track of the VMAD itself and not the working-copy form (so it can't use `form_reference_t::set`). This means that form-editing dialogs have to manually commit changes (from the UI to the working-copy VMAD) on save. We *could* have the model track the working-copy form in order to allow auto-committing... *or* once the backend rewrite is done and working copies use bare `form_stub*` fields, it'll then be possible to give `DKPapyrusBoundScriptListPane` the ability to optionally auto-commit directly to a non-use-info-managed VMAD.

* **Phase 2: Rewrite the renderer.**  
  The current renderer design is difficult to maintain and not configurable for different use cases. It's not *that* terrible for, like, the third draft of the first 3D renderer I've ever built, but it *is* terrible, and it's not scalable in the ways I need it to be. It's easily the single worst "God object" I've ever written: this is the only time in my entire life that I've had to have *four separate `.cpp` files* for a single header. We need to figure out how best to divide up the renderer's systems and data, and figure out how to specify things like render pass and shader definitions as `constexpr` PODs in order to separate "data" from "code." We also need to make the renderer configurable: right now, it *always* preallocates enough VRAM resources to run the Render Window (i.e. enough to render a large portion of a worldspace), which is beyond excessive for things like previewing a single form or editing an actor's appearance; and this, indeed, is why DovahKit will not ship with a Preview Window.

  This phase is expected to enable:

  * Stable performance when the Render Window is open
  * "Preview" window for any form with a 3D model
  * "Preview" pane shown when editing a form's 3D model (including its texture swaps)
  * Editing ActorBase appearances
    * This is something we'll have to actually lock down on initial release, since having users "fly blind" when editing appearances is straight-up not viable at all.
    * A bonus feature we could offer here, once actor editing in general is available, is the ability to let the user draw a separate "custom facepaint" image file that we automatically bake (i.e. alpha-blend) onto an NPC's exported tintmask.
      * ...And a *very far-future, if ever,* idea would be to let the user paint with the mouse directly onto the 3D render of the actor's face, as one can do in Blender.
  * Improvements to the renderer's accuracy (e.g. support for water, EffectShaders, etc.)
  * "Preview" pane for EffectShaders (akin to the unimplemented one in the FO4 CK)

* **Phase 3: Refactor Worldedit.**  
  Worldedit's "tool" system is close to what we want, but conceptually it's not well-organized. We should divide it into a two-tiered hierarchy: "tools" with "compositions" (maybe we'll call them "archetypes" in user-facing text). Separately from this, we should investigate things like ordered binds (i.e. a flexible list of actions per frame, allowing multiples of the same tool, rather than coalescing/merging/clobbering on a per-tool basis). Potentially in the future that could even enable things like having multiple tools per bind.

  This phase is expected to enable, or make it easier to implement, the following features:

  * Completion of all binds for editing objects
  * Navmesh editing
  * Terrain editing

* **Phase ?: Refactor file loading.**  
  Not sure where in the process to put this one. Maybe part of Phase 1.
  
  `file_load_order` should be renamed to `active_load_order` and made the end product of a load operation, with some temporary "load process" data structure actually handling (and containing the code for) the load operation. Additionally, the classes (and class hierarchy) for actually parsing and loading files is spaghetti.

* **Phase ?: Refactor Dovahscript.**  
  Not sure where in the process to put this one.
  
  Dovahscript currently relies on *lots* of copying and pasting in order to implement the bulk of form access APIs. This means that refactoring how APIs work (e.g. adding new checks or changing internals) will be prohibitively difficult, and adding new APIs involves a lot of boilerplate. Pre-launch, I'm deliberately choosing *not* to fix this, because I don't want to commit to the wrong abstraction or development process. I want to get the APIs mostly feature-complete, if maybe lacking in some robustness and polish, and then -- after all core functionality is working -- look for common patterns and trends, pay careful attention to their edge-cases and exceptions, and *carefully* explore ways to cut down on duplication and boilerplate using metaprogramming.

  This phase is expected to enable, or make it easier to implement, the following features:

  * More ergonomic script APIs, e.g. Dovahscript more consistently allowing you to "copy-assign" data sub-structures between forms or form components rather than having to manually copy individual members across data sub-structures.
  * Potentially, changes to the script engine internals
    * Currently, Dovahscript runs on a worker thread to limit the damage that an infinite loop in a script can do. *[EDIT: That isn't the sole benefit, and a worker thread may not be required to achieve it.]* I have some pie-in-the-sky ideas for optionally letting Dovahscript run on the main thread, which would make it viable to offer APIs that allow for stronger integration into the editor: think "add-ons" rather than "scripts". (**Do not take this as a promise or even a statement of intent,** but some examples of that kind of integration are attach points for adding scripted UI controls into native windows, and real-time scripted control over the Render Window.) This would require being able to configure the script engine's threading model at run-time, and adding *that* would, in turn, require changes to how basically all form access APIs work -- which, again, would be easier with less code duplication throughout Dovahscript.