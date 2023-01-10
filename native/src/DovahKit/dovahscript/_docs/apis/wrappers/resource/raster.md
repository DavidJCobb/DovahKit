
# raster

Represents a raster image &mdash; that is, an image made up of a grid of pixels.

## Notes

For consistency with Lua as a whole, the top-left pixel in a raster is at coordinates (1, 1). If a function expects coordinates, then passing non-integer, negative, zero, or out-of-bounds values will throw an error.

### Drawing options

Functions that allow you to draw pixels or shapes will generally take an options table as an argument. The following options can be set on this table. Functions will list which options they require.

<dl>
   <dt>fill_color</dt>
   <dd>
      A color, used to fill in a region of space. Defaults to transparent.
   </dd>
   <dt>fill_gradient</dt>
   <dd>
      <p>A table or userdata containing configuration information for a gradient. Overrides <code>fill_color</code> if both are specified.</p>
      <dl>
         <dt>angle</dt>
         <dd>
            A number of degrees. Required for conical and linear gradients; otherwise unused.
         </dd>
         <dt>center</dt>
         <dd>
            The centerpoint of the gradient; optional for linear gradients; required for conical and radial gradients. If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>type</dt>
         <dd>
            A string or a value convertible to a string. Recognized types are <code>"conical"</code>, <code>"linear"</code>, and <code>"radial"</code>. Values are case-insensitive. Unrecognized values will throw an error.
         </dd>
         <dt>stops</dt>
         <dd>
            An array containing at least one element. All elements must be of the form <code>{ position, color }</code>, where the position is a number between 0 and 1, inclusive. Color stops do not have to be in order. If multiple color stops have the same position, then some may be discarded; no guarantee is made as to which of such color stops are retained.
         </dd>
      </dl>
   </dd>
   <dt>line_color</dt>
   <dd>
      A color, used to outline a region of space. Defaults to transparent.
   </dd>
   <dt>line_join</dt>
   <dd>
      A string, used to specify how corners in outlined shapes are handled. Allowed values are "bevel", "miter", and "round". Values are case-insensitive.
   </dd>
   <dt>line_width</dt>
   <dd>
      The width of the drawn outline. Defaults to 1.
   </dd>
</dl>

## Instance methods

