
# ui.canvas

A layered canvas. Widgets of this type have a fixed size, configurable through script, and can be used to display several graphics layered on top of each other, with different blend modes and opacities. The user can right-click on a canvas and export it as either a flattened image, or "as layers" &mdash; that is, as a ZIP archive containing the individual layer graphics and a text file with layer configuration information.

## Instance methods

<dl>
   <dt>instance:append_layer()</dt>
   <dd>
      Creates, appends, and returns a new `canvas_layer`.
   </dd>
   <dt>instance:append_layer_group()</dt>
   <dd>
      Creates, appends, and returns a new `canvas_layer_group`.
   </dd>
   <dt>instance:remove_layer(which)</dt>
   <dd>
      <p>Removes a layer or layer group. The <var>which</var> argument can be a `canvas_layer` or `canvas_layer_group` object, or an integer indicating the the index of the layer to remove. Throws an error if it receives an argument of the wrong type, if it receives a canvas layer or layer group that isn't part of <em>this</em> canvas, or if it receives an integer index that is out of bounds.</p>
   </dd>
</dl>

## Instance properties

<dl>
   <dt>height</dt>
   <dd>
      The height of the canvas, in pixels.
   </dd>
   <dt>layers</dt>
   <dd>
      A live-updating array of the canvas's layer objects. The array itself is read-only and cannot be overwritten. This lists only direct children; not descendants.
   </dd>
   <dt>width</dt>
   <dd>
      The width of the canvas, in pixels.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.canvas.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.canvas.new()</dt>
   <dd>
      <p>Creates and returns a new canvas. Passing any arguments to this function will throw an error.</p>
   </dd>
</dl>