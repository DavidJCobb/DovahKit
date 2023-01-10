
# ui.formpicker

This is a subclass of `ui.widget`, and inherits all non-static members. This widget can be used to allow the user to select a form, optionally limiting the types they're allowed to choose from.

The widget may display in either of two formats, depending on how many form types are allowed:

* A single drop-down listing all permitted forms.
* Two drop-downs: one which filters by form type (from among the allowed form types); and another which lists all forms of the currently selected type.

The form types that are allowed by default are the same ones supported by the `GetIsID` condition:

* Base forms:
  * form_types.acoustic_space
  * form_types.activator
  * form_types.actor_base
  * form_types.container
  * form_types.door
  * form_types.flora
  * form_types.furniture
  * form_types.grass
  * form_types.hazard
  * form_types.idle_marker
  * form_types.light
  * form_types.movable_static
  * form_types.projectile
  * form_types.sound
  * form_types.static
  * form_types.talking_activator
  * form_types.tree
* Items:
  * form_types.ammo
  * form_types.armor
  * form_types.armor_addon
  * form_types.book
  * form_types.key
  * form_types.leveled_item
  * form_types.misc_item
  * form_types.potion
  * form_types.scroll
  * form_types.soul_gem
  * form_types.weapon
* Magic: 
  * form_types.enchantment
  * form_types.leveled_spell
  * form_types.shout
  * form_types.spell
* Other:
  * form_types.formlist

## Events

### OnChanged

Fires when the selected form is changed. The newly-selected form, if any, is passed to the listener as an argument.

## Instance methods

<dl>
   <dt>instance:clear()</dt>
   <dd>
      Clears the currently-selected value. If the widget allows users to select none, then none is selected; otherwise, the default form is selected.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.allow_none</dt>
   <dd>
      A boolean controlling whether the user is allowed to select "none."
   </dd>
   <dt>instance.default_form</dt>
   <dd>
      The form that is selected by default.
   </dd>
   <dt>instance.form</dt>
   <dd>
      The currently selected form, if any; or nil otherwise.
   </dd>
   <dt>instance.form_types</dt>
   <dd>
      <p>Reading this property returns a copy of the array of form types that the user is limited to selecting. If the array is empty, then the user can select forms of any type.</p>
      <p>When writing to this property, you can set it to a single form type, to an array of form types, or (if you wish to allow forms of any type) to nil. Regardless of what value you write, subsequent reads will always return an array.</p>
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.formpicker.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.formpicker.new()</dt>
   <dd>
      <p>Creates and returns a new formpicker instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>