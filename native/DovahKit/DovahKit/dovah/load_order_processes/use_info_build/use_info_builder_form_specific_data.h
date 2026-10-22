#pragma once
#include <optional>
#include <set>
#include <unordered_map>
#include "lu/integral_character_code.h"
#include "dovah/types/bare_form_id.h"
namespace dovah::load_order_processes::use_info_build {
   class use_info_builder;
}

namespace dovah::load_order_processes::use_info_build {
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
   class use_info_builder_form_specific_data {
      public:
         struct for_default_object_manager_form {
            std::unordered_map<lu::four_cc, bare_form_id> default_objects; // key is the DOBJ signature
         };
         struct for_navmesh_info_map_form {
            /*// TODO: Put this back in place once NAVI is implemented
            loaded_forms::structs::navmesh_info_map::navmesh_info_collection::form_specific_use_info_data     navmesh_info;
            loaded_forms::structs::navmesh_info_map::precomputed_path_collection::form_specific_use_info_data precomputed_paths;
            loaded_forms::structs::navmesh_info_map::road_marker_map::form_specific_use_info_data             road_markers;
            //*/
            std::set<bare_form_id> deleted_navmeshes;
         };

      public:
         struct {
            std::optional<for_default_object_manager_form> default_object_manager;
            std::optional<for_navmesh_info_map_form>       navmesh_info_map;
         } by_form_type;
         struct {
            /*// TODO: Put this back in place once WRLD is implemented
            std::optional<loaded_forms::structs::large_ref_index::form_specific_use_info_data> large_ref_index;
            //*/
         } by_form_struct;

         void clear();
         void commit(use_info_builder& dst);
   };
}