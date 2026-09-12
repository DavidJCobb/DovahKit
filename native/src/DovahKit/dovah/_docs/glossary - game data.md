
# Glossary (game data)

## A

### action
A [form](#form) type representing something that an [actor](#actor) can be made to do. When an actor performs an action, it will also play the appropriate [idle animation](#idle) if there is one.

### actor
A type of [reference](#reference) used for characters and creatures in-game.

### actor base
A [base form](#base%20form) used by [actors](#actor), defining the core attributes of a character, such as their name, race, stats, and appearance.

### attached (cell)
One of the states that a [cell](#cell) can have during gameplay.

* If a cell is "attached," then the player-character is actively inside of the cell or (if it's an exterior) inside of an adjacent cell. This is what most players would think of as "the area being loaded:" the game is willing and able to render the cell in 3D, and is simulating events (physics, [actor](#actor) AI, et cetera) inside of the cell at full detail.

* If a cell is merely "loaded," then it and all of its [refs](#ref) exist in memory. However, the player may not actually be inside or near the cell; the cell may be kept loaded in the background as an optimization (e.g. because it's a recently visited interior). If that's the case &mdash; if the cell isn't attached &mdash; then actors inside of the cell may have their AI simulated at a low level of detail, and nothing else inside of the cell (physics, etc.) will be simulated.

Exterior cells are attached and detached based on the player's position: the cell that the player is in, and a limited number of cells surrounding them (defined by the `uGrids to Load` INI setting), will be attached. If the player is an interior, then all exterior cells are detached, and only that sole interior is attached.

### attached (script)
See [script object](#script%20object).

## B

### base (of a ref)
See [base form](#base%20form).

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

During gameplay, cells can be [attached](#attached%20cell) and fully simulated, or they can be merely loaded in the background.

### chunk
Bethesda's internal term for [subrecords](#subrecord). They call the chunk's [signature](#signature) its "ID."

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

### default object
A slot that the game engine exposes, such that a [data file](#data%20file) can specify a [form](#form) to fill it; or any form which has been provided to fill such a slot.

There are a number of systems hardcoded into the game engine that need forms to operate on. Bethesda used to [hardcode](#hardcoded%20form) forms into the game engine and then [override](#override%20record) them in data files, but as the engine's functionality grew, this became impractical. Beginning in <i>Skyrim</i>, Bethesda created the Default Object Manager, a [singleton form](#singleton%20form) whose data maps hardcoded [FourCCs](#FourCC) to forms. During play, the game engine can look up which form is associated with some FourCC.

Examples of default objects include:

| FourCC | Name | Purpose |
| :- | :- | :- |
| `CMPX` | Complex Scene Object | The [base form](#base%20form) to be used for "complex scene markers," intended to be placed at specific points in the game world to signal performance-heavy areas to the game engine. The game keeps track of how many [refs](#ref) with this base form exist in [attached](#attached%20cell) [cells](#cell), and will alter hardcoded behaviors based on their presence or absence. |
| `FTNP` | Furniture Test NPC | The base form spawned by the `PlaceFurnitureTester` console command to serve as a test NPC. |
| `GOLD` | Gold | The item to use as currency when bartering. Gold is already hardcoded into the engine (as `[MISC:00F]Caps001`), but the form has been grandfathered into the default object system for consistency. |
| `LKPK` | Lockpick | The item to use as a lockpick. Lockpicks are already hardcoded into the engine (as `[MISC:00A]BobbyPin`), but the form has been grandfathered into the default object system for consistency. |
| `PLOC` | "Persist All" Location | A location form: all refs in this location are automatically made [persistent](#persistent). |
| `SAT1` | Keyword: Scale Actor To 1.0 | [Actors](#actor) will have their scale temporarily forced to 1.0 while using any furniture that has this keyword. |
| `SKLK` | SkeletonKey | The item to use as the Skeleton Key: a lockpick that never breaks, and is used before any other lockpicks. |
| `WWSP` | Werewolf Spell | The spell to apply to an actor when they change form into a werewolf. |

### delocalized
The adjective Bethesda uses to refer to a [localized string](#localized%20string) whose content is stored in a string file.

This choice of terminolgy is ambiguous (see [On the word "delocalized"](#on-the-word-delocalized)) and should not be used within DovahKit's internals or documentation. Prefer "localized" for this situation, and "non-localized" for the reverse.

## E

### editor ID
Every [form](#form) has a unique name string called an <dfn>editor ID</dfn>. These are displayed in the Creation Kit, and some [form types](#form%20type) retain them in memory during gameplay so that they can be used for debugging.

(Before Oblivion, forms referred to each other by their editor IDs; [form IDs](#form%20ID) didn't exist back then. A vestigial remnant of this history is the prevalence of [subrecords](#subrecord) whose [signatures](#signature) describe them as "names," e.g. `REFR/NAME` to identify a [ref](#ref)'s [base form](#base%20form) and `RACE/DNAM` to identify the "decapitate armor" for a race.)

It's worth noting that some game systems (particularly [behavior graphs](#behavior%20graph)) still refer to forms solely by their editor ID. Among other things, behavior graphs can issue animation events that refer to sounds by editor ID (i.e. <code>SoundPlay.<var>EditorID</var></code>) in order to trigger sound playback.

### exterior cell
A [cell](#cell) that has a parent [worldspace](#worldspace).

### extra data
Some [form](#form) types (specifically, cells and [references](#reference)) can contain lists of <dfn>extra data</dfn> objects, which annotate these forms with additional information. Extra data objects are created and stored separately from the forms they belong to. They're generally used when a [form type](#form%20type) can contain an extremely wide variety of highly specialized information.

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

Forms are generally defined in [data files](#data%20file), though a small number of forms are [hardcoded](#hardcoded%20form).

### form ID
The unique numeric ID of a [form](#form). Form IDs are four-byte unsigned integers, with the most-significant byte (called the "[load order prefix](#load%20order%20prefix)") identifying the [data file](#data%20file) that originally defined the form (i.e. the index of the file containing the form's [base record](#base%20record)).

Forms created at run-time, during gameplay, have the most-significant byte fixed at <code>0xFF</code>.

Forms defined in light files have the most-significant byte fixed at <code>0xFE</code>, and use the next three hexadecimal digits to indicate the data file that originally defined them.

All form IDs below `00000800` are reserved for [hardcoded forms](#hardcoded%20form). Bethesda games prior to <i>Skyrim Special Edition</i> version 1.6.1130 had a bug in the form loader: if, after stripping off the load order prefix, a form ID or record ID was less than `0x800`, then the [record](#record) would be treated as an [override](#override%20record) of a hardcoded form. Version 1.6.1130 fixes this bug so that hardcoded forms exclusively use load order prefix `00`; among other things, this means that a [light data file](#light%20data%20file) with at least one [master](#master) can now define twice as many new forms.

### form type
This can refer to a [form](#form)'s internal class, to the enumerated type (collection of named integer constants) corresponding to all possible classes, or to a value from that enumeration.

Internally, all forms are instances of the `TESForm` class, and all form type constants correspond to derived classes. Each such derived class has a corresponding form type constant. Additionally, a small number of form type constants exist for entities that are not forms, but are serialized using [records](#record), such as the file header and [game settings](#game%20setting).

Thus the term "form type" is context-dependent, used to refer to:

* One of those classes.
* One of those constants.
* The enumerated type of which those constants are members.

### FourCC
A four-character code: a sequence of four ASCII bytes meant to serve as a human-readable identifier within data that is otherwise non-textual. In [data files](#data%20file), FourCCs are used to identify [groups](#group), [records](#record), and [subrecords](#subrecord). In this context, FourCCs may also be referred to as "signatures."

See also: ["FourCC" on Wikipedia](https://en.wikipedia.org/wiki/FourCC).

## G

### game setting
A named configuration option whose value is defined via [records](#record) in [data files](#data%20file). Game settings can be booleans, integers, or strings; their names use Hungarian notation, and the type prefix in a setting's name determines the setting's value type.

Although records are used to define them, game settings are not [forms](#form). The game engine doesn't create a form in memory when it sees a game setting.

### grid coordinates
An outdoor space, or [worldspace](#worldspace), is divided into a grid of [cells](#cell). Each such cell is aware of its <dfn>grid coordinates</dfn>: the physical coordinates of its northwest corner, divided by 4096 world units (the length of a cell along either horizontal axis).

These coordinates are defined in the `CELL/XCLC` [subrecord](#subrecord). (Despite the [signature](#signature), this subrecord is not [extra data](#extra%20data).) Exterior cell [records](#record) are organized into [groups](#group) based on their grid coordinates so that the game can find and load the records quickly. (Interior cells have no grid coordinates, and are grouped based on their [form IDs](#form%20ID).)

### group
A group of [records](#record) within a [data file](#data%20file). Groups have a small header (beginning with the [FourCC](#FourCC) `GRUP`) and a size, and are used to partition data files into sections. Theoretically, these sections could be read or indexed using multiple threads.

Typically, all [forms](#form) of a given [form type](#form%20type) will have their records placed in a top-level group dedicated to that form type. The order of these groups is significant, as the code to load some form types will assume (based on that order) that all forms of certain other types have already been loaded. (For example, [action](#action) forms must load before [idle](#idle) forms, as the loader for idles checks whether an idle's parent is an action by doing a form ID lookup earlier than usual.)

Groups can be nested. This is generally done when a [form](#form) needs to have "child forms" arranged such that loading of the parent form can trigger bulk loading of all child forms. In these scenarios, the child forms' records will be placed in a group located immediately after the parent form's record.

## H

### hardcoded form
A [form](#form) that is hardcoded into the game engine: the game (and/or Creation Kit) will create an instance of this form in memory at startup, with a fixed [form ID](#form%20ID).

Forms are generally hardcoded into the game engine when they're needed for built-in behaviors. For example, the game must have at least one [worldspace](#worldspace) to load the player into, so form ID `03C` is the hardcoded `DefaultWorld` (renamed to `Tamriel` by `Skyrim.esm`). Beginning with <i>Skyrim</i>, Bethesda introduced [default objects](#default%20object) as an alternative to hardcoding forms into the engine.

## I

### idle<br/>idle animation
<a name="idle"></a>
<a name="idle animation"></a>
A [form](#form) defining an animation that may be played when [actors](#actor) that have a given [behavior graph](#behavior%20graph) perform a given [action](#action).

### info
See [topic info](#topic%20info).

## L

### large ref
A [ref](#ref) in an exterior [cell](#cell) that has been indexed in a [worldspace](#worldspace)'s "large ref data" (`BGSLargeRefData`). This system is used to force refs to load even if they're not in the loaded area, so that especially large objects (e.g. the architecture of the College of Winterhold) render at full detail from a further distance than normal.

### legacy script
Scripts made for the script engine that preceded [Papyrus](#Papyrus).

Legacy scripts were stored within form data as [subrecords](#subrecord) (for the script source code, the compiled code, the property values, et cetera). These scripts execute synchronously on the game's main thread, with the invocation of a script function running to completion before the game engine's other operations can continue.

## O

### ObjectReference
Strictly speaking, this is the Papyrus typename for the [script-objects](#script-object) applied to every [ref](#ref). However, it's not necessarily *wrong* to use this term to refer to the refs themselves.

## L

### light data file<br/>light plug-in<br/>light master
<a name="light"></a>
<a name="light data file"></a>
<a name="light plugin"></a>
<a name="light master"></a>
A [data file](#data%20file) which has the "light" flag set in its file header, or which has the `.esl` file extension, *and* which is being loaded by a game that supports light data files.

Light data files use an alternate [form ID](#form%20ID) format which permits the game to load more of these files at a time, while constraining the number of [forms](#form) each such file can define.

### load order
Collectively, all of the [data files](#data%20file) that have been loaded for a given gameplay or editing session.

### load order prefix
The most-significant byte of a [form ID](#form%20ID) or [record ID](#record%20ID), used to identify the [data file](#data%20file) that originally defined a given [form](#form).

In games that support [light data files](#light%20data%20file), forms defined in such files use the sentinel value `0xFE` as their load order prefix in their form IDs. Whether they also do so in their record IDs varies from game to game, with all pre-<i>Starfield</i> games declining to do so.

For forms created during gameplay, the load order prefix is the sentinel value `0xFF`.

### localized string
A string within a [data file](#data%20file) whose content can be stored either inline or in a [strings file](#strings-file). When localized strings are inlined, the file header will lack the "localized" flag, and the localized string's [subrecord](#subrecord) will contain the string's content. When strings are stored in a strings file, the file header will have the "localized" flag, and the localized string's subrecord will contain the four-byte index of a string in the string file.

The name of a strings file is the name of the data file, suffixed with an underscore and the name of the language. There are three sub-types of localized string, each using different strings file extensions:  `.STRINGS`, `.DLSTRINGS`, and `.ILSTRINGS`. (The latter two are used for various "description" strings and for [topic info](#topic-info) text.) Internally, these sub-types use the classes `BGSLocalizedString`, `BGSLocalizedStringDL`, and `BGSLocalizedStringIL`.

Strings files make it possible to localize a single game data file for use with multiple languages. The localized strings can be translated without needing to touch the data file itself.

Localized strings may be [tagified](#tagification) when working with version control.

#### On the word "delocalized"

Strangely, Bethesda seems to use the term "[delocalized](#delocalized)" to mean "localized." For example, if a file is flagged as "localized" but a localized string subrecord isn't exactly four bytes long, the Creation Kit will emit the warning message "LOCALIZATION: Delocalized TESFile, but [chunk](#chunk)size is not a BSUInt32." It's possible that Bethesda is here using "localize" to mean two different things at once: they may be using the verb form "localization" to refer to the process of translating content for different languages, but using the verb form "localized" to mean "stored locally," such that a string whose content is stored in a strings file (facilitating "localization" of its content) would be "delocalized" because its content is not "local" to the data file.

DovahKit intentionally diverges from Bethesda's terminology on this point: we consistently use "localize" (in the context of strings) to refer to the process of "localization." To put it more explicitly, we use the verb form "localized" to describe a situation wherein the relevant flag in the file header is set, and the localized string content is stored in string files and referenced by four-byte string IDs; thus the text content used by the data file can be subject to the process of "localization" for multiple languages. We may use "non-localized" to refer to the opposite situation, wherein localized string content is inlined into the data file and "localization" for multiple languages at a time is therefore not possible. We never use "delocalized."

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
A [record](#record) in a [data file](#data%20file) which contains data for a [form](#form) originally defined in one of the file's [masters](#master). When the same form has records in multiple loaded files, the last-loaded record is the one that "wins," and so overrides typically do as their name suggests: they wholly replace any previously-loaded data for the form.

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

Within the game engine, cells and their contents can be loaded and unloaded individually and on-demand. Cells exist "semantically" within gameplay as slices of physical space, and "mechanically" within the file format as containers for forms that can be loaded in bulk. Persistent cells fulfill both of those functions on behalf of an entire worldspace: persistent refs anywhere in a worldspace are listed in data files as children of the worldspace's persistent cell; and when a worldspace isn't currently loaded, any persistent refs located inside of that worldspace will have their parent cell set to the worldspace's persistent cell. (The alternative would be to risk these refs having a null "parent cell" pointer.)

You can think of a worldspace's persistent cell as overlapping all of the worldspace's ordinary cells.

### plug-in
See [data file](#data%20file).

### promotion
A temporary [ref](#ref) can be "promoted" to [persistent](#persistent) during gameplay.

## R

### record
The community term for a [form](#form)'s serialized data within a game data file. A record has a header followed by the record body, which is divided into [subrecords](#subrecord). The record body may potentially be compressed using zlib. In rare cases, non-form objects such as [game settings](#game%20setting) are also serialized as records and subrecords.

The record header begins with a [FourCC](#FourCC) (commonly called a "signature") identifying the [form type](#form%20type). The header contains a small amount of additional data, including the [record ID](#record%20ID), a multi-purpose flags field, and the record's serialized size.

Some records are capable of having <dfn>child records</dfn>. These children are stored inside of a [group](#group) placed directly after the parent record.

### record ID
Every [record](#record) contains a unique numeric ID analogous to a [form ID](#form%20ID).

Record IDs are local to the record's containing [data file](#data%20file), with an ID's most significant byte (called the "[load order prefix](#load%20order%20prefix)") being the index (within the containing file's list of [masters](#master)) of the file which originally defined the [form](#form). By contrast, form IDs are the unique numeric IDs that [forms](#form) have after they've been loaded, with the most significant bytes being indices within the entire load order. You could think of record IDs as being "file-local" and form IDs as being "global."

Prior to <i>Starfield</i>, record IDs consisted only of the load order prefix byte and a unique identifier for the form, even if the form was defined in a [light file](#light%20data%20file). This meant that files could not have more than 254 masters.

Beginning with <i>Starfield</i>'s launch, record IDs for those forms now follow the same conventions as form IDs: the most-significant byte is the sentinel value <code>0xFE</code>, and the next three hexadecimal digits indicate the index of a light file within the containing file's list of masters. This allows files to have significantly more masters (provided those masters are light files), but it also makes inter-file dependencies more brittle: if one of a dependent file's masters is independently modified and *becomes* or *ceases to be* light, then the record IDs on overrides will become mismatched, trashing the dependent file. Fortunately, this change has not been backported to any edition of <i>Skyrim</i>.

### ref<br/>reference
<a name="ref"></a>
<a name="reference"></a>
A <dfn>reference</dfn> or <dfn>ref</dfn> is an object that physically exists in the game world. These are a kind of [form](#form), and Bethesda conceptualizes them as forms which "refer" to a [base form](#base%20form) that defines their shared properties. (The base form is often referred to as the reference's "base.")

Using the term "reference" for this is, obviously, a bit confusing, since plenty of things can "refer" to each other without constituting this particular kind of "reference." DovahKit therefore prefers the term "ref," though documentation may also refer to "[ObjectReferences](#ObjectReference)" or to `TESObjectREFR`, the internal class name for refs.

* A reference's location is stored as its position and rotation within a "parent [cell](#cell)." Sometimes, the "parent [worldspace](#worldspace)" of a ref's parent cell may be referred to as the ref's own "parent worldspace," but this is a linguistic shortcut; refs do not store their containing worldspace.
* References are typically loaded and unloaded on demand, unless they are [persistent](#persistent).
* References can store [extra data](#extra%20data) for specialized state.
* All characters and creatures are [actors](#actor), a special kind of reference that reserves additional space for character-specific state such as stats.

### Rule of One
Typically, when a form is defined in multiple [data files](#data%20file), i.e. in multiple [records](#record), only the data from the [winning record](#winning%20record) is retained. This is known as the Rule of One.

This is the leading source of mod conflicts: if one mod tries to change the price of a Green Apple, and another mod tries to change the keywords on a Green Apple, then only one mod's set of changes will be applied, because each mod includes a full copy of the original Green Apple record with a handful of changes made.

In rare cases, game data won't follow the Rule of One. This is because the rule is enforced manually for all data in all form types. When the game sees an override record, it asks the to-be-loaded form to clear all of its already-loaded data (by invoking a virtual member function on the form), but whether and how that's actually done is left to the form's discretion. All a form has to do is just... *not* clear something, and then that data will be coalesced across all records. A few examples where this occurs include:

* The Default Object Manager (`DOBJ`) [singleton form](#singleton%20form) doesn't follow the Rule of One. Default object entries always append to a common map. The NavMeshInfoMap singleton form behaves similarly.

* Location (`LCTN`) forms maintain multiple lists of refs that exist within the given location. Location forms disobey the Rule of One for these lists specifically, and will manage the lists' contents manually via "define," "add," and "remove" subrecords.

## S

### script<br/>`ScriptObject`<br/>script-object
<a name="script"></a>
<a name="scriptobject"></a>
<a name="script-object"></a>
In [Papyrus](#Papyrus), a <dfn>script file</dfn> defines one or more classes. A <dfn>script-object</dfn> is an instance of that class.

In common usage, the term "script" can refer to a script file, a class, or a script-object; the term `ScriptObject` was only introduced beginning with Fallout 4. It's critical to distinguish between classes and script-objects when working on an editor for game data, so within DovahKit, "script" is the preferred term for Papyrus classes, while "script-object" is preferred when referring to an instance of an attached script.

A script can be "attached" or "bound" to a form. If this form is a [base form](#base%20form), then every time a [ref](#ref) with that base is created, a script-object will be created and attached to the ref. If the form is, itself, ref, then the binding defines a single script-object on that single ref.

### SharedInfo
A [topic info](#topic%20info) whose text content and voice files can be reused verbatim by multiple other topic infos. The typical use case is for common lines of dialogue &mdash; "yes," "no," "Skyrim belongs to the Nords," and so on &mdash; that can appear in significantly different conversations and contexts, with different conditions attached.

The path to an info's voice files is computed automatically based on

* the name of the file containing the info's [base record](#base%20record)
* the form ID and editor ID of the info's parent [topic](#topic)
* the form ID and editor ID of that topic's owning quest
* the form ID of the info
* the internal unique ID of each response (a single-byte integer hidden while editing)
* the voicetypes that the Creation Kit determines could be able to say the info, based on the info conditions and on the voices used by all defined [actors](#actor)

which means that it isn't possible to create a single voice file and then point two infos at it. The workaround is to use a SharedInfo to define the responses, and then have other infos point to that SharedInfo.

### signature
Synonym for a [FourCC](#FourCC).

### singleton form
A limited number of [form types](#form%20type) only allow one [form](#form) of their type to exist; those forms are <dfn>singleton forms</dfn>. If a [record](#record) exists with that form type, then no matter what [record ID](#record%20ID) it has, it's treated as an [override](#override%20record) of the singleton form.

Two singleton forms exist in <i>Skyrim</i>. Both of them are [hardcoded](#hardcoded%20form), and coalesce all of their data across all of their records, rather than only loading the contents of the [winning record](#winning%20record).

* The [Default Object](#default%20object) Manager is used to provide forms to hardcoded engine features, so that the forms themselves don't need to be hardcoded.

* The NavMeshInfoMap stores pre-computed [worldspace](#worldspace)-level waypoint graphs. NPC pathing within an [attached cell](#attached%20cell) uses navmeshes; pathing within detached cells and unloaded environments uses the waypoint graphs, wherein each navmesh is a graph node. This allows NPCs to follow <i>Skyrim</i>'s roads and trails in a plausible manner even when they're on the other side of the country from the player-character, with the player tracking their position via quest waypoints.

### space
In certain contexts, this term can refer to a [worldspace](#worldspace) or [cell](#cell), as in the following sentences:

* The `IsPlayerMovingIntoNewSpace` [condition](#condition) will return 1 when the player is in a loading screen.
* Load doors are typically used to teleport actors from one space into another, though you *can* link two load doors that exist in the same space.

DovahKit prefers the term "world or cell" over this usage of "space."

### strings file
A sidecar file that contains the content of [localized strings](#localized-string) in a [data file](#data-file). A strings file is stored in `Data/Strings/` and will have a name of the form `DataFileName_Language.STRINGS` e.g. `Skyrim_English.STRINGS`. There are three possible file extensions, one for each type of localized string.

### subrecord
The community term for a fragment of a [record](#record), consisting of a [FourCC](#FourCC) (commonly called a "signature"), a length, and data. A subrecord may represent a single field within a [form](#form) (or nested struct), or an entire data structure.

Some subrecord signatures are unique across forms; for example, `VMAD` always refers to a form's attached [Papyrus](#Papyrus) script data ("VM attachment data"). Other subrecord signatures are reused across [form types](#form%20type) with different meanings; for example, `ACTI/SNAM` and `RACE/SNAM` have very different meanings.

*Typically*, subrecords are order-independent. However, *some* subrecords do have specific ordering requirements:

* One subrecord may be dependent on data or conditions established by a previous subrecord.
* A list of structs within a form may be serialized using multiple subrecords per struct, with a particular subrecord marking the beginning of the next struct.
* One subrecord may expect to be followed by another subrecord, and the game engine may just blindly open the latter subrecord without even checking its four-CC.
* Subrecords are typically consumed by a `while` loop. When structs are nested in a form type, the loading code *may* enter a nested `while` loop for those structs, such that non-struct-related subrecords are ignored/unrecognized until the struct is finished loading. (This behavior is not consistently done for every form type, nor necessarily for every struct within those form types that behave this way.)

Bethesda's own term for subrecords is "[chunks](#chunk)," with a signature being called a chunk's "ID."

## T

### tagification
<a name="tagified"></a>
<a name="tagify"></a>
A Creation Kit process triggered by the `-TagifyMasterfile` command line switch. This appears to involve applying a prefix of the form `<ID=xxxxxxxx>` to all [localized strings](#localized%20string) in a file. Within the prefix, `xxxxxxxx` is the hexadecimal unique ID of a localized string within the relevant [strings file](#strings-file).

Localized strings that begin with `<ID=` are assumed by the Creation Kit to be tagified.

If the Creation Kit INI setting `[General]bReconstructIDTags` is enabled, then localized strings that get loaded from string files will be tagified on load. The Creation Kit is designed to hide the tags from its UI when editing form data.

### temporary
The antonym of [persistent](#persistent).

### `TESObjectREFR`
See [ref](#ref).

### text replacement
Certain strings shown in the game's UI can contain substitution tokens taking the general form `<Tag.Subtag=Parameter>`. The game will detect and replace these tokens at run-time.

* DovahKit offers text-replacement utilities that can be used to compute and preview how text replacement would occur in-game; see `dovah/utils/text_replacers/`.

### topic
One of the [form types](#form%20type) used to define in-game dialogue. In typical usage, a <dfn>topic</dfn> is an individual remark that the player can say to an NPC, and a [topic info](#topic%20info) (also called an info) is a response that an NPC can give. Some topics instead serve as hidden containers for dialogue that can play in response to various engine-level events, such as pained yelps when actors are hit by attacks; these topics are identified by their [subtype](#topic%20subtype).

(These [forms](#form) are so named because in Bethesda's older titles, it was common to write general worldbuilding dialogue and assign it to NPCs <i>en masse</i>: you could ask almost any NPC questions about the world around you and get an informative response. In other words, you could pick just about any "topic" to ask about, and be told "info" about it.)

Topics exist as parent forms to infos, with the latter stored in child [groups](#group). Topics are generally owned by a dialogue branch or quest form, and thse are commonly called the topics' "parents," but this parent/child relationship is defined via [subrecords](#subrecord) rather than by record grouping.

When the game decides what topics to present to the player, it does so by scanning through each topic's child infos sequentially, looking for an info whose [conditions](#condition) test as true. If none of a topic's infos test positive, then the topic will not be available to the player. Otherwise, the game will usually use the first such info it finds as the NPC's response to the topic. (It's also possible to flag a contiguous range of infos as "random," such that the game will test all infos in that range and pick randomly from those whose conditions test as true.)

### topic info
<a name="TopicInfo"></a>
One of the [form types](#form%20type) used to define in-game dialogue. In typical usage, a <dfn>topic info</dfn> or <dfn>info</dfn> is a sequence of dialogue lines that an [actor](#actor) can say, typically in response to [something the player has said](#topic%20info).

A typical info contains a list of [conditions](#condition) to determine whether the info can play at all, and a set of <dfn>responses</dfn> which are played sequentially. Each response has a text caption, a voice file, and settings to tweak the NPC's facial animations.

Infos can borrow response data from a [SharedInfo](#SharedInfo), for situations where voice lines should be reused.

### topic subtype
In general, <dfn>topic subtypes</dfn> are the potential triggers for activation of a [topic](#topic). Topics used in conventional dialogue have the "Custom" (`CUST`) subtype, but other subtypes correspond to hardcoded engine events involving [actors](#actor), with a few examples including:

| [FourCC](#FourCC) | Name | Description |
| :- | :- | :- |
| `DETH` | Death | Used to play death rattles and last words as an actor dies. |
| `ENBZ` | EnterBowZoomBreath | Used when the player-character holds their breath while aiming a bow, to zoom in. The "holding my breath" sound effect is an [info](#topic%20info). |
| `HELO` | Hello | Used for dialogue that NPCs say spontaneously when they come within a short range of other actors. |
| `HIT_` | Hit | Used for groans, yelps, and screams that an actor lets out when they're hit by an attack. |
| `IDAT` | SharedInfo | Used for topics that serve as generic containers for [SharedInfos](#SharedInfo). |
| `KNOO` | KnockOverObject | Used for dialogue that NPCs use to chastise the player for knocking over nearby MiscItems in the game world. |
| `SCEN` | Scene | Used for topics that serve as hidden containers for scenes' Dialogue Actions. |

Aside from the Custom and Scene subtypes, each quest may only have one topic with a given subtype.

## W

### winning record
<a name="winning override"></a>
The last-loaded [record](#record) for a given [form](#form) (i.e. the record from the last-loaded [data file](#data%20file) which contains a record for that form), be that record an [override](#override%20record) or, if no overrides exist, the [base record](#base%20record). When the winning record is an override, it is also called the "winning override."

### world<br/>worldspace
<a name="world"></a>
<a name="worldspace"></a>
A <dfn>worldspace</dfn> is a [form](#form) which represents an outdoor environment. That environment is divided into a grid of [cells](#cell) that can each be loaded and unloaded individually.

One worldspace can be made a "child" of another worldspace. The child worldspace will inherit properties from the parent worldspace, including the latter's LOD. Parent and child worldspaces are otherwise separate from each other: if the player is present in one, the other's cells and [refs](#ref) will not be loaded. For example, `RiftenWorld` defines the environment inside of Riften's city walls, and is a child of `Tamriel`, the outdoor space beyond those walls.

In limited situations, the term "world" may be used as a synonym of "worldspace," such as when referring to a cell or ref's "parent world" or when discussing some data field that may refer to a "world or cell." This is not the same as phrases like "the game world," which refer to the totality of all in-game environments (i.e. all worldspaces, and interior cells that by definition don't belong to any worldspace).