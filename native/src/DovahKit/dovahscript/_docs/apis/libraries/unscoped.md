
# Global-scope values

## Functions

<dl>
   <dt>object_is_form(x)</dt>
   <dd>
      Returns true if <var>x</var> is a form of any type, or false otherwise.
   </dd>
   <dt>object_is_zombie(x)</dt>
   <dd>
      <p>Returns true if <var>x</var> is a "zombie" object &mdash; that is, an object whose underlying data has been deleted. For example, if you delete a form, then any references to that form from script variables will become "zombified." Zombie objects cannot be interacted with; you cannot call methods or write values to them.</p>
      <p>This function exists to work around an unfortunate technical limitation within the Lua script engine: there is no way to forcibly clear all variables that refer to a given object. There is also no way to create any non-nil, non-false value that tests as nil or as falsy in conditions. This means that when DovahKit offers scripted access to resources that can be explicitly deleted, like forms, there is no way to make any variables that refer to a deleted resource test as falsy; <code>deleted_form != nil and (not not deleted_form) == true</code>.</p>
   </dd>
</dl>