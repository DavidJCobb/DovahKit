
# Landscape

Landscape forms define a grid of heightmapped terrain, to be rendered inside of an exterior cell. Each vertex in the grid has a height, a normal, a color, and texture-paint data.

## The grid

A landscape is a 4096x4096-world-unit plane (the same size as a cell); vertices on either axis are spaced 128 world units apart, so the plane is a 33x33-vertex grid. There's an extra vertex on each axis because the landscape has to physically connect to the next landscape in each direction (so, 4096 / 128 + 1 = 33). The 33rd vertex in each row of one landscape should have an identical position in 3D space to the 0th vertex in the same row of the next landscape; ditto for the columns.

The game divides landscape meshes into four quads. Since landscapes are an odd number of vertices, the quads are as well: each quad has &#8968;33 / 2&#8969; = 17 vertices along each side, i.e. a 17x17 grid, with similar overlapping between quads (in the same landscape or adjacent ones) as between whole landscapes.

Consequently, the centerline vertices within a landscape are at cell-relative coordinates X=16 and Y=16 (i.e. 17 - 1 *or* &#8970;33 / 2&#8971;). A vertex at X=16 exists in both the lefthand and righthand quads; a vertex at Y=16 exists in both the bottom and top quads; and a vertex at (16, 16) exists in all four quads.

All of the constants above can be derived from the side length <var>L</var> of a cell in world units, and the vertex distance <var>D</var> in world units. Bethesda could theoretically have used any values for each, provided the former is always a multiple of the latter:

* vertices per cell side <var>C</var> = <var>L</var> &divide; <var>D</var> + 1
* vertices per quad side <var>Q</var> = &#8968;<var>C</var> &divide; 2&#8969;
* centerline coordinate = &#8970;<var>C</var> &divide; 2&#8971; = <var>Q</var> - 1

## In DovahKit

The game defines whole-cell data (e.g. vertex heights, colors, and normals) in a cell-relative grid, and DovahKit stores it the same way. The game defines per-quad data (e.g. texture painting) in quad-relative sub-grids; as of this writing, DovahKit stores those in whole-cell grids as well.[^post-launch-subgrids] Thus it is useful to be able to refer to cell-relative vertices versus quad-relative vertices (i.e. indices and coords both).

[^post-launch-subgrids]: This should be changed post-launch, to make invalid states (e.g. storing a non-zero texture opacity in a bottom-left-quad vertex, but within the bottom-right-quad's layers) unrepresentable.

The following constants related to grid structure are defined in the `dovah::landscapes` namespace (i.e. the `dovah/data/landscapes/` folder):

| Constant | Value | Meaning |
| :- | -: | :- |
| `centerline_vertex_cell_coord` | 16 | Cell-relative X- or Y-coordinate of a vertex that lies on the axis's centerline through the worldspace. |
| `vertex_distance` | 128 | Distance between two adjacent vertices. |
| `vertices_per_cell` | 1089 | Total number of vertices in a cell. |
| `vertices_per_cell_side` | 33 | Length of a whole landscape's side, in vertices. |
| `vertices_per_quad` | 289 | Total number of vertices in a quad. |
| `vertices_per_quad_side` | 17 | Length of a quad's side, in vertices. |

The following helper functions are defined in the `dovah::utils::landscapes` namespace (i.e. the `dovah/utils/landscapes/` folder):

| Identifier | Purpose |
| :- | :- |
| `cell_vertex_coords_to_quad_vertex_coords` | Given a quad and a pair of cell-relative vertex coords, converts to quad-relative vertex coords. Does not verify that these coords lie inside the given quad. |
| `cell_vertex_index_to_quad_vertex_index` | Given a linear index of a cell-relative vertex, converts to a quad-relative index. |
| `containing_quad_of_cell_vertex_coords` | Returns the quad that contains a set of cell-relative vertex coords. If the vertex exists in multiple quads, biases toward the top and right sides of the grid. |
| `quad_contains_cell_vertex_coords` | Tests whether the specified quad contains the specified cell-relative vertex coords. |
| `quad_vertex_coords_to_cell_vertex_coords` | Given a quad and a pair of quad-relative vertex coords, converts to cell-relative vertex coords. |
