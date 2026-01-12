
# Glossary (DovahKit architecture)

## B

### backend
The core systems within DovahKit which deal with loading and parsing game data files (i.e. ESPs and friends), archive files (i.e. BSAs), localized string files, and anything else integral to proper handling of game data. Anything in the `dovah` namespace is part of the backend.

## D

### datastore
A helper class provided for use by [frontends](#frontend). Some datastores are provided by the [backend](#backend); others exist wholly within the frontend.

DovahKit's frontend is built using Qt, and Qt uses a model/view system for treeviews, tableviews, and similar widgets. Datastores are appropriate when:

* A common set of data needs to be shared across multiple models, possibly with different sorting, filtering, or scoping.
* The data in question has to be built and managed using complex logic that should be handled by the backend, rather than left to each individual frontend to implement.

#### Examples
* `dovah::datastores::idles` analyzes all loaded idle animations and builds a complete tree of behavior graphs, actions, and idles. It also offers callbacks for use by a `QAbstractItemModel`.
* The `DKFormPicker` UI widget allows users to pick forms of a given type (or set of types) from a drop-down. To avoid every single formpicker having to convert editor IDs from `std::string` to `QString` en masse, all formpickers share a common datastore that exists entirely within the frontend.

### Dovahscript
The scripting engine for DovahKit, built around Lua.

Dovahscript runs Lua code in a worker thread, so that the main thread (powering the application UI) remains unblocked and the user can always force-kill a script. This wouldn't guard against internal errors (i.e. an accidental infinite loop within the native code that Dovahscript APIs invoke) but will guard well enough against user errors (i.e. an accidental infinite loop within a Lua script). It also has the benefit of allowing some scripted UI modifications to be executed asynchronously with the script.

## F

### form stub
DovahKit does not load forms in full; it indexes forms across the load order, tracking their source files and offsets, and their key properties such as form ID and editor ID. Form stubs are the entries in this index. Given a stub, you can load the full data for a form on demand.

### form stub addenda
An add-on struct created and tracked by a [form stub](#form%20stub), for certain properties that are common on forms of a given type and need to be tracked even when the form is not fully loaded. Form stub addenda is used so that most form stubs can avoid having to make room for data they don't use.

### frontend
Any code existing outside of the [backend](#backend).

## R

### record skimmer
A helper struct that can be used to read a form's winning record and extract just specific information, skipping past everything else. They exist for cases where a [frontend](#frontend) might need to use some information for every form of a given type, but that information isn't worth packing into [form stub addenda](#form%20stub%20addenda).

#### Examples
* `dovah::loaded_forms::Package::record_skimmers::legacy_type` reads the legacy package type from a `PACK` record, taking into account all subrecords (including deprecated ones) that would mutate a package's type during the load process.
* `dovah::loaded_forms::Topic::record_skimmers::subtype` reads a topic's subtype, as indicated by `DATA` or `SNAM`.

## U

### use<br/>use info<br/>used form<br/>user form
<a name="use"></a>
<a name="use info"></a>
<a name="used form"></a>
<a name="user form"></a>
When one form refers to another, this is a <dfn>use</dfn>. The referring form is the <dfn>user form</dfn>, and the form to which it refers is the <dfn>used form</dfn>. Every [form stub](#form%20stub) tracks its <dfn>use info</dfn> bidirectionally, so given any stub, you can find [the stubs of] its user forms or used forms.

When two forms have a parent/child relationship within the data file (i.e. when one form exists inside of a child group of the other form), the child is considered the user of its parent.
