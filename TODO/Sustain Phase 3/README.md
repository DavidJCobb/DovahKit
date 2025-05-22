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

#### Musings on tools invoked in tandem

If we subcategorize tools into "tools" and, for want of a better word, "archetypes," then potentially a solution exists for running tools in tandem while accounting for partial or complete failures:

* Allow a single bind to host multiple tools, to be run in sequence
* Allow tools to return a result, if they're marked as doing so by the user
* Give tools an archetype that takes [a property of] the last returned result as input (if the types match)

Under this idea, the Move Selection tool could return the position to which the selection was moved, and the vector by which the selection was moved (which could optionally also be accessed as a direction). Then, a Move Camera tool on the same bind could be set to use the vector by which the selection was moved as input.

This does, however, introduce the question of whether the added complexity here (in terms of both implementation and UX/onboarding) is worth it &mdash; whether we get enough practical value out of such a system to merit using it further. Are there many other cases where two tools could or should be run in tandem like this?

Moreover, this way of executing tools feels closer to being an inadequate scripting language than an adequate configuration system. I said above that scripts aren't a good alternative to Worldinput or Worldedit due to the complexity of the 3D transforms involved for many of these tools; but if we're already splitting tools into tools and archetypes, then the logical metaphor (in a text-based programming language) is function overloads:

```c++
auto result = move_selection::simple{
   .frame = reference_frame::world,
   .by    = vec(0, 0, 2)
});
move_camera::simple({
   .frame = reference_frame::world,
   .by    = result.translated_by,
});
```

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
  
  > "Where all this gets tricky is that if we treat clicking and dragging as a modifier [key/button] and a vector input, then you can't activate any other modifier keys while the operation is in progress. Using Windows Notepad as an example, it'd be as if drag-selecting text prevented you from using the Ctrl + S shortcut."
* It was around this time that I also discovered that some Creation Kit functions (e.g. editing depth bias, scale, etc., by mouse dragging with certain keys held) could be swapped between, seamlessly, by pressing and releasing individual non-modifier keys. That rendered the original node tree concept entirely unsalvageable: these Creation Kit binds would've been impossible to replicate in this system.

These cases helped motivate a redesign of Worldinput (named `worldinput2` until its completion; now the complete and "canonical" "Worldinput") beginning circa early February 2023. Planning happened in earnest (in the form of writing a massive spec) from 2/28/2023 to 5/10/2023 for the bulk of the latest design, with additional work occurring every week or two in September through October 2023.