
# ui.radio_button

This is a subclass of `ui.widget`, and inherits all non-static members.

Radio buttons are similar to checkboxes, except that they are used for mutually exclusive selections: when they are grouped using a `radio_group`, selecting one button deselects all others in the group.

## Events

### OnChanged

Fires when the button's state changes. The listener receives one argument: a string indicating the button's new state ("checked" or "unchecked").

### OnToggled

Fires when the button's state changes. The listener receives one argument: a boolean indicating whether the button is now checked.

## Instance properties

<dl>
   <dt>checked</dt>
   <dd>
      A boolean indicating whether the button is currently checked.
   </dd>
   <dt>group</dt>
   <dd>
      A <code>radio_group</code> instance, or nil.
   </dd>
   <dt>id</dt>
   <dd>
      <p>A numeric ID which should uniquely identify the radio button within its <code>radio_group</code>. IDs greater than zero are manually assigned, whereras IDs less than zero are automatically assigned when the radio button is added to a <code>radio_group</code>.</p>
      <p>If the radio button is not currently in a <code>radio_group</code>, then this value is nil, and attempting to write to it throws an error. If the ID you assign is already in use by another radio button in the same group, then an error will be thrown. Removing the radio button from its group will clear its ID, and transferring it to a new group will auto-assign a new ID.</p>
   </dd>
   <dt>text</dt>
   <dd>
      A string, to be displayed next to the radio button as a label. The label counts as part of the radio button's clickable area; clicking the label also toggles the radio button.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.radio_button.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.radio_button.new(text)</dt>
   <dd>
      <p>Creates and returns a new radio button with the specified label text, if any. Passing any argument other than nil or a string is an error.</p>
   </dd>
</dl>