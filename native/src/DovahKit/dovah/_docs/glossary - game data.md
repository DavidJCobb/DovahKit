
# Glossary (game data)

## A

### actor
A type of [reference](#reference) used for characters and creatures in-game.

### actor base
A [base form](#base%20form) used by [actors](#actor), defining the core attributes of a character, such as their name, race, stats, and appearance.

## B

### base form
A [form](#form) which defines a variety of physical object that can be placed in the game world. Given a base form, an actual placed object of that variety is a [reference](#reference). "A kind of treasure chest that can exist" is a base form; "a treasure chest of that kind" is a reference.

## C

### cell
A region of physical space that can exist in-game. Cells can contain [refs](#ref), and a cell and its refs can be loaded and unloaded on demand as the player travels around the game world.

Outdoor spaces, called [worldspaces](#worldspace), are divided into a grid of <dfn>exterior cells</dfn>, which are loaded and unloaded on demand based on the player's current location. An exterior cell is 4096x4096 units. Its containing worldspace is called its "parent worldspace."

Every individual indoor space is a single <dfn>interior cell</dfn> existing in isolation, like a pocket dimension. These environments are cells because they use most of the same internal mechanisms within the game engine. Interior cells are not considered to belong to any worldspace, even if they're only accessible via doors that are placed in a worldspace's exterior cells.

There exists one other kind of cell: a [persistent cell](#persistent%20cell).

### condition
<a name="condition"></a>
<a name="condition function"></a>
Some forms can specify a list of conditions, linked with "and" versus "or" logical operators. These conditions have simple parameters filled out in the Creation Kit, and can be used to run fast checks against game state. The <dfn>condition functions</dfn> that serve as the checks you can run will always return a double-precision floating-point number as their result; a condition as a whole specifies the function to run, the [ref](#ref) to run the function on, and a comparison operator and operand to test that result against.

(Condition functions are actually [legacy script](#legacy%20script) opcodes that remain available in the engine. Similarly, console commands are also [legacy script](#legacy%20script) opcodes.)

## E

### exterior cell
A [cell](#cell) that has a parent [worldspace](#worldspace).

### extra data
Some [form](#form) types (specifically, cells and [references](#reference)) can contain lists of <dfn>extra data</dfn> objects, which annotate these forms with additional information. Extra data objects are created and stored separately from the forms they belong to. They're generally used when a form type can contain an extremely wide variety of highly specialized information.

For example, the following properties on references are stored as extra data:

* An actor's dismemberment state (decapitation; werewolf/vampire feeding)
* An item's owning faction or NPC
* A soul gem's current soul size
* The changes made to a container's inventory
* The poison currently applied to a dropped weapon
* The scaling factor applied to a reference's size

It would be wasteful if every reference in the game world &mdash; every fencepost, mountain range, and tree &mdash; preemptively reserved memory to store all of that data &mdash; memory that they would never use, for properties they would never have. The extra data system makes it possible for forms to only reserve memory for this data when the data is actually relevant.

## F

### form
Broadly, a piece of game data that has a unique numeric ID. These can be "pure" data, such as a Combat Style or a Footstep Set; they can be "[base forms](#base%20form);" or they can be "[references](#reference)."

## L

### legacy script
Scripts made for the script engine that preceded [Papyrus](#Papyrus).

Legacy scripts were stored within form data as subrecords (for the script source code, the compiled code, the property values, et cetera). These scripts execute synchronously on the game's main thread, with the invocation of a script function running to completion before the game engine's other operations can continue.

## O

### ObjectReference
Strictly speaking, this is the Papyrus typename for the script-objects applied to every [ref](#ref). However, it's not necessarily *wrong* to use this term to refer to the refs themselves.

## M

### master
In the context of game data files, this can have two meanings:

* A <dfn>master file</dfn> is a data file that has a particular flag set in its file header, or which has the `.esm` or `.esl` file extension. These game data files are treated as "primary:" they're forced to the top of the load order, and they lack some accommodations that the game engine makes for user-authored mods (e.g. regarding [persistence](#persistent)).

* A data file's <dfn>masters</dfn> are the files that it depends on, and are listed in its file header.

## N

### NIF
The NetImmerse Format is the file format for Skyrim's 3D models. Although "NIF" is an acronym for the file format itself, it is typically used in noun form ("a NIF") to refer to files in that format.

NetImmerse was a game engine middleware that later became Gamebryo. The important parts of Bethesda's engine &mdash; the parts that enable their games' massive open worlds &mdash; are custom, but Gamebryo provided the original backbone for their 3D renderer, and Gamebryo utilities (e.g. `NiStream` for reading files; `NiPoint3` as a 3D vector type) are used throughout Skyrim's codebase.

## P

### Papyrus
The new script engine introduced with Skyrim, replacing the previous engine seen in Oblivion, Fallout 3, and Fallout: New Vegas.

Papyrus scripts are stored in external files and referenced by name within form data. These scripts execute asynchronously and in a multi-threaded environment, though certain script APIs can only run on the main thread and will cause a script to wait on thread synchronization. The game devotes a certain amount of time per frame to running Papyrus scripts and will pause scripted call stacks when that time is used up. This is in contrast to [legacy scripts](#legacy%20script), which would run synchronously and to completion.

### persistent
In the context of [forms](#form) that are loaded and unloaded on demand, such as [cells](#cell) and [refs](#ref), a <dfn>persistent</dfn> form is one that is kept in memory and never unloaded. Persistence can be controlled from game data files, and refs can also be <dfn>promoted</dfn> to persistent during gameplay, typically as a result of a Papyrus variable referring to them.

* All refs defined in a non-[master](#master) file are forced to be persistent at run-time.

The term [temporary](#temporary) is the antonym.

### persistent cell
A [cell](#cell) that exists as a special case within the file format, acting as a parent record for all [persistent](#persistent) [refs](#ref) within a given worldspace.

Within the game engine, cells and their contents can be loaded and unloaded individually and on-demand. Cells exist "semantically" within gameplay as slices of physical space, and "mechanically" within the file format as containers for forms that can be loaded in bulk. Persistent cells fulfill the latter function only: they exist so that the game can bulk-load all persistent refs, instead of having to scan through every single cell in search of them. You can think of a worldspace's persistent cell as overlapping all of the worldspace's ordinary cells.

### promotion
A temporary [ref](#ref) can be "promoted" to [persistent](#persistent) during gameplay.

## R

### ref<br/>reference
<a name="ref"></a>
<a name="reference"></a>
A <dfn>reference</dfn> or <dfn>ref</dfn> is an object that physically exists in the game world. These are a kind of [form](#form), and Bethesda conceptualizes them as forms which "refer" to a [base form](#base%20form) that defines their shared properties.

Using the term "reference" for this is, obviously, a bit confusing, since plenty of things can "refer" to each other without constituting this particular kind of "reference." DovahKit therefore prefers the term "ref," though documentation may also refer to "[ObjectReferences](#ObjectReference)" or to `TESObjectREFR`, the internal class name for refs.

* A reference's location is stored as its position and rotation within a "parent [cell](#cell)." Sometimes, the "parent [worldspace](#worldspace)" of a ref's parent cell may be referred to as the ref's own "parent worldspace," but this is a linguistic shortcut; refs do not store their containing worldspace.
* References are typically loaded and unloaded on demand, unless they are [persistent](#persistent).
* References can store [extra data](#extra%20data) for specialized state.
* All characters and creatures are [actors](#actor), a special kind of reference that reserves additional space for character-specific state such as stats.

## S

### script<br/>`ScriptObject`<br/>script-object
<a name="script"></a>
<a name="scriptobject"></a>
<a name="script-object"></a>
In [Papyrus](#Papyrus), a <dfn>script file</dfn> defines one or more classes. A <dfn>script-object</dfn> is an instance of that class.

In common usage, the term "script" can refer to a script file, a class, or a script-object; the term `ScriptObject` was only introduced beginning with Fallout 4. It's critical to distinguish between classes and script-objects when working on an editor for game data, so within DovahKit, "script" is the preferred term for Papyrus classes, while "script-object" is preferred when referring to an instance of an attached script.

A script can be "attached" or "bound" to a form. If this form is a [base form](#base%20form), then every time a [ref](#ref) with that base is created, a script-object will be created and attached to the ref. If the form is, itself, ref, then the binding defines a single script-object on that single ref.

### space
In certain contexts, this term can refer to a [worldspace](#worldspace) or [cell](#cell), as in the following sentences:

* The `IsPlayerMovingIntoNewSpace` [condition](#condition) will return 1 when the player is in a loading screen.
* Load doors are typically used to teleport actors from one space into another, though you *can* link two load doors that exist in the same space.

DovahKit prefers the term "world or cell" over this usage of "space."

## T

### temporary
The antonym of [persistent](#persistent).

### `TESObjectREFR`
See [ref](#ref).

### Text replacement
Certain strings shown in the game's UI can contain substitution tokens taking the general form `<Tag.Subtag=Parameter>`. The game will detect and replace these tokens at run-time.

* DovahKit offers text-replacement utilities that can be used to compute and preview how text replacement would occur in-game; see `dovah/utils/text_replacers/`.

## W

### world<br/>worldspace
<a name="world"></a>
<a name="worldspace"></a>
A <dfn>worldspace</dfn> is an outdoor environment, which is divided into a grid of [cells](#cell) that can each be loaded and unloaded individually.

One worldpsace can be made a "child" of another worldspace. The child worldspace will inherit properties from the parent worldspace, including the latter's LOD. Parent and child worldspaces are otherwise separate from each other: if the player is present in one, the other's cells and [refs](#ref) will not be loaded. For example, `RiftenWorld` defines the environment inside of Riften's city walls, and is a child of `Tamriel`, the outdoor space beyond those walls.

In limited situations, the term "world" may be used as a synonym of "worldspace," such as when referring to a cell or ref's "parent world" or when discussing some data field that may refer to a "world or cell." This is not the same as phrases like "the game world," which refer to the totality of all in-game environments (i.e. all worldspaces, and interior cells that by definition don't belong to any worldspace).