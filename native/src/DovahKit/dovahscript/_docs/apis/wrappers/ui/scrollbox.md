
# ui.scrollbox

This is a subclass of `ui.widget`, and inherits all non-static members. The widget is a scrollable pane.

You cannot append child widgets directly to the scrollbox; instead, append them to the scrollbox's <code>body</code>.

## Instance methods

<dl>
   <dt>instance:scroll_to(x, y, w, h)</dt>
   <dd>
      <p>Attempts to scroll to the specified X- and Y-coordinates, which must be integers.</p>
      <p>The <var>w</var> and <var>h</var> arguments are optional, and default to 0 if unspecified or if they aren't integers. If specified, the scrollbox will attempt to center a <var>w</var>-by-<var>h</var>-pixel area with the area's top-left corner located at (<var>x</var>, <var>y</var>).</p>
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.body</dt>
   <dd>
      An automatically-created widget which serves as the scrollbox's "body" &mdash; its interior, holding child widgets. This widget cannot be reparented. This property is read-only, though the value itself can be modified (e.g. to have a layout set).
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.scrollbox.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.scrollbox.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>