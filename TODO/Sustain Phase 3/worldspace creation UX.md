
# Worldspace creation UX

Jonx0r, creator of Wyrmstooth, has published [a tutorial](https://old.reddit.com/r/skyrimmods/comments/1s5nvvh/skyrim_new_lands_mod_mega_tutorial_update/) on creating worldspaces. It'd be good to add features and functionality to DovahKit that align with this process.

* Importing a heightmap, creating cells and adjusting landscapes to match it.

  * In addition to supporting typical raster formats, we should also support the `.r32` file extension (32-bit floating-point).

  * TESAnnyn supports the following features:
  
    * Import/export multiple formats (BMP, PNG, CSV, 32-bit floating-point raw)
    * Import/export landscape vertex colors
	 * Set landscape lateral scale (i.e. map pixels to world units)
	   * For every case except one pixel = one vertex, warn about rounding errors and whatnot.
	 * Adjust landscape height on import/export (displace by world units; scale by multiplier)
	 * Indicate location of cell (0, 0) bottom-left corner in the image
	 * Skip importing any landscape within a given height range
	 * Clamp imported landscape heights to a given range

* Content generation via the Region Editor. (This tutorial is valuable for describing edge-cases and demonstrating how a lot of this behavior actually works in practice.)
