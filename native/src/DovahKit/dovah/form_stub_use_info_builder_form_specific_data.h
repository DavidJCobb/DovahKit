#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <set>
#include <unordered_map>
#include <vector>
#include "./forms/structs/navmesh_info_map/navmesh_info_collection.h"
#include "./forms/structs/navmesh_info_map/precomputed_path_collection.h"
#include "./forms/structs/world_large_ref_data.h"
#include "./core.h"

namespace dovah {
   class form_stub_use_info_builder;
}

namespace dovah {
   //
   // Some data structures in forms generate use info in a very complicated way; this class 
   // exists as a storage area for whatever state is needed to manage that.
   // 
   // As an example -- the specific example that motivated this class's creation -- the RNAM 
   // subrecord(s) in a Worldspace are coalesced across all records, and are used to define 
   // a hashmap of cell IDs to one or more refs. We prune duplicate entries from the hashmap 
   // on load (that is: if a ref is listed in the same cell ID more than once, we only retain 
   // one use of that ref), and so we need to do the exact same pruning when generating use 
   // info. There's really no solution but to have a similar hashmap set up during use info 
   // generation, but it'd be wasteful to bolt that hashmap onto every use info builder when 
   // it's only needed by WRLD/RNAM. Therefore, we put it (and any similar structures that 
   // we may discover on other forms) on this class instead. Use info builders have room for 
   // one pointer to this class, and instances are created on demand.
   // 
   // Another example: the Default Object Manager uses a similar hashmap, with FourCCs as 
   // keys and form IDs as values.
   //
   class form_stub_use_info_builder_form_specific_data {
      public:
         struct for_default_object_manager_form {
            std::unordered_map<uint32_t, form_id_t> default_objects; // key is the DOBJ signature
         };
         struct for_navmesh_info_map_form {
            loaded_forms::structs::navmesh_info_map::navmesh_info_collection::form_specific_use_info_data     navmesh_info;
            loaded_forms::structs::navmesh_info_map::precomputed_path_collection::form_specific_use_info_data precomputed_paths;
            std::set<form_id_t, uint32_t> deleted_navmeshes;
         };

      public:
         struct {
            std::optional<for_default_object_manager_form> default_object_manager;
            std::optional<for_navmesh_info_map_form>       navmesh_info_map;
         } by_form_type;
         struct {
            std::optional<loaded_forms::structs::world_large_ref_data::form_specific_use_info_data> world_large_ref_data;
         } by_form_struct;

         void clear();
         void commit(form_stub_use_info_builder& dst);
   };
}