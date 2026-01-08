
We have model templates that automate a lot of the boilerplate involved in subclassing `QAbstractItemModel`. However, these templates aren't super well-organized, and they sometimes still involve a lot of setup that I'd rather avoid. Here's a potential idea:

* A template that takes, as its parameters, a node type and a list of column definitions. Each column definition consists of a column name, a pointer-to-member for the node type, and any other info we need.
  * We could also have additional column metadata e.g. text alignment overrides or "this string is, specifically, a game file path with these constraints."
* The template automatically sets up columns based on that list, auto-caching QStrings for form-stub columns and auto-handling form deletion and changing.
* You can mark columns as "mandatory non-null." If, for a given row, one of these columns is a form stub and that stub is deleted, then the whole row is deleted.
* Model instances store "coalesced node" instances, which contain a node-type instance and the cached data (with `[[no_unique_address]] [[msvc::no_unique_address]]` in case it's empty) for that node.

We could additionally make a Qt Designer widget consisting of a QTableView and Add/Move/Edit/Remove buttons. You can't template widgets and still use them in Qt Designer, but we could have a template function callable on this widget which, given a model type following the template described above (and optionally a pointer to an already extant model), automatically wires together all the editing functionality. (For actual editing of list items, we'd have to also supply a dialog type, which accepts the node type as input.) 

This would basically remove a lot of the copying, pasting, and boilerplate from things like Climate's weather list and possibly also Debris's model list.