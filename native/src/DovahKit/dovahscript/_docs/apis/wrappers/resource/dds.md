
# dds_resource

Represents a loaded DDS texture file.

## Instance methods

<dl>
   <dt>instance:copy_to_raster()</dt>
   <dd>
      Copies the highest-detail mip-level of the texture's topmost layer into a new raster instance, and returns it.
   </dd>
</dl>

## Instance properties

<dl>
   <dt>instance.images</dt>
   <dd>
      Allows access to the texture's layers. You can access entries by index and query the number of entries as you would for any normal array, e.g. <code>instance.images[#instance.images]</code> to get the last entry.
   </dd>
   <dt>instance.is_cubemap</dt>
   <dd>
      A boolean value indicating whether the DDS file is a cubemap. Read-only.
   </dd>
</dl>
