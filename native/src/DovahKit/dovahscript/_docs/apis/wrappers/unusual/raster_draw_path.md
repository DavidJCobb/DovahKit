
# ui.raster_draw_path

Represents a path that you've queued to draw onto a raster.

## Instance methods

<dl>
   <dt>instance:add_ellipse(options)</dt>
   <dd>
      <p>Queues an ellipse to draw. The options table can contain the following values:</p>
      <dl>
         <dt>center</dt>
         <dd>
            If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>radii</dt>
         <dd>
            <p>The radii of the circle. If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.</p>
            <p>Used only if <var>radius</var> is unspecified.</p>
         </dd>
         <dt>radius</dt>
         <dd>
            The radius of the circle. If specified, this value must be a number.
         </dd>
         <dt>x</dt>
         <dd>
            The X-coordinate of the circle's center, as a number. Used only if <var>center</var> is invalid or unspecified.
         </dd>
         <dt>y</dt>
         <dd>
            The Y-coordinate of the circle's center, as a number. Used only if <var>center</var> is invalid or unspecified.
         </dd>
      </dl>
   </dd>
   <dt>instance:arc_to(by, to, radius)</dt>
   <dd>
      <p>Draws an arc from the current path position, through <var>by</var>, to <var>to</var>, with radius <var>radius</var>. Comparable to the HTML5 Canvas API of the same name.</p>
      <p>The <var>by</var> and <var>to</var> arguments must both be tables or userdata describing path coordinates. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then the points are invalid and an error will be thrown.</p>
   </dd>
   <dt>instance:line_to(coords)</dt>
   <dd>
      Draws a line from the current path position to the specified coordinates. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then the points are invalid and an error will be thrown.
   </dd>
   <dt>instance:move_to(coords)</dt>
   <dd>
      Moves the current path position to the specified coordinates. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then the points are invalid and an error will be thrown.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.raster_draw_path.new()</dt>
   <dd>
      Creates and returns a new instance. Passing any arguments to this function will throw an error.
   </dd>
   <dt>ui.raster_draw_path.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>