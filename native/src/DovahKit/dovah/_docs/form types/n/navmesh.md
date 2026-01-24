
# Navmesh

A navigation mesh, or navmesh, is a 3D model whose triangles define surfaces that an NPC can walk on.

You may be familiar with an older approach to NPC pathing: waypoint graphs, wherein level designers place individual points all over a level and draw paths between them. Navmeshes are an improvement over this technique because they define whole walkable areas, not dense clusters of individual paths: if an NPC needs to walk around an obstacle, or check whether they're small enough to fit on a given path, they can test against a navmesh polygon's whole area.

In Skyrim, navmeshes can be defined for cells or for base forms. The former is the more common case, and involves a navmesh form existing as a child of a cell form. The latter is a more niche use: <dfn>object navmeshes</dfn> define the walkable areas of an individual placeable object, and are used to help with automatically generating navmeshes for in-game locations.

## Exterior navmeshes

Navmeshes are bound to a single cell. When a walkable area spans multiple adjacent cells, the navmeshes in each cell must store <dfn>edge links</dfn>. An edge link in one navmesh will specify the other navmesh, and a triangle index within that other navmesh. The Creation Kit will display linked edges with thick green lines.

The Creation Kit will create edge links if the following criteria are met:

* The vertices that comprise the edge, in both navmeshes, have nearly exactly identical positions.
* You manually "finalize" either navmesh.

## Object navmeshes

Object navmeshes are stored as children of the default "navmesh gen cell" (form ID `025`), and have a special record flag (`1 << 31`) as well. These navmeshes list their target object via `NAVM/ONAM`.

You can create an object navmesh by right-clicking a base form in the Creation Kit and selecting "Navmesh Object." This will load the navmesh gen cell in the Render Window, and spawn a ref with that base form in the cell; but the Render Window will hide everything except that ref and any navmesh you draw for it.
