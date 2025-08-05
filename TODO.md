
# To-Do

Stopped working on DovahKit for a few months, to tackle other projects (in part motivated by IRL stuff), so here's the short(ish)-term to-do list that I had at the time so I can find it when I return.

## Table of contents

* Sound Descriptor and DKGameFilePicker
* Quest editing
* Immediate next steps
* General form work
* Backend
* 

## Sound Descriptor and DKGameFilePicker
`DKGameFilePicker` is jankily designed, and that's causing us to mishandle file paths in our UI, when adding new ones to a Sound Descriptor. It also means we can't "trap" the user within specific folders (e.g. `Data/Sound/`).

We should redesign `DKGameFilePicker` to work more sensibly.

## Quest editing

I was in the middle of working on scene editing at the time I switched to other projects. There were some low-prio things left to do for dialogue (e.g. transplanting forms across different pseudo-parents and parents) but scenes were the main focus.

* Dialogue editing
  * Info tableview: flags: consider a custom item delegate to draw them as multiple icons.
  * Info tableview: flags: consider an expanded tooltip that lists the flags out in full, with line breaks and rich text.
  * Transplantation
    * ℹ️ QuestAllDialogueDatastore has partial code to handle transplantation. The code for transplanted DIALs is janky and not well-thought-out, and probably doesn't work. We'd need to get this working before we can think about adding any UI for this.
    * ℹ️ An INFO whose conditions run on aliases of the owning quest cannot be safely transplanted across quests; thus also to its containing DIAL. We must check for such conditions, show a list of any INFOs which have them, and allow the user to either clear the relevant conditions en masse, reset those conditions' run-on target to Subject en masse, or cancel the transplantation.
    * ℹ️ A SharedInfo cannot be safely transplanted out of its containing "shared info"-category DIAL nor its containing quest unless no other INFOs reference it.
      * ℹ️ The latter implies that its containing DIAL cannot be safely transplanted (or have its category changed) either.
    * Moving DLBRs across quests
    * Moving top-level DIALs across quests
    * Moving branched DIALs across DLBRs
    * Moving INFOs across DIALs
      * When implementing dialogue transplantation, do not allow an INFO to be transplanted out of an IDAT topic if it's actively being used as a SharedInfo by another INFO.
      * ℹ️ When transplanting an INFO, we may need to manually flag the destination DIAL as edited, to ensure it gets saved to the file -- and the moved INFO with it.
* Scene editing
  * Test adding and deleting phases, actors, and actions.
  * Test editing phases' and actions' properties.
  * We need a better way to delete actors.

## Immediate next steps

