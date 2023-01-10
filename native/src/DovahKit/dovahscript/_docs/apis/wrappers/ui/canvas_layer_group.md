
# ui.canvas_layer

A layer group in a `ui.canvas`.

Children of a layer group will be positioned relative to the group's own position, and will be affected by the group's opacity, blend mode, and similar settings.

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
   <dt>instance:move_backward()</dt>
   <dd>
      Moves the layer group backward, behind whatever layer is currently just behind it.
   </dd>
   <dt>instance:move_forward()</dt>
   <dd>
      Moves the layer group forward, in front of whatever layer is currently just in front of it.
   </dd>
   <dt>instance:remove_layer(which)</dt>
   <dd>
      <p>Removes a layer or layer group. The <var>which</var> argument can be a `canvas_layer` or `canvas_layer_group` object, or an integer indicating the the index of the layer to remove. Throws an error if it receives an argument of the wrong type, if it receives a canvas layer or layer group that isn't part of <em>this</em> layer group, or if it receives an integer index that is out of bounds.</p>
   </dd>
</dl>

## Instance properties

<dl>
   <dt>blend_mode</dt>
   <dd>
      <p>A string indicating the layer group's blend mode. Works the same as on <code>ui.canvas_layer</code>.</p>
   </dd>
   <dt>canvas</dt>
   <dd>
      The <code>ui.canvas</code> instance that this layer group belongs to. Read-only.
   </dd>
   <dt>layers</dt>
   <dd>
      A live-updating array of this group's child layer objects. The array itself is read-only and cannot be overwritten.
   </dd>
   <dt>name</dt>
   <dd>
      The internal name of this layer group. This value is an empty string by default, and does not need to be unique; however, setting a non-string value will throw an error. This value is not displayed in the UI, but users will see it when exporting a canvas as layers.
   </dd>
   <dt>opacity</dt>
   <dd>
      A number between 0 and 1, inclusive, indicating the layer group's opacity. Writing non-numeric values, or values outside of this range, will throw an error.
   </dd>
   <dt>visible</dt>
   <dd>
      A boolean controlling whether the layer group is visible. This defaults to false, to allow you to configure new layers before displaying them. Writing any value other than a boolean is an error.
   </dd>
   <dt>x</dt>
   <dd>
      The layer group's X-position, relative to that of its parent, or to the whole canvas if the layer group is a direct child of the canvas. This value must be an integer.
   </dd>
   <dt>y</dt>
   <dd>
      The layer group's Y-position, relative to that of its parent, or to the whole canvas if the layer group is a direct child of the canvas. This value must be an integer.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.canvas_layer.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>