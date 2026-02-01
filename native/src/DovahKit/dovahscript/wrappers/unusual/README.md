
This folder  is for Lua classes that wrap a native object directly  rather than 
through an instance of `wrapper`. As such, there's a one-to-one mapping between 
a userdata and an  instance &mdash; that is, no two userdata  refer to the same 
underlying  object, the userdata are not  tracked in any internal  storage, and 
the class needs to have  its own `__gc` and `__close`  metamethods which manage 
the lifetime of the underlying object.
