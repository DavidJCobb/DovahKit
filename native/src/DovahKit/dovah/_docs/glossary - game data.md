
# Glossary (game data)

## A

### action
A [form](#form) type representing something that an [actor](#actor) can be made to do. When an actor performs an action, it will also play the appropriate [idle animation](#idle) if there is one.

### actor
A type of [reference](#reference) used for characters and creatures in-game.

### actor base
A [base form](#base%20form) used by [actors](#actor), defining the core attributes of a character, such as their name, race, stats, and appearance.

## B

### base form
A [form](#form) which defines a variety of physical object that can be placed in the game world. Given a base form, an actual placed object of that variety is a [reference](#reference). "A kind of treasure chest that can exist" is a base form; "a treasure chest of that kind" is a reference.

### base record
The [record](#record) that originally defined a given [form](#form), as opposed to [override records](#override%20record).

### behavior graph
A collection of animations for a given animation skeleton, as part of Havok's animation engine.

## C

### cell
A region of physical space that can exist in-game. Cells can contain [refs](#ref), and a cell and its refs can be loaded and unloaded on demand as the player travels around the game world.

Outdoor spaces, called [worldspaces](#worldspace), are divided into a grid of <dfn>exterior cells</dfn>, which are loaded and unloaded on demand based on the player's current location. An exterior cell is 4096x4096 units. Its containing worldspace is called its "parent worldspace."

Every individual indoor space is a single <dfn>interior cell</dfn> existing in isolation, like a pocket dimension. These environments are cells because they use most of the same internal mechanisms within the game engine. Interior cells are not considered to belong to any worldspace, even if they're only accessible via doors that are placed in a worldspace's exterior cells.

There exists one other kind of cell: a [persistent cell](#persistent%20cell).

### chunk
Bethesda's internal term for [subrecords](#subrecord).

### condition
<a name="condition"></a>
<a name="condition function"></a>
Some forms can specify a list of conditions, linked with "and" versus "or" logical operators. These conditions have simple parameters filled out in the Creation Kit, and can be used to run fast checks against game state. The <dfn>condition functions</dfn> that serve as the checks you can run will always return a double-precision floating-point number as their result; a condition as a whole specifies the function to run, the [ref](#ref) to run the function on, and a comparison operator and operand to test that result against.

(Condition functions are actually [legacy script](#legacy%20script) opcodes that remain available in the engine. Similarly, console commands are also [legacy script](#legacy%20script) opcodes.)

## D

### data file
A file consisting of [groups](#group) and [records](#record), which defines [forms](#form), the basic entity in the game engine. Data files have one of the following extensions:

* `.esm`: Elder Scrolls [Master](#master)
* `.esp`: Elder Scrolls Plug-in
* `.esl`: Elder Scrolls Light master

## E

### editor ID
Every [form](#form) has a unique name string called an <dfn>editor ID</dfn>. These are displayed in the Creation Kit, and some form types retain them in memory during gameplay so that they can be used for debugging.

(Before Oblivion, forms referred to each other by their editor IDs; [form IDs](#form%20ID) didn't exist back then. A vestigial remnant of this history is the prevalence of [subrecords](#subrecord) whose [signatures](#signature) describe them as "names," e.g. `REFR/NAME` to identify a [ref](#ref)'s [base form](#base%20form) and `RACE/DNAM` to identify the "decapitate armor" for a race.)

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

Extra data is an implementation detail. The Creation Kit doesn't specifically mention extra data; it simply offers appropriate fields (e.g. Radius, Scale). Most extra data is serialized using [subrecords](#subrecord) whose [signatures](#signature) begin with an "X."

## F

### form
Broadly, a piece of game data that has a unique numeric ID (the [form ID](#form%20ID)). Forms can be "pure" data, such as a Combat Style or a Footstep Set; they can be "[base forms](#base%20form);" or they can be "[references](#reference)."

### form ID
The unique numeric ID of a [form](#form). Form IDs are four-byte unsigned integers, with the most-significant byte identifying the [data file](#data%20file) that originally defined the form (i.e. the index of the file containing the form's [base record](#base%20record)).

## G

### game setting
A named configuration option whose value is defined via [records](#record) in [data files](#data%20file). Game settings can be booleans, integers, or strings; their names use Hungarian notation, and the type prefix in a setting's name determines the setting's value type.

Although records are used to define them, game settings are not [forms](#form). The game engine doesn't create a form in memory when it sees a game setting.

### grid coordinates
An outdoor space, or [worldspace](#worldspace), is divided into a grid of [cells](#cell). Each such cell is aware of its <dfn>grid coordinates</dfn>: the physical coordinates of its northwest corner, divided by 4096 world units (the length of a cell along either horizontal axis).

These coordinates are defined in the `CELL/XCLC` [subrecord](#subrecord). Despite the [signature](#signature), this subrecord is not [extra data](#extra%20data).

### group
A group of [records](#record) within a [data file](#data%20file). Groups have a small header and a size, and are used to partition data files into sections. Theoretically, these sections could be read using multiple threads.

## I

### idle<br/>idle animation
<a name="idle"></a>
<a name="idle animation"></a>
A [form](#form) defining an animation that may be played when [actors](#actor) that have a given [behavior graph](#behavior%20graph) perform a given [action](#action).

## L

### legacy script
Scripts made for the script engine that preceded [Papyrus](#Papyrus).

Legacy scripts were stored within form data as [subrecords](#subrecord) (for the script source code, the compiled code, the property values, et cetera). These scripts execute synchronously on the game's main thread, with the invocation of a script function running to completion before the game engine's other operations can continue.

## O

### ObjectReference
Strictly speaking, this is the Papyrus typename for the [script-objects](#script-object) applied to every [ref](#ref). However, it's not necessarily *wrong* to use this term to refer to the refs themselves.

## M

### master
In the context of [data files](#data%20file), this can have two meanings:

* A <dfn>master file</dfn> is a data file that has a particular flag set in its file header, or which has the `.esm` or `.esl` file extension. These game data files are treated as "primary:" they're forced to the top of the load order, and they lack some accommodations that the game engine makes for user-authored mods (e.g. regarding [persistence](#persistent)).

* A data file's <dfn>masters</dfn> are the files that it depends on, and are listed in its file header.

## N

### NIF
The NetImmerse Format is the file format for Skyrim's 3D models. Although "NIF" is an acronym for the file format itself, it is typically used in noun form ("a NIF") to refer to files in that format.

NetImmerse was a game engine middleware that later became Gamebryo. The important parts of Bethesda's engine &mdash; the parts that enable their games' massive open worlds &mdash; are custom, but Gamebryo provided the original backbone for their 3D renderer, and Gamebryo utilities (e.g. `NiStream` for reading files; `NiPoint3` as a 3D vector type) are used throughout Skyrim's codebase.

## O

### override record
A [record](#record) in a [data file](#data%20file) which contains data for a [form](#form) originally defined in one fo the file's [masters](#master). When the same form has records in multiple loaded files, the last-loaded record is the one that "wins," and so overrides typically do as their name suggests: they wholly replace any previously-loaded data for the form.

(Some forms, like Locations and the Default Object Manager, coalesce data across all records.)

## P

### Papyrus
The new script engine introduced with Skyrim, replacing the previous engine seen in Oblivion, Fallout 3, and Fallout: New Vegas.

Papyrus scripts are stored in external files and referenced by name within [form](#form) data. These scripts execute asynchronously and in a multi-threaded environment, though certain script APIs can only run on the main thread and will cause a script to wait on thread synchronization. The game devotes a certain amount of time per frame to running Papyrus scripts and will pause scripted call stacks when that time is used up. This is in contrast to [legacy scripts](#legacy%20script), which would run synchronously and to completion.

### persistent
In the context of [forms](#form) that are loaded and unloaded on demand, such as [cells](#cell) and [refs](#ref), a <dfn>persistent</dfn> form is one that is kept in memory and never unloaded. Persistence can be controlled from game data files, and refs can also be <dfn>promoted</dfn> to persistent during gameplay, typically as a result of a Papyrus variable referring to them.

* All refs defined in a non-[master](#master) [data file](#data%20file) are forced to be persistent at run-time.

The term [temporary](#temporary) is the antonym.

### persistent cell
A [cell](#cell) that exists as a special case within the [data file](#data%20file) format, acting as a parent [record](#record) for all [persistent](#persistent) [refs](#ref) within a given worldspace.

Within the game engine, cells and their contents can be loaded and unloaded individually and on-demand. Cells exist "semantically" within gameplay as slices of physical space, and "mechanically" within the file format as containers for forms that can be loaded in bulk. Persistent cells fulfill the latter function only: they exist so that the game can bulk-load all persistent refs, instead of having to scan through every single cell in search of them. You can think of a worldspace's persistent cell as overlapping all of the worldspace's ordinary cells.

### promotion
A temporary [ref](#ref) can be "promoted" to [persistent](#persistent) during gameplay.

## R

### record
The community term for a [form](#form)'s serialized data within a game data file. A record has a header followed by the record body, which is divided into [subrecords](#subrecord). The record body may potentially be compressed using zlib. In rare cases, non-form objects such as [game settings](#game%20setting) are also serialized as records and subrecords.

The record header begins with a four-CC (commonly called a "signature") identifying the form type. The header contains a small amount of additional data, including the [record ID](#record%20ID), a multi-purpose flags field, and the record's serialized size.

Some records are capable of having <dfn>child records</dfn>. These children are stored inside of a [group](#group) placed directly after the parent record.

### record ID
Every [record](#record) contains a unique numeric ID analogous to a [form ID](#form%20ID).

Record IDs are local to the record's containing [data file](#data%20file), with an ID's most significant byte being the index (within the containing file's list of [masters](#master)) of the file which originally defined the [form](#form). By contrast, form IDs are the unique numeric IDs that [forms](#form) have after they've been loaded, with the most significant bytes being indices within the entire load order. You could think of record IDs as being "file-local" and form IDs as being "global."

### ref<br/>reference
<a name="ref"></a>
<a name="reference"></a>
A <dfn>reference</dfn> or <dfn>ref</dfn> is an object that physically exists in the game world. These are a kind of [form](#form), and Bethesda conceptualizes them as forms which "refer" to a [base form](#base%20form) that defines their shared properties. (The base form is often referred to as the reference's "base.")

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

### signature
Synonym for a four-CC, as seen in [records](#record) and [subrecords](#subrecord).

### space
In certain contexts, this term can refer to a [worldspace](#worldspace) or [cell](#cell), as in the following sentences:

* The `IsPlayerMovingIntoNewSpace` [condition](#condition) will return 1 when the player is in a loading screen.
* Load doors are typically used to teleport actors from one space into another, though you *can* link two load doors that exist in the same space.

DovahKit prefers the term "world or cell" over this usage of "space."

### subrecord
The community term for a fragment of a [record](#record), consisting of a four-CC (commonly called a "signature"), a length, and data. A subrecord may represent a single field within a [form](#form) (or nested struct), or an entire data structure.

Some subrecord signatures are unique across forms; for example, `VMAD` always refers to a form's attached [Papyrus](#Papyrus) script data ("VM attached data"). Other subrecord signatures are reused across form types with different meanings; for example, `ACTI/SNAM` and `RACE/SNAM` have very different meanings.

*Typically*, subrecords are order-independent. However, *some* subrecords do have specific ordering requirements:

* One subrecord may be dependent on data or conditions established by a previous subrecord.
* A list of structs within a form may be serialized using multiple subrecords per struct, with a particular subrecord marking the beginning of the next struct.
* One subrecord may expect to be followed by another subrecord, and the game engine may just blindly open the latter subrecord without even checking its four-CC.

Bethesda's own term for these is "chunk," with a signature being called a chunk's "ID."

## T

### temporary
The antonym of [persistent](#persistent).

### `TESObjectREFR`
See [ref](#ref).

### text replacement
Certain strings shown in the game's UI can contain substitution tokens taking the general form `<Tag.Subtag=Parameter>`. The game will detect and replace these tokens at run-time.

* DovahKit offers text-replacement utilities that can be used to compute and preview how text replacement would occur in-game; see `dovah/utils/text_replacers/`.

## W

### winning record
<a name="winning override"></a>
The last-loaded [record](#record) for a given [form](#form) (i.e. the record from the last-loaded [data file](#data%20file) which contains a record for that form), be that record an [override](#override%20record) or, if no overrides exist, the [base record](#base%20record). When the winning record is an override, it is also called the "winning override."

### world<br/>worldspace
<a name="world"></a>
<a name="worldspace"></a>
A <dfn>worldspace</dfn> is a [form](#form) which represents an outdoor environment. That environment is divided into a grid of [cells](#cell) that can each be loaded and unloaded individually.

One worldpsace can be made a "child" of another worldspace. The child worldspace will inherit properties from the parent worldspace, including the latter's LOD. Parent and child worldspaces are otherwise separate from each other: if the player is present in one, the other's cells and [refs](#ref) will not be loaded. For example, `RiftenWorld` defines the environment inside of Riften's city walls, and is a child of `Tamriel`, the outdoor space beyond those walls.

In limited situations, the term "world" may be used as a synonym of "worldspace," such as when referring to a cell or ref's "parent world" or when discussing some data field that may refer to a "world or cell." This is not the same as phrases like "the game world," which refer to the totality of all in-game environments (i.e. all worldspaces, and interior cells that by definition don't belong to any worldspace).