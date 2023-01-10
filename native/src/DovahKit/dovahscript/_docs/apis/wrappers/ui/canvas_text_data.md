
# ui.canvas_text_data

Text data, to be attached to a `ui.canvas_layer` and displayed in a `ui.canvas`.

## Instance properties

<dl>
   <dt>color</dt>
   <dd>
      The color of the text to draw. These take the same format as color options for <code>raster</code> drawing.
   </dd>
   <dt>font</dt>
   <dd>
      A <code>font</code> object describing font options.
   </dd>
   <dt>max_height</dt>
   <dd>
      The maximum height of the area over which text will be drawn; text that overflows out of this area will be cropped. This must be nil (if there is no limit) or an integer.
   </dd>
   <dt>max_width</dt>
   <dd>
      The maximum width of the area over which text will be drawn; text that overflows out of this area will be cropped. This must be nil (if there is no limit) or an integer.
   </dd>
   <dt>text</dt>
   <dd>
      A string containing the text to draw.
   </dd>
   <dt>word_wrap</dt>
   <dd>
      A boolean indicating whether text word-wraps upon reaching the maximum width.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.canvas_text_data.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.canvas_text_data.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments to this function will throw an error.</p>
   </dd>
</dl>