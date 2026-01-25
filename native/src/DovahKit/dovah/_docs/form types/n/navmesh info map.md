
# Navmesh info map

A singleton form which defines long-distance pathfinding data. Navmeshes allow actors to pathfind around a loaded cell; the navmesh info map allows actors to pathfind across entire worldspaces.

This form's primary function is to map `NavMesh` forms to `NavMeshInfo` data structures, but it also stores a list of precomputed paths defining roads.

## Data

### Navmesh info

The `NVMI` subrecord corresponds to a data structure that in the Creation Kit is called `NavMeshInfo`.

### Preferred paths

The `NVPP` subrecord defines a collection of precomputed paths, originating at any navmesh that has a `RoadMarker` ref placed on it. Functionally, this seems to be a waypoint graph, wherein whole navmeshes serve as waypoints. Its structure when loaded seems to be an unordered map of navmeshes (that have `RoadMarker` refs) to paths, wherein a "path" is a vector of navmesh forms (all beginning with the "key" navmesh). So, `std::unordered_map<NavMesh*, std::vector<std::vector<NavMesh*>>>`.

Its structure when serialized is very different. First, a key are serialized as "one-node-long paths;" then, each of its values (i.e. paths) is serialized. After all keys and values are serialized, a mapping of navmeshes to indices is serialized: the navmeshes are the "keys" from the precomputed path map, and the indices indicate the order in which each navmesh was initially serialized. Pseudocode:

```c++
using path_type = std::vector<NavMesh*>;
using NVPP_type = std::unordered_map<NavMesh*, std::vector<path_type>>;

void serialize(const NVPP_type& NVPP) {
   std::unordered_map<NavMesh*, uint32_t> key_ordering;

   //
   // As of January 2026, xEdit's NAVI/NVPP definition refers to this 
   // data as "Precomputed Paths."
   //
   write((uint32_t)NVPP.size());
   {
      uint32_t current = 0;
      for (auto& pair : NVPP) {
         NavMesh* key = pair.first;

         // Serialize a "path" consisting only of the key.
         write((uint32_t)1);
         write(key);

         // Serialize the actual paths.
         for (const path_type& path : pair.second) {
            write((uint32_t)path.size());
            for(NavMesh* node : path)
               write(node);
         }

         key_ordering[key] = current;
         ++current;
      }
   }
   
   //
   // As of January 2026, xEdit's NAVI/NVPP definition refers to this 
   // data as "Road Markers."
   //
   write((uint32_t)key_ordering.size());
   for (auto [navmesh, index] : key_ordering) {
      write(navmesh);
      write(index);
   }
}
```

I have no clue why it's stored this way.