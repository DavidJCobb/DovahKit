#pragma once
#include <vector>
#include "../../../core.h"
#include "../package_event_dialogue.h"

namespace dovah::loaded_forms::components {
   class extra_data_use_info_state {
      public:
         union {
            struct {
               struct {
                  form_id_t xczr;
               } unknown;
               //
               struct {
                  form_id_t type;
               } ammo;
               form_id_t attach_ref;
               form_id_t cell_acoustic_space;
               form_id_t cell_climate;
               form_id_t cell_imagespace;
               form_id_t cell_music_override;
               form_id_t cell_water_type;
               form_id_t emittance_source;
               struct {
                  form_id_t ref;
               } enable_state_parent;
               form_id_t encounter_zone;
               form_id_t global;
               form_id_t horse;
               form_id_t interior_lock_list;
               form_id_t leveled_item_base;
               form_id_t location;
               form_id_t location_ref_type;
               struct {
                  form_id_t key;
               } lock;
               form_id_t merchant_container;
               form_id_t multibound_ref;
               struct {
                  form_id_t navmesh;
               } navmesh_door_portal;
               form_id_t ownership;
               struct {
                  form_id_t type;
               } poison;
               struct {
                  form_id_t origin;
                  form_id_t destination;
               } portal_origin_and_destination;
               form_id_t random_teleport_marker;
               struct {
                  form_id_t lighting_template;
                  form_id_t imagespace;
               } room_ref_data;
               form_id_t spawn_container;
               struct {
                  form_id_t target_door;
               } teleport;
               form_id_t teleport_name;
               struct {
                  form_id_t cell;
               } water_current_zone_data;
            } by_name;
            std::array<form_id_t, 32> list = {};
         } form_ids;
         std::vector<form_id_t> cell_region_list;
         loaded_forms::components::package_event_dialogue::use_info_state patrol_ref_data;
         struct {
            std::set<form_id_t> refs;
         } water_current_zone_data;

         static_assert(sizeof(form_ids.list) >= sizeof(form_ids.by_name), "The array has too few elements.");
         static_assert(sizeof(form_ids.list) <= sizeof(form_ids.by_name), "The array has too many elements.");

         void clear();
         void commit_to(form_stub_use_info_builder&);
   };
}