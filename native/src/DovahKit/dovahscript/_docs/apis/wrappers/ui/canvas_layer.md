
# ui.canvas_layer

A layer in a `ui.canvas`.

## Instance methods

<dl>
   <dt>instance:move_backward()</dt>
   <dd>
      Moves the layer backward, behind whatever layer is currently just behind it.
   </dd>
   <dt>instance:move_forward()</dt>
   <dd>
      Moves the layer forward, in front of whatever layer is currently just in front of it.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>blend_mode</dt>
   <dd>
      <p>A string indicating the layer's blend mode. The following values are permitted:</p>
      <dl>
         <dt>normal</dt>
         <dd>
            The layer is drawn directly on top of whatever is behind it. Any alpha-transparent content in the layer is blended normally.
         </dd>
         <dt>add</dt>
         <dd>
            The layer's RGB values are scaled by its alpha values and then added to those of whatever is behind it.
         </dd>
         <dt>burn</dt>
         <dd>
            Description TBA.
         </dd>
         <dt>darken only</dt>
         <dd>
            The layer is only drawn in places where it is darker than whatever is behind it.
         </dd>
         <dt>difference</dt>
         <dd>
            The layer's RGB values are scaled by its alpha values and then subtracted from those of whatever is behind it.
         </dd>
         <dt>dodge</dt>
         <dd>
            Description TBA.
         </dd>
         <dt>hard light</dt>
         <dd>
            Description TBA.
         </dd>
         <dt>lighten only</dt>
         <dd>
            The layer is only drawn in places where it is lighter than whatever is behind it.
         </dd>
         <dt>multiply</dt>
         <dd>
            The layer's RGB values are multipied by whatever is behind it.
         </dd>
         <dt>overlay</dt>
         <dd>
            Description TBA.
         </dd>
         <dt>screen</dt>
         <dd>
            Description TBA.
         </dd>
         <dt>soft light</dt>
         <dd>
            Description TBA.
         </dd>
      </dl>
   </dd>
   <dt>canvas</dt>
   <dd>
      The <code>ui.canvas</code> instance that this layer belongs to. Read-only.
   </dd>
   <dt>data</dt>
   <dd>
      The contents of this layer, to be drawn on the canvas: an instance of <code>raster</code>, <code>dds_resource</code>, or <code>ui.canvas_text_data</code>; or nil.
   </dd>
   <dt>name</dt>
   <dd>
      The internal name of this layer. This value is an empty string by default, and does not need to be unique; however, setting a non-string value will throw an error. This value is not displayed in the UI, but users will see it when exporting a canvas as layers.
   </dd>
   <dt>opacity</dt>
   <dd>
      A number between 0 and 1, inclusive, indicating the layer's opacity. Writing non-numeric values, or values outside of this range, will throw an error.
   </dd>
   <dt>visible</dt>
   <dd>
      A boolean controlling whether the layer is visible. This defaults to false, to allow you to configure new layers before displaying them. Writing any value other than a boolean is an error.
   </dd>
   <dt>x</dt>
   <dd>
      The layer's X-position, relative to that of its parent, or to the whole canvas if the layer is a direct child of the canvas. This value must be an integer.
   </dd>
   <dt>y</dt>
   <dd>
      The layer's Y-position, relative to that of its parent, or to the whole canvas if the layer is a direct child of the canvas. This value must be an integer.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.canvas_layer.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
</dl>