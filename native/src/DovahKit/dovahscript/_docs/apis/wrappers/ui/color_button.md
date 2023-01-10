
# ui.color_button

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a button that, when clicked, opens a dialog that the user can use to select a color.

## Events

### OnChanged

Fires when the selected color changes. Passes the new color as an argument.

## Instance properties

<dl>
   <dt>color</dt>
   <dd>
      The currently selected color.
   </dd>
   <dt>has_alpha</dt>
   <dd>
      A boolean indicating whether the user is allowed to modify the color's alpha component.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.color_button.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.color_button.new(color)</dt>
   <dd>
      <p>Creates and returns a new color_button with the specified color, or nil.</p>
   </dd>
</dl>