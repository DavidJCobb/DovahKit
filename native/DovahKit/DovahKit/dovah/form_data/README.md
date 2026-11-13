
# Form data

## Organization

Types are divided up as such:

* **Components:** Sub-objects that appear as members of multiple form types, and for which you may want type-erased access (i.e. "I have some piece of form data and I do not, at this moment, know its form type; but I want to access its 3D model data if its form type has any"). Examples include:

  * 3D model data (MODL and friends)
  * Attack data (ATKR+ATKD)
  * Condition lists (CTDA)
  * Destruction data (DEST and friends)
  * Extra-data
  * Inventory
  * Keyword lists (KSIZ+KWDA)
  * Object bounds (OBND)
  * Papyrus VM attachment data (VMAD)

* **Pieces:** Sub-objects which only appear on a single form type, *or* for which generic access isn't needed. In some cases, a type within this category may only be used on a single form type, but it's been separated off for ease of implementation and editing. Examples include:

  * ActorBase creature sounds
  * ActorBase FaceGen morphs
  * Navmesh `PathingCell`
  * Quest alias fill parameters
  * Typed package data
    * Custom package data

**Types:** Classes that represent loaded data for a given form type.

Other groupings include:

* **Fragments:** C++ files which are incomplete on their own; they are meant to be `#include`d at specific points within the files that use them. For example, one fragment file is meant to be included mid-struct-definition, after defining a few preprocessor macros to serve as parameters; this fragment file sets up `visit_fields` member functions on the struct, as a "poor man's reflection" system.

* **Function tables:** Function tables indexed by form type, e.g. "I want to load the full form data for a form of *this* type."

* **Helpers:** Various definitions meant to make implementation of form-data easier.

* **Operations:** Function templates that can be invoked on form-data objects or their sub-objects, to perform some operation (e.g. severing all outbound uses). Commonly, each operation will have an associated function table: if you have a piece of form data on hand and you know its type at compile time, use an operation directly; otherwise, use the function tables, which just dispatch to the operations.