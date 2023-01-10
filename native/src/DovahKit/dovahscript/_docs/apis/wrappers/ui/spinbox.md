
# ui.spinbox

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is an input control that can be used to enter a numeric value.

## Events

### OnChanged

Fires when the control's value changes. Event listeners receive the new value as an argument.

## Instance methods

<dl>
   <dt>instance:step_by(n)</dt>
   <dd>
      Modifies the control's value by <var>n</var> times the control's <var>step</var> property, while respecting the minimum and maximum limits.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>alignment</dt>
   <dd>
      A string controlling the alignment of text within the spinbox.
   </dd>
   <dt>decimals</dt>
   <dd>
      The number of digits allowed after the value's decimal point, or zero to force values to integers.
   </dd>
   <dt>maximum</dt>
   <dd>
      The maximum value allowed by the control. If you write a decimal number, it will be truncated to an integer.
   </dd>
   <dt>minimum</dt>
   <dd>
      The minimum value allowed by the control. If you write a decimal number, it will be truncated to an integer.
   </dd>
   <dt>prefix</dt>
   <dd>
      A string displayed inside the control, before the number. This value must be a string, but it can be an empty string.
   </dd>
   <dt>read_only</dt>
   <dd>
      A boolean controlling whether the control is read-only. A read-only spinbox's value cannot be changed, but the control itself is not disabled; the value can be selected as text and copied.
   </dd>
   <dt>step</dt>
   <dd>
      The amount by which the control's value changes, when the user clicks on the increase or decrease buttons on the control.
   </dd>
   <dt>suffix</dt>
   <dd>
      A string displayed inside the control, after the number. This value must be a string, but it can be an empty string.
   </dd>
   <dt>value</dt>
   <dd>
      The control's current value.
   </dd>
   <dt>wraparound</dt>
   <dd>
      A boolean controlling whether the value wraps around when the user pushes it below its minimum or past its maximum using the step buttons.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.spinbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.spinbox.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>