
# Reference

Forms can define fully "abstract" data, such as collections of NPC AI parameters, but they can also define objects that can be placed in the game world, and placements of those objects. Bethesda conceptualizes the latter two kinds of forms as <dfn>objects</dfn> (`TESObject` under the hood), and as <dfn>references</dfn> to objects. The term "reference" is extremely ambiguous and confusing here, so within DovahKit's documentation, the less ambiguous diminutive "ref" is more commonly used.

A "ref," then, is a physical object existing in the game world. It has a "base object" or "base form" which defines its model and other behaviors. A kind of treasure chest is a base form; and a specific treasure chest of that kind, placed in a specific area, is a ref.

Skyrim loads and unloads refs dynamically during play. Some refs are flagged as <dfn>persistent</dfn>, meaning that the game engine keeps them loaded at all times. Other refs are <dfn>promoted</dfn> to persistent by systems such as the Papyrus script engine, meaning that they're kept loaded until they cease to be promoted. Otherwise, refs are loaded on demand when you visit the cells [or locations](./../a/actor%20base.md#Unique%20actors) in which they are located, and they're kept loaded until you leave those areas and no game systems are using them anymore. The latter condition is managed via a "ref handle" system, akin to smart pointers. The game supports about a million ref handles at a time, and handles are given to both persistent and non-persistent refs whenever they're in memory.

## Specific data

### Load doors

A pair of load doors each have teleport data (`ExtraTeleport`) linking the doors together. The teleport data includes the absolute position and rotation of a <dfn>teleport marker</dfn> indicating where an actor will be moved upon using the door: each door's teleport marker pertains to the opposite door's containing cell or world.

Load doors must be linked to the navmesh; refer to [Navmesh form documentation](./../n/navmesh.md) for information.

## Notes

### Edge-cases

* When a ref is loaded from a non-master-flagged file, the ref is forced to persistent regardless of whether its record in that file is flagged as persistent. Among other things, this means that very large-scale mods (i.e. "new lands" mods) need to be master-flagged to avoid blowing the ref handle budget.

* At run-time, every loaded form maintains a source file array listing the files which defined the form. However, for refs (`TESObjectREFR`), only persistent refs are loaded and retained at game startup; all other refs are loaded on demand based on the parent cell or relevant location form.

  This leads to some quirks. If an originally non-persistent ref is made persistent by an override, then only that override and any subsequent overrides will be kept in the ref's source file list. This is because the non-persistent base record is skipped at game startup, while the persistent override is loaded and retained at game startup; from that point onward, the `TESObjectREFR` remains in memory, so the game loads its data directly rather, than relying entirely on its parent cell to find all applicable records for the ref. By the same logic, if an originally persistent ref has a non-persistent override, that override will be skipped. This means that when a ref has records in multiple game files, not all of those records will influence the ref, even in cases where data is coalesced across files i.e. even when data doesn't follow the Rule of One. (TODO: *is* any data for refs coalesced across files?)

  Note that this only happens for non-persistent ref records defined in a master-flagged file (i.e. not a bare ESP). When a `REFR` record (or subclass) is loaded from an ESP file at game startup, it is forcibly flagged as persistent at run-time.

  * I'm not clear how any of this would be affected by a non-persistent ref being retained on startup by virtue of a ref handle being created for it. (That is: if another form is loaded and retained, and that other form stores a handle to the ref, then the ref would be kept loaded even if it isn't flagged as persistent. This other form could be a persistent ref that uses the non-persistent ref in extra-data, or hypothetically it could be something else &mdash; a Location form's ref lists, perhaps; I'd need to check that.) The behavior here hinges on when a `TESObjectREFR` instance is loaded and retained, relative to when any of its non-persistent records are initially parsed at game startup.

  * As of this writing, DovahKit does not emulate this quirk.