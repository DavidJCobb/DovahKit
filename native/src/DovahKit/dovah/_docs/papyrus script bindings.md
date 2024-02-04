
# Papyrus script bindings

Papyrus is the scripting language introduced for Skyrim.

Papyrus is similar to Java in that all code must be tied to a class (there's no way to have "free" functions or variables) and you can only define one class per script file. As such, the terms "class" and "script" are often used interchangeably. Inheritance is allowed, and function calls use dynamic dispatch, so all member functions can be overridden.

If a script has static member functions, these can be invoked directly via the script's name, e.g. `Math.Abs(-3)`. In general, however, a script must be *attached* to a form and instantiated during play in order for its code to run. Static data members don't exist; variables and properties defined at script scope exist only on instances of the class.

## Script attachment

In order to understand script attachment (and through it, class instantiation), we must define two terms.

* **Script attachment, a.k.a. BoundScript:** A piece of data on a form, defining a to-be-attached (i.e. to-be-instantiated) script; `BoundScript` is the typename used in Skyrim's game engine to refer to this data. This data specifies: the name of the script to attach; a script status code; and any properties whose values have been assigned via the Creation Kit or similar tools.

* **ScriptObject:** An instance of the script existing at run-time, during play. This term did not exist in Skyrim; it was retroactievly introduced in Fallout 4, which also had all (instantiated) scripts derive from the `ScriptObject` class.

A ScriptObject can be the product of up to two BoundScripts. BoundScripts can be attached to base forms or to ObjectReferences, and it's possible to attach data for the same script to both a reference and its base form, with data on the reference-side BoundScript overriding data on the base-side BoundScript. (It's also possible to specify that a reference *should not* produce a ScriptObject for a given class, even if that class has a BoundScript on the base form, by giving the reference a BoundScript for that class with the "inherited and removed" script status code.)

### Rules for script attachment

* When a BoundScript is attached to a base form, the game will attempt to bind that script to all references created with this base form. (Not sure if that applies retroactively i.e. to already-existing refs if a mod is edited to attach the script.)

  If the script does not derive from ObjectReference, then the game will also attempt to bind it to the base form itself. This means, for example, that an Activator form can have a BoundScript for a class that extends `Activator`, and that class will then be successfully bound to the Activator form *and* unsuccessfully bound (with Papyrus log errors) to all references using that base form.
 
* When a BoundScript is attached to a reference (i.e. ObjectReference), the game will attempt to bind that script directly to that reference.

* When a BoundScript is attached to an alias, the game will attempt to bind that script directly to that alias.

* When a BoundScript is attached to a Magic Effect, the game will attempt to bind that script to any ActiveMagicEffects created from that Magic Effect at run-time.

* At run-time, a BoundScript for class *X* on a form or alias *Y* will only successfully produce a ScriptObject if *X* inherits from the Papyrus class associated with *Y*'s form type or alias type (i.e. reference/location/any). Notably, some form types *do* inherit from each other, e.g. Furniture being a subclass of Activator.
  
  When types are mismatched, or when the to-be-attached class does not inherit from any native type, you will see an error in the Papyrus log resembling the following:

  > error: Unable to bind script *Name* to *FormEditorID* (09000D68) because their base types do not match

  A similar error message appears when a quest alias's BoundScript is mismatched:

  > error: Unable to bind script *Name* to alias *AliasName* on quest *QuestEditorID* (0401AAC8) because their base types do not match

  In turn, any Papyrus properties of type *X* that were set (in the Creation Kit or with similar tools) to *Y* will instead yield None. This also applies to elements of an array of type *X*[]. Error messages will be produced for these properties as well, e.g.

  > Element of property *PropertyName* on script *Name* attached to *FormEditorID* (09000D66) cannot be bound because *TargetFormEditorID* (09000D68) is not the right type  
  > Element of property *PropertyName* on script *Name* attached to *FormEditorID* (09000D66) cannot be bound because alias *AliasName* on quest *QuestEditorID* (09000D67) is not the right type

  * By implication, this means that it's impossible for any Papyrus array to hold, at run-time, both forms and aliases. If the array derives from `Alias`, then it can't hold forms; if it derives from `Form`, then it can't hold aliases; and if it's of a type *X* that doesn't derive from anything, then nothing can be an *X*, so nothing can be placed in the array. I don't know offhand if this is still true post-Skyrim, where Fallout 4 and future games have a root `ScriptObject` type that you could conceivably have an array of.


## Notes on Creation Kit behavior

* If a script doesn't derive from any native class, the Creation Kit will allow you to attach it to any form or alias, even though the script will fail to bind to forms and aliases during play.

* If a Papyrus property's type is *X*, where *X* is a script that doesn't derive from any native class, then the Creation Kit will fail to show any UI for editing the property's value.

  * If the property's type is *X*[] i.e. an array of *X*, then the Creation Kit will show the standard controls for adding, removing, and reordering array elements. However, when an element is selected, its value will change to `<NULL alias> (0) on <NULL quest> (00000000)` (also the default for newly-inserted array elements), and the Creation Kit will fail to show any UI for editing the element's value. This means that without outside tools, you can't actually set any values in these arrays.

    In turn, the Creation Kit doesn't auto-correct these properties <i>en masse</i> when you first open the properties dialog; it only corrects individual values when you "touch" them.