<dl>
   <dt>instance:blend_pixel(x, y, color)</dt>
   <dd>
      Blends the passed-in <var>color</var> onto the pixel at (<var>x</var>, <var>y</var>). Throws an error if the specified coordinates are out of bounds.
   </dd>
   <dt>instance:draw_ellipse(options)</dt>
   <dd>
      <p>Draws an ellipse. The options table can contain the following values, in addition to any of the drawing options described above:</p>
      <dl>
         <dt>angle</dt>
         <dd>
            The counterclockwise angle in degrees by which the ellipse will be rotated. If unspecified, it defaults to zero; if specified, it must be a number, or an error will be thrown.
         </dd>
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
            The X-coordinate of the circle's center, as a number. Used only if <var>center</var> is invalid or unspecified, in which case it must be a valid number.
         </dd>
         <dt>y</dt>
         <dd>
            The Y-coordinate of the circle's center, as a number. Used only if <var>center</var> is invalid or unspecified, in which case it must be a valid number.
         </dd>
      </dl>
   </dd>
   <dt>instance:draw_line(options)</dt>
   <dd>
      <p>Draws a line. The options table can contain the following values, in addition to the drawing options described above (of which <code>line_color</code> is required, <code>line_width</code> is optional, and all others are ignored).</p>
      <dl>
         <dt>from</dt>
         <dd>
            If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>to</dt>
         <dd>
            If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
      </dl>
   </dd>
   <dt>instance:draw_path(options)</dt>
   <dd>
      <p>Draws a path. The options table can contain the following values, in addition to any of the drawing options described above:</p>
      <dl>
         <dt>offset</dt>
         <dd>
            Optional. If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>path</dt>
         <dd>
            An instance of <code>ui.raster_draw_path</code>. Required.
         </dd>
      </dl>
   </dd>
   <dt>instance:draw_raster(x, y, raster)</dt>
   <dd>
      <p>Draws another raster overtop this one, placing the other raster's top-left corner at (<var>x</var>, <var>y</var>) on this raster.</p>
   </dd>
   <dt>instance:draw_rect(options)</dt>
   <dd>
      <p>Draws a rectangle. The options table can contain the following values, in addition to any of the drawing options described above:</p>
      <dl>
         <dt>from</dt>
         <dd>
            Optional: coordinates for one corner of the rectangle. If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>h</dt>
         <dd>
            The height of the rectangle. This value is only checked if <var>to</var> is unspecified, and in that case must be a number.
         </dd>
         <dt>height</dt>
         <dd>
            The height of the rectangle. This value is only checked if <var>to</var> and <var>h</var> are unspecified, and in that case must be a number.
         </dd>
         <dt>to</dt>
         <dd>
            Optional: coordinates for the corner of the rectangle opposite that described by the <var>from</var> option. If specified, this value must be a table or userdata. Given a table or userdata <var>T</var>, this function will use <var>T.x</var> if available or <var>T[1]</var> otherwise; and <var>T.y</var> if available or <var>T[2]</var> otherwise. These must be numbers or strings convertible to numbers; if none are found, then this option is invalid.
         </dd>
         <dt>w</dt>
         <dd>
            The width of the rectangle. This value is only checked if <var>to</var> is unspecified, and in that case must be a number.
         </dd>
         <dt>width</dt>
         <dd>
            The width of the rectangle. This value is only checked if <var>to</var> and <var>w</var> are unspecified, and in that case must be a number.
         </dd>
         <dt>x</dt>
         <dd>
            The X-coordinate for the rectangle's first corner. This value is only checked if <var>from</var> is unspecified, and in that case must be a valid integer coordinate.
         </dd>
         <dt>y</dt>
         <dd>
            The Y-coordinate for the rectangle's first corner. This value is only checked if <var>from</var> is unspecified, and in that case must be a valid integer coordinate.
         </dd>
      </dl>
   </dd>
   <dt>instance:fill(color)</dt>
   <dd>
      Sets all pixels in the raster to the specified color.
   </dd>
   <dt>instance:fill_rgb(color)</dt>
   <dd>
      Sets the RGB values of all pixels in the raster to the specified color, while leaving pixels' alpha values unaltered.
   </dd>
   <dt>instance:fill_rgb(direction)</dt>
   <dd>
      Flips the image on the specified direction(s). The <var>direction</var> argument must be a case-insensitive string with one of the following values:
      <dl>
         <dt>both</dt>
         <dd>
            The image is flipped both horizontally and vertically.
         </dd>
         <dt>h</dt>
         <dt>horizontal</dt>
         <dd>
            The image is flipped horizontally.
         </dd>
         <dt>v</dt>
         <dt>vertical</dt>
         <dd>
            The image is flipped vertically.
         </dd>
      </dl>
   </dd>
   <dt>instance:get_pixel(x, y)</dt>
   <dd>
      Returns the color of the pixel at the specified coordinates. Returned colors will have both named and indexed fields, i.e. <code>result[1] == result.r</code>.
   </dd>
   <dt>instance:levels(options)</dt>
   <dd>
      Applies a "levels" filter to the image. Options are as follows:
      <dl>
         <dt>from</dt>
         <dd>
            A table or userdata value <var>T</var> describing a range from 0 to 255, inclusive. The function will use: <var>T.min</var> if present, or <var>T[1]</var> otherwise; and <var>T.max</var> if present, or <var>T[2]</var> otherwise.
         </dd>
         <dt>gamma</dt>
         <dd>
            A number between 0 and 1, inclusive. If specified, out-of-bounds values will throw an error; if unspecified, a warning is emitted and 1.0 is assumed.
         </dd>
         <dt>to</dt>
         <dd>
            A table or userdata value <var>T</var> describing a range from 0 to 255, inclusive. The function will use: <var>T.min</var> if present, or <var>T[1]</var> otherwise; and <var>T.max</var> if present, or <var>T[2]</var> otherwise.
         </dd>
      </dl>
   </dd>
   <dt>instance:resize(w, h)</dt>
   <dd>
      Resizes the raster to the specified size, scaling its existing contents. The arguments must be positive non-zero integers.
   </dd>
   <dt>instance:scale(x, y)</dt>
   <dd>
      Resizes the raster, multiplying its width by <var>x</var> and its height by either <var>y</var>, if present, or <var>x</var> otherwise. The <var>x</var> argument is required and must be a positive non-zero number, where 1.0 is 100% size. The <var>y</var> argument is optional but, if specified, must be a positive non-zero number, where 1.0 is 100% size.
   </dd>
   <dt>instance:set_pixel(x, y, color)</dt>
   <dd>
      Sets the color of the pixel at the given coordinates. No blending is performed; the pixel is wholly replaced.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>height</dt>
   <dd>
      The raster's height, in pixels. You can modify this value to resize the raster without scaling its existing contents, but you must pass in a positive non-zero integer, or a string convertible to one.
   </dd>
   <dt>width</dt>
   <dd>
      The raster's width, in pixels. You can modify this value to resize the raster without scaling its existing contents, but you must pass in a positive non-zero integer, or a string convertible to one.
   </dd>
</dl>

## Static methods

<dl>
   <dt>raster.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>raster.new(options)</dt>
   <dd>
      <p>Creates and returns a new raster. The <var>options</var> argument must contain a table or userdata and can define the following fields:</p>
      <dl>
         <dt>background_color</dt>
         <dd>
            The color used for the new raster's pixels; if unspecified, zero-opacity black is the default.
         </dd>
         <dt>height</dt>
         <dd>
            The new raster's height in pixels. This must be specified, and must be a positive non-zero integer.
         </dd>
         <dt>width</dt>
         <dd>
            The new raster's width in pixels. This must be specified, and must be a positive non-zero integer.
         </dd>
      </dl>
   </dd>
</dl>