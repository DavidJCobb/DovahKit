
# ui.checkbox

This is a subclass of `ui.widget`, and inherits all non-static members.

## Events

### OnChanged

Fires when the checkbox's state changes. Receives the new state as a string argument.

### OnToggled

Fires when the checkbox's state is toggled. Receives a boolean indicating whether the checkbox is now checked.

## Instance properties

<dl>
   <dt>checked</dt>
   <dd>
      Boolean indicating whether the box is checked.
   </dd>
   <dt>state</dt>
   <dd>
      <p>String indicating the checkbox's state; the following case-insensitive values are allowed:</p>
      <ul>
         <li>checked</li>
         <li>indeterminate</li>
         <li>unchecked</li>
      </ul>
   </dd>
   <dt>text</dt>
   <dd>
      A string, to be displayed next to the checkbox as a label. The label counts as part of the checkbox's clickable area; clicking the label also toggles the checkbox.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.checkbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.checkbox.new(text)</dt>
   <dd>
      <p>Creates and returns a new checkbox with the specified label text, if any. Passing any argument other than nil or a string is an error.</p>
   </dd>
</dl>