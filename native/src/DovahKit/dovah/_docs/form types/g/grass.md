
# Grass

Oblivion introduced the Grass form type. Grass in that game and newer games is rendered as instanced geometry which is produced on any heightmapped terrain painted with appropriately configured LandTexture forms.

A Grass form consists of a 3D model, and settings for where the grass should spawn (based on terrain slope, altitude relative to a cell's water plane, et cetera).

## Notes

### Historical information

* Prior to Oblivion, tufts of grass seen in the game world were Static forms. Oblivion introduced Grass forms as we know them today. At some point during Oblivion's development, however, Grass was originally developed for use as a base form. There are remnants of this in two places: [Region](./../r/region.md) forms can define a mapping of LandTexture to Grass forms; and the `REFR` loader has a specific warning if grass is set as a base form, saying that it's no longer allowed.
