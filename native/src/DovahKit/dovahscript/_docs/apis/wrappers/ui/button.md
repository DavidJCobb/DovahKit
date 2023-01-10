
# ui.button

This is a subclass of `ui.widget`, and inherits all non-static members.

## Events

### OnActivated

Fires when the button is clicked, or when it's pressed via keyboard navigation.

### OnCheckStateChanged

Currently unimplemented. If implemented, it would fire for "checkable" buttons, which "stick" down when clicked initially and pop back up when clicked again.

## Instance properties

<dl>
   <dt>auto_default</dt>
   <dd>
      A boolean value indicating whether this button can automatically be made the default button (triggered by pressing Enter) for its containing window. Defaults to false. Writing any value other than a boolean is an error.
   </dd>
   <dt>flat</dt>
   <dd>
      A boolean value indicating whether this button is styled without a border or other decoration. Defaults to false. Writing any value other than a boolean is an error.
   </dd>
   <dt>text</dt>
   <dd>
      The button's label text. Writing any value other than a string is an error.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.button.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.button.new(text)</dt>
   <dd>
      <p>Creates and returns a new button with the specified label text, if any. Passing any argument other than nil or a string is an error.</p>
   </dd>
</dl>