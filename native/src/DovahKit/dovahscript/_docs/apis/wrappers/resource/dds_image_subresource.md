
# dds_image_subresource

Any subresource within a DDS file, e.g.:

* A DDS layer
* A mip-level in a DDS layer
* A cubemap face
* A mip-level in a cubemap face

## Instance methods

<dl>
   <dt>instance:copy_to_raster()</dt>
   <dd>
      Copies the highest-detail graphic within this subresource into a new raster instance, and returns it.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.cubemap_faces</dt>
   <dd>
      <p>Available only on DDS layers. Returns a cubemap face list &mdash; a userdata with the following read-only properties:</p>
      <ul>
         <li>x_neg</li>
         <li>x_pos</li>
         <li>y_neg</li>
         <li>y_pos</li>
         <li>z_neg</li>
         <li>z_pos</li>
      </ul>
   </dd>
   <dt>instance.mipmaps</dt>
   <dd>
      Returns a read-only array of mip-levels for this layer or cubemap face.
   </dd>
</dl>