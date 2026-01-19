
# Regions

The primary purpose served by regions is to demarcate areas of physical space within a worldspace which have common properties. A single region form may be used to draw one or more areas within a single worldspace. Each <dfn>region area</dfn> is a non-self-intersecting polygon defined on the XY-plane &mdash; so, a shape as seen from above.

The properties that can be set on a region include:

* Whether the region's areas should be considered playable space (i.e. whether it is a "border region")
* The weather and ambient sounds that can be encountered within the region's areas
* The objects that the Creation Kit's procedural generation should spawn in the region's areas

In addition to the purpose above, regions can also be used to hold music and ambient sound settings to be shared amongst interior cells.

## Notes

* Each `REGN` defines its own polygonal areas. However, every single exterior cell intersected by any of a region's areas will also hold a reference to the region, by way of `CELL/XCLR`, the cell's region list.

* When drawing region areas in the Creation Kit, drawing an area over cells that don't yet exist will create those cells (assuming the area is otherwise valid and you do not cancel its creation).

* The Creation Kit allows you to set the "Edge Fall-Off" for a region form as a whole. However, in the underlying data, each of a region's areas may have a different fall-off value.

### Edge-cases

* The `REGN/WNAM` subrecord indicates the worldspace that the region has been used in. However, it's possible for a region to be used in a worldspace without having `WNAM` set. In these cases, the Creation Kit will automatically backfill `WNAM` based on the first-loaded `CELL/XCLR` subrecord which uses the given region, which means that in essence, even `CELL/XCLR` in a losing record can override `REGN/WNAM` from afar.

  In other words: to know what worldspace a region belongs to, you must check `REGN/WNAM` and, if that's null or not a worldspace form, you must then check every `CELL/XCLR` (from every record, winning or losing) that refers to the region in question. Given the earliest-loading exterior cell record (i.e. a cell that has a parent worldspace record) that contains such an `XCLR`, grab the cell's parent worldspace.

### Historical information

* Prior to Oblivion, tufts of grass seen in the game world were Static forms. Oblivion introduced Grass forms, which define pieces of instanced geometry that can sprout out from LandTextures automatically. At some point during Oblivion's development, however, Grass was originally developed for use as a base form. There are remnants of this in two places: region forms can define a mapping of LandTexture to Grass forms; and the `REFR` loader has a specific warning if grass is set as a base form, saying that it's no longer allowed.
