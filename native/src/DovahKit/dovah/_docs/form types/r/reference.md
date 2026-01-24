
# Reference

## Specific data

### Load doors

A pair of load doors each have teleport data (`ExtraTeleport`) linking the doors together. The teleport data includes the absolute position and rotation of a <dfn>teleport marker</dfn> indicating where an actor will be moved upon using the door: each door's teleport marker pertains to the opposite door's containing cell or world.

Load doors must be linked to the navmesh; refer to [Navmesh form documentation](./../n/navmesh.md) for information.