* Rename the `shader_particle_geometry_data` form type to `shader_particle_geometry`. It's a noun: it's a geometry (a 3D cube) filled with particles drawn via a special shader.
* Modify the DKFormInventoryWidget: Add the ability to hide ExtraData-related widgets (ownership, health). Add the ability to disallow leveled items, and hide the "Preview Calculated Result" widgets when leveled items are disallowed. We need these features for Constructible Object forms, which use TESContainer for their crafting ingredients but shouldn't allow leveled items.
* Investigate adding a backend component, and a reusable UI widget and/or dialog, for editing the list of magic effects attached to a Spell/Potion/Enchantment/Ingredient. (The common base class used by the game engine for these forms is `MagicItem`. We'll... probably want a better name. Maybe it can literally just be `magic_effect_list` for the backend and `DKMagicEffectListWidget`/`DKMagicEffectEditDialog` for the UI.)

## Forms in general

* Reportedly, ActorBases with no name can't be interacted with to initiate dialogue. Verify this, also test whether such actors become interactable if an alias renames them, test whether a Short Name but no Full Name is interactable, and update the What's This? text for the ActorBase name field accordingly.
* Bulk editing and re-save tests:
  * AMMO (Ammo)
  * ANIO (Animation Object)
  * COLL (Collision Layer)
  * EXPL (Explosion)
  * HAZD (Hazard)
  * WEAP (Weapon)
* UI designs lacking an implementation:
  * ARMO (Armor)
  * BOOK (Book)
  * CAMS (CameraShot)
  * CSTY (CombatStyle)
  * COBJ (ConstructibleObject)
  * FURN (Furniture)
  * LSCR (Loading Screen)
  * MATO (Material Object)
  * MATT (Material Type)
  * MUST (Music Track)
  * MUSC (Music Type)
  * PROJ (Projectile)
  * REFR (Reference)
  * REVB (Reverb Parameters)
  * SPGD (Shader Particle Geometry Definition)
  * SOUN (Sound Marker)
  * SNCT (Sound Category)
  * SNDR (Sound Descriptor)
  * SOPM (Sound Output Model)
  * SPEL (Spell)
  * TACT (Talking Activator)
  * TREE (Tree)
* Form types with incomplete/placeholder backends:
  * LCTN (Location)
  * MGEF (Magic Effect)
  * PACK (Package)

### Table

* **⬛:** Not implemented
* **🟥:** In progress, but blocked by a requirement
* **🟨:** In progress (implementation incomplete)
  * e.g. for UI, this means it's been built in Qt Designer but there's no C++ implementation backing it
* **🟩:** In progress (core tasks done)
* **✅:** Fully implemented (AFAIK; does not include testing)

Backend refers to the existence of a form data class within the `dovah` folder. UI refers to the UI code -- not just the Qt Designer UI file, but the C++ code to make it work. Dovahscript refers to the Lua script bindings, though those aren't going to be a major focus anymore until post-launch.

Package UI would benefit from a system for the variable UIs that appear for different packdata value types.

ImpactData and TextureSet both have an optional decal data component; I think we'd benefit from abstracting the UI for them into a reusable widget. Said widget should support both a vertical layout (ImpactData) and a horizontal one (TextureSet), and should be able to store and return a whole component verbatim (since this form component doesn't have any outbound uses and so doesn't need to go through the use info management boilerplate).

| CK Category | FourCC | Form type | Backend | UI | Dovahscript | Details |
| :- | :- | :- | :-: | :-: | :-: | :- |
| Actors | NPC_ | ActorBase        | ✅ | ✅ | ⬛ |
| Actors | AACT | Action           | ✅ | ✅ | ⬛ |
| Actors | BPDT | BodyPartData     | ✅ | ✅ | ⬛ |
| Actors | LVLN | LeveledCharacter | ✅ | ✅ | ⬛ |
| Actors | PERK | Perk             | ✅ | ⬛ | ⬛ | UI for entry points needs special care |
| Actors | TACT | TalkingActivator | ✅ | 🟨 | ⬛ |
| Audio | ACSP | Acoustic Space     | ✅ | ✅ | ⬛ |
| Audio | MUST | Music Track        | ✅ | 🟨 | ⬛ |
| Audio | MUSC | Music Type         | ✅ | ✅ | ⬛ |
| Audio | REVB | Reverb Parameters  | ✅ | ✅ | ⬛ |
| Audio | SNCT | Sound Category     | ✅ | ✅ | ⬛ |
| Audio | SNDR | Sound Descriptor   | ✅ | ✅ | ⬛ |
| Audio | SOUN | Sound Marker       | ✅ | ✅ | ⬛ |
| Audio | SOPM | Sound Output Model | ✅ | ✅ | ⬛ |
| Character | ASTP | Association Type | ✅ | ✅ | ⬛ |
| Character | CLAS | Class            | ✅ | ✅ | ⬛ |
| Character | EQUP | Equip Slot       | ✅ | ✅ | ⬛ |
| Character | FACT | Faction          | ✅ | ✅ | ⬛ |
| Character | HDPT | HeadPart         | ✅ | ✅ | ⬛ |
| Character | MOVT | Movement Type    | ✅ | ✅ | ⬛ |
| Character | PACK | Package          | ✅ | 🟨 | ⬛ | Once the UI's done, test editing conditions with packdata run-on/params. |
| Character | QUST | Quest            | ✅ | 🟩 | 🟨 |
| Character | RACE | Race             | ✅ | ✅ | ⬛ |
| Character | RELA | Relationship     | ✅ | ✅ | ⬛ |
| Character | SMEN | SM Event Node    | ⬛ | ⬛ | ⬛ | How do the game and CK cope with multiple forms for a single event type? |
| Character | VTYP | Voicetype        | ✅ | ✅ | ✅ |
| Items | AMMO | Ammo           | ✅ | ✅ | ⬛ |
| Items | ARMO | Armor          | ✅ | 🟨 | ⬛ |
| Items | ARMA | ArmorAddon     | ✅ | ⬛ | ⬛ | Race has a checkbox list for equip types. Probably should make that a reusable widget i.e. `DKFormsCheckboxList`, and use it here for Additional Races. |
| Items | BOOK | Book           | ✅ | 🟨 | ⬛ |
| Items | COBJ | Constructible Object | ✅ | 🟨 | ⬛ |
| Items | INGR | Ingredient     | ✅ | ✅ | ⬛ |
| Items | KEYM | Key            | ✅ | ✅ | ⬛ |
| Items | LVLI | LeveledItem    | ✅ | ✅ | ⬛ |
| Items | MISC | MiscItem       | ✅ | ✅ | ⬛ |
| ~~Items~~ | NOTE | Note       | ✅ | ✅ | ⬛ |
| Items | OTFT | Outfit         | ✅ | ✅ | ⬛ |
| Items | SLGM | Soul Gem       | ✅ | ✅ | ⬛ |
| Items | WEAP | Weapon         | ✅ | ✅ | ⬛ |
| Magic | DUAL | Dual Cast Data | ✅ | ✅ | ⬛ |
| Magic | ENCH | Enchantment    | ✅ | ✅ | ⬛ |
| Magic | LVSP | LeveledSpell   | ✅ | ✅ | ⬛ |
| Magic | MGEF | Magic Effect   | ✅ | ✅ | ⬛ |
| Magic | ALCH | Potion         | ✅ | ✅ | ⬛ |
| Magic | SCRL | Scroll         | ✅ | ✅ | ⬛ |
| Magic | SHOU | Shout          | ✅ | ✅ | ✅ |
| Magic | SPEL | Spell          | ✅ | ✅ | ⬛ |
| Magic | WOOP | Word of Power  | ✅ | ✅ | ✅ |
| Miscellaneous | ANIO | AnimObject      | ✅ | ✅ | ⬛ |
| Miscellaneous | ARTO | ArtObject       | ✅ | ✅ | ⬛ |
| Miscellaneous | COLL | Collision Layer | ✅ | ✅ | ⬛ |
| Miscellaneous | CLFM | ColorForm       | ✅ | ✅ | ⬛ |
| Miscellaneous | CSTY | CombatStyle     | ✅ | ✅ | ⬛ |
| Miscellaneous | FLST | FormList        | ✅ | ✅ | ✅ |
| Miscellaneous | GLOB | Global          | ✅ | ✅ | ⬛ |
| Miscellaneous | IDLM | IdleMarker      | ✅ | 🟨 | ⬛ |
| Miscellaneous | KYWD | Keyword         | ✅ | ✅ | ⬛ |
| Miscellaneous | LTEX | LandTexture     | ✅ | ✅ | 🟨 |
| Miscellaneous | LSCR | LoadScreen      | ✅ | 🟨 | ⬛ |
| Miscellaneous | MATO | Material Object | 🟥 | 🟥 | ⬛ | Impossible to complete until we have the ability to save NIF files. We'll get to that during sustain. |
| Miscellaneous | MESG | Message         | ✅ | 🟨 | ⬛ |
| Miscellaneous | TXST | TextureSet      | ✅ | ✅ | ✅ |
| SpecialEffect | ADDN | AddOnNode       | ✅ | ✅ | ⬛ |
| SpecialEffect | CAMS | CameraShot      | ✅ | ✅ | ⬛ |
| SpecialEffect | DEBR | Debris          | ✅ | 🟨 | ⬛ |
| SpecialEffect | EFSH | EffectShader    | ✅ | ✅ | ⬛ |
| SpecialEffect | EXPL | Explosion       | ✅ | ✅ | ⬛ |
| SpecialEffect | FSTP | Footstep        | ✅ | ✅ | ⬛ |
| SpecialEffect | FSTS | Footstep Set    | ✅ | ⬛ | ⬛ |
| SpecialEffect | HAZD | Hazard          | ✅ | ✅ | ⬛ |
| SpecialEffect | IMGS | Imagespace      | ✅ | ⬛ | ⬛ |
| SpecialEffect | IMAD | Imagespace Modifier | ✅ | ⬛ | ⬛ |
| SpecialEffect | IPCT | ImpactData      | ✅ | ✅ | ⬛ |
| SpecialEffect | IPDS | ImpactDataSet   | ✅ | 🟨 | ⬛ |
| SpecialEffect | MATT | Material Type   | ✅ | ✅ | ⬛ |
| SpecialEffect | PROJ | Projectile      | ✅ | 🟨 | ⬛ |
| WorldData | CLMT | Climate                  | ✅ | ⬛ | ⬛ |
| WorldData | ECZN | Encounter Zone           | ✅ | ⬛ | ⬛ |
| WorldData | LGTM | Lighting Template        | ✅ | ⬛ | ⬛ |
| WorldData | LCTN | Location                 | 🟨 | ⬛ | ⬛ |
| WorldData | LCRT | Location Ref Type        | ✅ | ✅ | ⬛ |
| WorldData | SPGD | Shader Particle Geometry | ✅ | 🟨 | ⬛ |
| WorldData | RFCT | Visual Effect            | ✅ | ✅ | ⬛ |
| WorldData | WATR | WaterType                | ✅ | ⬛ | ⬛ |
| WorldData | WTHR | Weather                  | ✅ | ⬛ | ⬛ |
| WorldObjects | ACTI | Activator     | ✅ | ✅ | ⬛ |
| WorldObjects | CONT | Container     | ✅ | ✅ | ⬛ |
| WorldObjects | DOOR | Door          | ✅ | ✅ | ⬛ |
| WorldObjects | FLOR | Flora         | ✅ | ✅ | ⬛ |
| WorldObjects | FURN | Furniture     | ✅ | 🟨 | ⬛ | Will need to be able to extract `FRN` data from the NIF |
| WorldObjects | GRAS | Grass         | ✅ | ✅ | ⬛ |
| WorldObjects | LIGH | Light         | ✅ | ✅ | ⬛ |
| WorldObjects | MSTT | MovableStatic | ✅ | 🟨 | ⬛ |
| WorldObjects | STAT | Static        | ✅ | ✅ | 🟨 |
| WorldObjects | SCOL | Static Collection | ✅ | ⬛ | ⬛ |
| WorldObjects | TREE | Tree          | ✅ | 🟨 | ⬛ |
| Menu: Gameplay | CPTH | Camera Path | ⬛ | ⬛ | ⬛ |
| Dialogue | DLBR | Dialogue Branch | ✅ | ✅ | ⬛ |
| Dialogue | DIAL | Dialogue Topic  | ✅ | ✅ | 🟨 |
| Dialogue | DLVW | Dialogue View   |
| Dialogue | INFO | TopicInfo       | ✅ | ✅ | 🟨 |
| Cell Children | LAND | Landscape | ✅ | ⬛ | 🟨 |
| Cell Children | NAVM | Navmesh | ⬛ | ⬛ | ⬛ |
| Cell Children | REFR | Reference | ✅ | 🟨 | 🟨 |
| Singletons | DOBJ | Default Object Manager | ✅ | ✅ | ⬛ |
| Singletons | NAVI | Navmesh Info Map | 🟩 |   | ⬛ | Can load and re-save; can't update/regenerate. |
| | AVIF | Actor Value | ✅ | 🟨 | ⬛ | We'll also need a custom widget and dialog for Perk Trees. |
| | CELL | Cell | ✅ | ✅ | 🟨 |
| | IDLE | Idle Animation | ✅ | 🟨 | ⬛ | Not a per-form dialog, but a shared dialog for all forms of this type. |
| | REGN | Region | ⬛ | ⬛ | ⬛ |
| | SCEN | Scene | ✅ | ✅ | ⬛ |
| | SMBN | Story Manager Branch Node | ⬛ | ⬛ | ⬛ |
| | SMQN | Story Manager Quest Node | ⬛ | ⬛ | ⬛ |
| | WRLD | Worldspace | ✅ | ⬛ | 🟨 |
| Deprecated | APPA | BGSApparatus | ✅ | ⬛ | ⬛ | CK32 has a loader for this; test in-game behavior |
| Deprecated | EYES | TESEyes | ✅ | ⬛ | ⬛ | CK32 has a loader for this |
| Deprecated | MICO | BGSMenuIcon | ✅ | ⬛ | ⬛ | CK32 has a loader for this |
| Deprecated | RGDL | BGSRagdoll | ✅ | ⬛ | ⬛ | CK32 has a loader for this |
| Skyrim Special | LENS | Lens Flare | ⬛ | ⬛ | ⬛ |
| Skyrim Special | VOLI | Volumetric Lighting | ⬛ | ⬛ | ⬛ |

Subclasses of `REFR`, such as `ACHR`, are not listed in the table above, as they should be loaded identically and have identical data. There are 9 such form types.

The "deprecated" form types don't appear to be loaded by the game at all, but are loaded by the CK.

Static Collections are `BGSStaticCollection` with form type 0x23. The CK has a full loader for them, but never creates them other than via the form-loading factory AFAICT.

### Planned next steps

* PACK
  * Once we can load Package forms and package data, we'll need to go back and update the condition system. We currently handle all "package data" parameters as a single type, but the game actually defines multiple types: package data (possibly null); package data (numeric); and just "package data." We can check the condition/console command table to get parameter types for any functions that take a package data, in order to refine things further.
* LCTN
* MGEF
* SPEL
* WRLD form UI
* REFR form UI
* PROJ

## Backend

* Does TES4/ONAM need to list injected records? Our last fix to `file_load_order::for_each_active_file_override_of_type` will prevent injected records from showing up in TES4/ONAM.

* Test converting files from SSE to LE -- both a file that has forms in the hardcoded range, and a file that doesn't.

### Character encodings

Some (all?) game languages were reportedly switched to UTF-8 for SE, and the CK64 doesn't convert text properly. We use LE encodings for both LE and SE. We need to figure out what encodings to use for SE, *and* make sure we perform encoding conversions properly when going from LE to SE or from SE to LE.

CKPE fixes LE-to-SE encoding conversions for CK64, so we can trust it as a reference for what encodings have changed. Apparently Cyrillic is a known case.

### BEES-range form IDs

SSE: Files with header versions below 1.71 need to warn on records with form IDs in the range xx000001 to xx0007FF whenever xx is non-zero. SSE made it so that new files can define new forms in that range, but it also applies the behavior retroactively rather than checking the HEDR version, so older files with hardcoded overrides that relied on this quirk will be loaded improperly by the game (defining new forms instead).

AFAIK, the CK never should've produced records overrides with that quirk, but who knows what community tools (e.g. clumsily performed mod merges, maybe?) might've done. Records of that variety shouldn't exist so if they do exist, in pre-1.71 files, then they almost certainly rely on pre-1.71 behavior and we should warn that they'll use post-1.71 behavior in-game. (I believe we do the HEDR check that the game doesn't, so we should apply pre-1.71 behavior to pre-1.71 files.)


## Long-term

### General UI

* Turns out, QAbstractSpinBox implements 90% of widget rendering, and can handle any value type that is representable in a QVariant. We should see if we have any spinboxes hooked up to a `uint32_t` and if so, we should create a DKSpinBoxU32 designable widget that mirrors the QSpinBox interface but uses a `uint32_t` for its value type. (QSpinBox uses `int` i.e. `int32_t`, so it can't represent the upper half of a `uint32_t`'s range.)

* The "detection sound level" combobox appears in enough places that we may as well make it a reusable widget. We did that for navmesh generation options, and those don't appear in nearly as many (or as diverse) places.

* DKHeaderView: Bug: If the total width of all columns is wider than the containing view, then you can resize a colum and enlarge it properly. However, attempting to shrink a column causes glitchy behavior: the column size shrinks by an unpredictable amount, and the table and header become visually desynched until you force the table rows to re-render (e.g. by changing your selection).


### `NavMeshInfoMap`

#### Loading perf
`NavMeshInfoMap` is massive, taking several seconds to load the full form data, and being large enough to make Visual Studio's debugger crash (absent Natvis tricks I've used specifically to prevent this) when I try to inspect the form data. This is not strictly a unique failing of DovahKit: xEdit also takes several seconds to prepare Skyrim.esm's 15462 `NVMI` entries for display. Is there anything we can do about this?

Potentially, we could multi-thread loading the data somehow -- spawn subordinate file readers, and divide `NVMI` subrecords across multiple threads. I'm not 100% clear on how we could sensibly handle the fact that `NVMI`s are, conceptually, a map. We store them as a `std::vector` which means that in theory, we could have each thread load its own `std::vector` of NVMIs, and then coalesce them via slicing somehow. Alternatively, the code that doles NVMI subrecords out for reading could peek and resolve the navmesh form ID at the start of each NVMI, and use that to track and override duplicates.

**That said: we should benchmark every part of the load process before we try to optimize anything. For all we know, the overhead may just come from all the times we expand the std::vector, and not from the actual reading.**

Some notes:

* The reason we store NVMI using a `std::vector` is because `form_reference_t` can't be used as a map/set key. Even if we added a `std::hash` specialization for it, the problem is that we can't control precisely how the things are created or destroyed -- that logic is handled within the container -- and as such, it becomes incredibly difficult to maintain use info.

* Since we don't offer navmesh editing, files created in DovahKit cannot end up making any change that updates `NAVI`, and so in that instance, we don't need to load `NAVI` at all, much less for the purpose of re-saving it. However, if users use DovahKit to modify files originally made in the CK which contain navmesh edits, then those files will contain `NAVI` overrides, which we'll have to load and resave, and so this perf issue would add several seconds to the file-save time.

#### Updating/regeneration
We can load and re-save the `NavMeshInfoMap` (the `NAVI` singleton form), but we don't actually know how to update it. The CK updates it just before saving, at the start of `NavMeshInfoMap::Save`; the code involved has INI settings and log strings that make it easy to identify.


### Dialogue/alias edge-case

So it turns out, I thought ahead when implementing the editing of conditions: the QUST UI already communicates with `DovahKitCore` regarding changes to its quest stages or aliases, and the condition-editing UI and supporting backend structures are both designed to prefer working-copy data by default. This leaves safeguards for editing INFOs, though, because this means it's possible to:

1. Begin editing a QUST.
2. Add a new alias Foo to that QUST.
3. Edit an INFO which is parented to the quest, and add conditions which refer to Foo.
4. Commit changes to the INFO.
5. Cancel changes to the QUST, such that Foo disappears and the INFO now has a dangling reference to an invalid alias ID.

This generalizes to any forms that have an owning quest (since "Alias #123 on whatever my owning quest is" can be the run-on target for a condition), as well as to conditions in *any* form that take quest stages or quest alias IDs as parameters. A half-remedy -- something to cover the case of dangling references to not-yet-committed quest edits -- would be to have all form-editing dialogs [for forms that have conditions] inform DovahKitCore on save, as to what quest stages and aliases their conditions refer to; and then when canceling changes to a quest, DovahKitCore can silently sever references to any to-be-cancelled stages and aliases. However, there are all sorts of ways to have dangling references to individual parts of forms, so it may perhaps be better to instead make long-term (i.e. post-launch) plans for a system akin to use info that handles references to individual pieces of data within a given form, be those quest aliases, quest stages, or perhaps other things like package data.

I don't consider this a high-prio option because I'd be surprised if the Creation Kit handled this edge-case at all, but at some point I should probably at least check if they do (because if they do, then it's higher-prio for us to as well).

### QTabBar

QTabBar in Qt 5 has some nasty bugs that are triggered by hiding tabs, and it's impossible to fix all of these without building complete replacements for both QTabBar and QTabWidget. Some of these bugs are critical (i.e. extremely visually disruptive to the user experience), so we should look into building those replacements that at some point. I have WIP replacement files in DKTabBarEx.h and DKTabWidgetEx.h but I've excluded them from the project until such time as they're complete. For now, we just disable tabs instead of hiding them (which is arguably the better UX anyway, but I hate not having the option to hide them).

**BUG:** If tabs are hidden, then scrolling the tabbar with the mouse wheel can cause it to jump to a massively negative position, the result of a mistake at [this line](https://codebrowser.dev/qt5/qtbase/src/widgets/widgets/qtabbar.cpp.html#723).

Basically, in `QTabBarPrivate::makeVisible`, they want to check if a given tab (e.g. the tab you've focused) is partially or completely scrolled out of view, and if so, scroll the minimum distance needed to bring the tab fully into view. The problem, however, is that they compute `lastTabEnd` as the last tab's right edge *even if that tab is hidden and therefore has an all-zeroes rect*, and then they compute `scrolledTabBarEnd` as the lowest of `lastTabEnd - 1` or `scrollRect.right() + scrollOffset`. They should've used the `lastVisible` index, which they already track and cache!

The effect of this is that `scrolledTabBarEnd` becomes 0, and when `tabEnd` (the righthand edge of the tab we wish to make visible) is compared to it, we get `tabEnd > scrolledTabBarEnd` a.k.a. `tabEnd > -1`. We then set `scrollOffset = tabEnd - scrollRect.right()`, which ends up computing to a negative value (generally) and perfectly right-aligning the tab we wish to view. 

In Qt 6, they *may* have fixed it by clamping the scroll offset to never be negative. Whether that stems from an actual understanding of the issue or just trial-and-error tweaking, I don't know. Qt 5 is LTS so no hope of a backport.

