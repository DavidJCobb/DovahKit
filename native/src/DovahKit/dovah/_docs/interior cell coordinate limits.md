
# Interior cell coordinate limits

Interior cells in Skyrim make use of a <dfn>center-on-cell marker</dfn>, or <dfn>COC marker</dfn>. These markers act as a convenient debugging tool and as a failsafe during gameplay: debug console commands exist to teleport to any cell, and will place you at the cell's COC marker; and if the player falls out of the map while in any interior space, they'll be swiftly teleported to the COC marker (they get "COC'd").

However, this system imposes certain limitations on interior cells. There appear to be three separate COC mechanisms at play in Skyrim:

* The player and any physics-simulated gameplay objects will be teleported to the COC marker if they travel too far beyond the playable space: the game computes a bounding box that wraps around the cell's pre-placed contents, and keeps gameplay objects inside of this bounding box.

* Static refs pre-placed in the Creation Kit will be teleported to the COC marker upon loading the cell if their X- or Y-coordinate is beyond (+/-)30,000. This happens before computation of the cell's bounding volume. A ref at 30,000 won't be teleported; a ref at 30,000.1 will be teleported.

* Refs will be teleported to the COC marker if the ref's collision model extends past a threshold located somewhere around 113720 world units on the XY plane (i.e. the ref's Z-coordinate is unbounded). This applies to the ref's entire collision model, not just its pivot point, so the exact threshold is difficult to determine experimentally. Notably, however, 113735.78125 world units is exactly 1625 Havok units.

The COC marker itself is never moved by this system. This means that if the COC marker itself is located beyond 114000 world units or such, then the entire cell will collapse into a singularity: all refs are teleported to the same position on every frame, making it impossible for anything to move (though rotation remains possible). Not even the `setpos` command can move a ref beyond the third-listed threshold and keep it there: the ref just gets COC'd on the next frame.