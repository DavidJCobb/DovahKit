
# ui.line

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a horizontal or vertical rule, usable as a divider between widgets.

## Instance properties

<dl>
   <dt>direction</dt>
   <dd>
      <p>A string controlling the direction in which the line is drawn. The value must be one of the following:</p>
      <dl>
         <dt>h</dt>
         <dt>horizontal</dt>
         <dd>
            A horizontal rule.
         </dd>
         <dt>v</dt>
         <dt>vertical</dt>
         <dd>
            A vertical divider.
         </dd>
      </dl>
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.line.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.line.new(text)</dt>
   <dd>
      <p>Creates and returns a new instance. You can optionally pass one argument: a string setting the line direction. If you don't pass a line direction, then the default is a horizontal rule.</p>
   </dd>
</dl>