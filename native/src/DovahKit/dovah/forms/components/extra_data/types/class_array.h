#pragma once
#include "helpers/class_array.h"
namespace dovah::loaded_forms::components::extra_data_types {
   #pragma region A
      class action;
      class activate_parents;
      class alpha_cutoff;
      class ammo;
      class attach_ref;
   #pragma endregion
   #pragma region C
      class cell_acoustic_space;
      class cell_climate;
      class cell_grass_data;
      class cell_imagespace;
      class cell_music_override;
      class cell_region_list;
      class cell_water_type;
      class charge;
      class collision_data;
      class count;
   #pragma endregion
   #pragma region D
      class distant_data;
   #pragma endregion
   #pragma region E
      class emittance_source;
      class enable_state_parent;
      class encounter_zone;
   #pragma endregion
   #pragma region F
      class favor_cost;
   #pragma endregion
   #pragma region G
      class global;
   #pragma endregion
   #pragma region H
      class headtracking_weight;
      class health;
      class health_percent;
      class horse;
   #pragma endregion
   #pragma region I
      class ignored_by_sandbox;
      class interior_lock_list;
   #pragma endregion
   #pragma region L
      class leveled_creature_modifier;
      class leveled_item_base;
      class light;
      class linked_ref;
      class linked_ref_color;
      class lit_water;
      class location;
      class location_ref_type;
      class lock;
   #pragma endregion
   #pragma region M
      class map_marker;
      class merchant_container;
      class multibound_bounds;
      class multibound_ref;
   #pragma endregion
   #pragma region N
      class navmesh_door_portal;
   #pragma endregion
   #pragma region O
      class occlusion_plane;
      class occlusion_plane_ref_data;
      class ownership;
   #pragma endregion
   #pragma region P
      class package_start_location;
      class patrol_ref_data;
      class poison;
      class portal;
      class portal_origin_and_destination;
      class primitive;
   #pragma endregion
   #pragma region R
      class radius;
      class ragdoll_data;
      class random_teleport_marker;
      class rank;
      class reflector_refs;
      class room_ref_data;
   #pragma endregion
   #pragma region S
      class scale;
      class spawn_container;
   #pragma endregion
   #pragma region T
      class teleport;
      class teleport_name;
      class time_left;
   #pragma endregion
   #pragma region W
      class water_current_zone_data;
      class water_data;
      class water_environment_map;
   #pragma endregion

   namespace deprecated {
      class XCET;
      class XDCR;
      class XEDL;
      class XENC;
      class XHRS;
      class XIBS;
      class XLMB;
      class XNVP;
      class XPCI;
      class XRAD;
      class XRDO;
      class XROO;
      class XSED;
      class XSOL;
      class XUSE;
      class XWLT;
      class XWNT;
   }
}

namespace dovah::loaded_forms::components {
   using all_extra_data_types = cobb::class_array<
      #pragma region A
         extra_data_types::action,
         extra_data_types::activate_parents,
         extra_data_types::alpha_cutoff,
         extra_data_types::ammo,
         extra_data_types::attach_ref,
      #pragma endregion
      #pragma region C
         extra_data_types::cell_acoustic_space,
         extra_data_types::cell_climate,
         extra_data_types::cell_grass_data,
         extra_data_types::cell_imagespace,
         extra_data_types::cell_music_override,
         extra_data_types::cell_region_list,
         extra_data_types::cell_water_type,
         extra_data_types::charge,
         extra_data_types::collision_data,
         extra_data_types::count,
      #pragma endregion
      #pragma region D
         extra_data_types::distant_data,
      #pragma endregion
      #pragma region E
         extra_data_types::emittance_source,
         extra_data_types::enable_state_parent,
         extra_data_types::encounter_zone,
      #pragma endregion
      #pragma region F
         extra_data_types::favor_cost,
      #pragma endregion
      #pragma region G
         extra_data_types::global,
      #pragma endregion
      #pragma region H
         extra_data_types::headtracking_weight,
         extra_data_types::health,
         extra_data_types::health_percent,
         extra_data_types::horse,
      #pragma endregion
      #pragma region I
         extra_data_types::ignored_by_sandbox,
         extra_data_types::interior_lock_list,
      #pragma endregion
      #pragma region L
         extra_data_types::leveled_creature_modifier,
         extra_data_types::leveled_item_base,
         extra_data_types::light,
         extra_data_types::linked_ref,
         extra_data_types::linked_ref_color,
         extra_data_types::lit_water,
         extra_data_types::location,
         extra_data_types::location_ref_type,
         extra_data_types::lock,
      #pragma endregion
      #pragma region M
         extra_data_types::map_marker,
         extra_data_types::merchant_container,
         extra_data_types::multibound_bounds,
         extra_data_types::multibound_ref,
      #pragma endregion
      #pragma region N
         extra_data_types::navmesh_door_portal,
      #pragma endregion
      #pragma region O
         extra_data_types::occlusion_plane,
         extra_data_types::occlusion_plane_ref_data,
         extra_data_types::ownership,
      #pragma endregion
      #pragma region P
         extra_data_types::package_start_location,
         extra_data_types::patrol_ref_data,
         extra_data_types::poison,
         extra_data_types::portal,
         extra_data_types::portal_origin_and_destination,
         extra_data_types::primitive,
      #pragma endregion
      #pragma region R
         extra_data_types::radius,
         extra_data_types::ragdoll_data,
         extra_data_types::random_teleport_marker,
         extra_data_types::rank,
         extra_data_types::reflector_refs,
         extra_data_types::room_ref_data,
      #pragma endregion
      #pragma region S
         extra_data_types::scale,
         extra_data_types::spawn_container,
      #pragma endregion
      #pragma region T
         extra_data_types::teleport,
         extra_data_types::teleport_name,
         extra_data_types::time_left,
      #pragma endregion
      #pragma region W
         extra_data_types::water_current_zone_data,
         extra_data_types::water_data,
         extra_data_types::water_environment_map,
      #pragma endregion

      extra_data_types::deprecated::XCET,
      extra_data_types::deprecated::XDCR,
      extra_data_types::deprecated::XEDL,
      extra_data_types::deprecated::XENC,
      extra_data_types::deprecated::XHRS,
      extra_data_types::deprecated::XIBS,
      extra_data_types::deprecated::XLMB,
      extra_data_types::deprecated::XNVP,
      extra_data_types::deprecated::XPCI,
      extra_data_types::deprecated::XRAD,
      extra_data_types::deprecated::XRDO,
      extra_data_types::deprecated::XROO,
      extra_data_types::deprecated::XSED,
      extra_data_types::deprecated::XSOL,
      extra_data_types::deprecated::XUSE,
      extra_data_types::deprecated::XWLT,
      extra_data_types::deprecated::XWNT
   >;
}