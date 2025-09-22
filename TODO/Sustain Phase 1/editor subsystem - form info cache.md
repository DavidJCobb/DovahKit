
# Form info cache

The use of the PImpl idiom doesn't work, because when we add new cached data types, we need to add new accessors for them and sometimes new signals as well, and those accessors are currently hosted on the FIC itself.

If we used non-member accessors in the FIC namespace, then we'd benefit at least a little from PImpl, though any time we need signals, things still get messy.
