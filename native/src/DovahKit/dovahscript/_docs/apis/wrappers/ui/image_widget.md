
# ui.image_widget

This is a subclass of `ui.widget`, and inherits all non-static members. The widget can be used to display a scripted resource.

## Instance properties

<dl>
   <dt>image</dt>
   <dd>
      A <code>dds_resource</code> or <code>raster</code> to display, or nil.
   </dd>
</dl>

## Static methods

<dl>
   <dt>ui.image_widget.is(arg)</dt>
   <dd>
      Tests whether <var>arg</var> is an instance of this class. Returns a boolean.
   </dd>
   <dt>ui.image_widget.new()</dt>
   <dd>
      <p>Creates and returns a new instance. Passing any arguments will throw an error.</p>
   </dd>
</dl>