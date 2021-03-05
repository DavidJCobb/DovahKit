#pragma once
#pragma region All extra-data classes
   #include "_unknown.h"
   #pragma region A
      #include "action.h"
      #include "activate_parent_data.h"
      #include "activate_ref.h"
      #include "alpha_cutoff.h"
      #include "ammo.h"
      #include "attach_ref.h"
   #pragma endregion
   #pragma region C
      #include "cell_acoustic_space.h"
      #include "cell_climate.h"
      #include "cell_grass_data.h"
      #include "cell_imagespace.h"
      #include "cell_music_override.h"
      #include "cell_region_list.h"
      #include "cell_water_type.h"
      #include "charge.h"
      #include "collision_data.h"
      #include "count.h"
   #pragma endregion
   #include "distant_data.h"
   #pragma region E
      #include "emittance_source.h"
      #include "enable_state_parent.h"
      #include "encounter_zone.h"
   #pragma endregion
   #include "favor_cost.h"
   #include "global.h"
   #pragma region H
      #include "headtracking_weight.h"
      #include "health.h"
      #include "health_percent.h"
      #include "horse.h"
   #pragma endregion
   #pragma region I
      #include "ignored_by_sandbox.h"
      #include "interior_lock_list.h"
   #pragma endregion
   #pragma region L
      #include "leveled_creature_modifier.h"
      #include "leveled_item_base.h"
      #include "light.h"
      #include "linked_ref.h"
      #include "linked_ref_color.h"
      #include "lit_water.h"
      #include "location.h"
      #include "location_ref_type.h"
      #include "lock.h"
   #pragma endregion
   #pragma region M
      #include "map_marker.h"
      #include "merchant_container.h"
      #include "multibound_bounds.h"
      #include "multibound_ref.h"
   #pragma endregion
   #include "navmesh_door_portal.h"
   #pragma region O
      #include "occlusion_plane.h"
      #include "occlusion_plane_ref_data.h"
      #include "ownership.h"
   #pragma endregion
   #pragma region P
      #include "package_start_location.h"
      #include "patrol_ref_data.h"
      #include "poison.h"
      #include "portal.h"
      #include "portal_origin_and_destination.h"
      #include "primitive.h"
   #pragma endregion
   #pragma region R
      #include "radius.h"
      #include "ragdoll_data.h"
      #include "rank.h"
      #include "random_teleport_marker.h"
      #include "reflector_refs.h"
      #include "room_ref_data.h"
   #pragma endregion
   #pragma region S
      #include "scale.h"
      #include "spawn_container.h"
   #pragma endregion
   #pragma region T
      #include "teleport.h"
      #include "teleport_name.h"
      #include "time_left.h"
   #pragma endregion
   #pragma region W
      #include "water_current_zone_data.h"
      #include "water_data.h"
      #include "water_environment_map.h"
   #pragma endregion
#pragma endregion

namespace dovah::loaded_forms::components::extra {
   template<extra_data_type t> struct class_for_type { using type = basic_extra_data; };
   #pragma region class_for_type
      template<> struct class_for_type<extra_data_type::unknown_xcza> { using type = unknown::XCZA; };
      template<> struct class_for_type<extra_data_type::unknown_xczc> { using type = unknown::XCZC; };
      template<> struct class_for_type<extra_data_type::unknown_xczr> { using type = unknown::XCZR; };
      template<> struct class_for_type<extra_data_type::unknown_xedl> { using type = unknown::XEDL; };
      template<> struct class_for_type<extra_data_type::unknown_xenc> { using type = unknown::XENC; };
      template<> struct class_for_type<extra_data_type::unknown_xlmb> { using type = unknown::XLMB; };
      template<> struct class_for_type<extra_data_type::unknown_xnvp> { using type = unknown::XNVP; };
      template<> struct class_for_type<extra_data_type::unknown_xpsl> { using type = unknown::XPSL; };
      template<> struct class_for_type<extra_data_type::unknown_xroo> { using type = unknown::XROO; };
      template<> struct class_for_type<extra_data_type::unknown_xuse> { using type = unknown::XUSE; };
      template<> struct class_for_type<extra_data_type::unknown_xwcs> { using type = unknown::XWCS; };
      template<> struct class_for_type<extra_data_type::unknown_xwlt> { using type = unknown::XWLT; };
      template<> struct class_for_type<extra_data_type::unknown_xwnt> { using type = unknown::XWNT; };
      template<> struct class_for_type<extra_data_type::deprecated_xcet> { using type = deprecated::XCET; };
      template<> struct class_for_type<extra_data_type::deprecated_xdcr> { using type = deprecated::XDCR; };
      template<> struct class_for_type<extra_data_type::deprecated_xhrs> { using type = deprecated::XHRS; };
      template<> struct class_for_type<extra_data_type::deprecated_xibs> { using type = deprecated::XIBS; };
      template<> struct class_for_type<extra_data_type::deprecated_xpci> { using type = deprecated::XPCI; };
      template<> struct class_for_type<extra_data_type::deprecated_xrad> { using type = deprecated::XRAD; };
      template<> struct class_for_type<extra_data_type::deprecated_xrdo> { using type = deprecated::XRDO; };
      template<> struct class_for_type<extra_data_type::deprecated_xsed> { using type = deprecated::XSED; };
      template<> struct class_for_type<extra_data_type::deprecated_xsol> { using type = deprecated::XSOL; };
      #pragma region A
         template<> struct class_for_type<extra_data_type::action> { using type = action; };
         template<> struct class_for_type<extra_data_type::activate_parent_data> { using type = activate_parent_data; };
         template<> struct class_for_type<extra_data_type::activate_ref> { using type = activate_ref; };
         template<> struct class_for_type<extra_data_type::alpha_cutoff> { using type = alpha_cutoff; };
         template<> struct class_for_type<extra_data_type::ammo> { using type = ammo; };
         template<> struct class_for_type<extra_data_type::attach_ref> { using type = attach_ref; };
      #pragma endregion
      #pragma region C
         template<> struct class_for_type<extra_data_type::cell_acoustic_space> { using type = cell_acoustic_space; };
         template<> struct class_for_type<extra_data_type::cell_climate> { using type = cell_climate; };
         template<> struct class_for_type<extra_data_type::cell_grass_data> { using type = cell_grass_data; };
         template<> struct class_for_type<extra_data_type::cell_imagespace> { using type = cell_imagespace; };
         template<> struct class_for_type<extra_data_type::cell_music_override> { using type = cell_music_override; };
         template<> struct class_for_type<extra_data_type::cell_region_list> { using type = cell_region_list; };
         template<> struct class_for_type<extra_data_type::cell_water_type> { using type = cell_water_type; };
         template<> struct class_for_type<extra_data_type::charge> { using type = charge; };
         template<> struct class_for_type<extra_data_type::collision_data> { using type = collision_data; };
         template<> struct class_for_type<extra_data_type::count> { using type = count; };
      #pragma endregion
      template<> struct class_for_type<extra_data_type::distant_data> { using type = distant_data; };
      #pragma region E
         template<> struct class_for_type<extra_data_type::emittance_source> { using type = emittance_source; };
         template<> struct class_for_type<extra_data_type::enable_state_parent> { using type = enable_state_parent; };
         template<> struct class_for_type<extra_data_type::encounter_zone> { using type = encounter_zone; };
      #pragma endregion
      template<> struct class_for_type<extra_data_type::favor_cost> { using type = favor_cost; };
      template<> struct class_for_type<extra_data_type::global> { using type = global; };
      #pragma region H
         template<> struct class_for_type<extra_data_type::headtracking_weight> { using type = headtracking_weight; };
         template<> struct class_for_type<extra_data_type::health> { using type = health; };
         template<> struct class_for_type<extra_data_type::health_percent> { using type = health_percent; };
         template<> struct class_for_type<extra_data_type::horse> { using type = horse; };
      #pragma endregion
      #pragma region I
         template<> struct class_for_type<extra_data_type::ignored_by_sandbox> { using type = ignored_by_sandbox; };
         template<> struct class_for_type<extra_data_type::interior_lock_list> { using type = interior_lock_list; };
      #pragma endregion
      #pragma region L
         template<> struct class_for_type<extra_data_type::leveled_creature_modifier> { using type = leveled_creature_modifier; };
         template<> struct class_for_type<extra_data_type::leveled_item_base> { using type = leveled_item_base; };
         template<> struct class_for_type<extra_data_type::light> { using type = light; };
         template<> struct class_for_type<extra_data_type::linked_ref> { using type = linked_ref; };
         template<> struct class_for_type<extra_data_type::linked_ref_color> { using type = linked_ref_color; };
         template<> struct class_for_type<extra_data_type::lit_water> { using type = lit_water; };
         template<> struct class_for_type<extra_data_type::location> { using type = location; };
         template<> struct class_for_type<extra_data_type::location_ref_type> { using type = location_ref_type; };
         template<> struct class_for_type<extra_data_type::lock> { using type = lock; };
      #pragma endregion
      #pragma region M
         template<> struct class_for_type<extra_data_type::map_marker> { using type = map_marker; };
         template<> struct class_for_type<extra_data_type::merchant_container> { using type = merchant_container; };
         template<> struct class_for_type<extra_data_type::multibound_bounds> { using type = multibound_bounds; };
         template<> struct class_for_type<extra_data_type::multibound_ref> { using type = multibound_ref; };
      #pragma endregion
      template<> struct class_for_type<extra_data_type::navmesh_door_portal> { using type = navmesh_door_portal; };
      #pragma region O
         template<> struct class_for_type<extra_data_type::occlusion_plane> { using type = occlusion_plane; };
         template<> struct class_for_type<extra_data_type::occlusion_plane_ref_data> { using type = occlusion_plane_ref_data; };
         template<> struct class_for_type<extra_data_type::ownership> { using type = ownership; };
      #pragma endregion
      #pragma region P
         template<> struct class_for_type<extra_data_type::package_start_location> { using type = package_start_location; };
         template<> struct class_for_type<extra_data_type::patrol_ref_data> { using type = patrol_ref_data; };
         template<> struct class_for_type<extra_data_type::poison> { using type = poison; };
         template<> struct class_for_type<extra_data_type::portal> { using type = portal; };
         template<> struct class_for_type<extra_data_type::portal_origin_and_destination> { using type = portal_origin_and_destination; };
         template<> struct class_for_type<extra_data_type::primitive> { using type = primitive; };
      #pragma endregion
      #pragma region R
         template<> struct class_for_type<extra_data_type::radius> { using type = radius; };
         template<> struct class_for_type<extra_data_type::ragdoll_data> { using type = ragdoll_data; };
         template<> struct class_for_type<extra_data_type::random_teleport_marker> { using type = random_teleport_marker; };
         template<> struct class_for_type<extra_data_type::rank> { using type = rank; };
         template<> struct class_for_type<extra_data_type::reflector_refs> { using type = reflector_refs; };
         template<> struct class_for_type<extra_data_type::room_ref_data> { using type = room_ref_data; };
      #pragma endregion
      #pragma region S
         template<> struct class_for_type<extra_data_type::scale> { using type = scale; };
         template<> struct class_for_type<extra_data_type::spawn_container> { using type = spawn_container; };
      #pragma endregion
      #pragma region T
         template<> struct class_for_type<extra_data_type::teleport> { using type = teleport; };
         template<> struct class_for_type<extra_data_type::teleport_name> { using type = teleport_name; };
         template<> struct class_for_type<extra_data_type::time_left> { using type = time_left; };
      #pragma endregion
      #pragma region W
         template<> struct class_for_type<extra_data_type::water_current_zone_data> { using type = water_current_zone_data; };
         template<> struct class_for_type<extra_data_type::water_data> { using type = water_data; };
         template<> struct class_for_type<extra_data_type::water_environment_map> { using type = water_environment_map; };
      #pragma endregion
   #pragma endregion
         
   template<class C> struct type_for_class { static constexpr extra_data_type value = extra_data_type::invalid; };
   #pragma region type_for_class
      template<> struct type_for_class<unknown::XCZA> { static constexpr extra_data_type value = extra_data_type::unknown_xcza; };
      template<> struct type_for_class<unknown::XCZC> { static constexpr extra_data_type value = extra_data_type::unknown_xczc; };
      template<> struct type_for_class<unknown::XCZR> { static constexpr extra_data_type value = extra_data_type::unknown_xczr; };
      template<> struct type_for_class<unknown::XEDL> { static constexpr extra_data_type value = extra_data_type::unknown_xedl; };
      template<> struct type_for_class<unknown::XENC> { static constexpr extra_data_type value = extra_data_type::unknown_xenc; };
      template<> struct type_for_class<unknown::XLMB> { static constexpr extra_data_type value = extra_data_type::unknown_xlmb; };
      template<> struct type_for_class<unknown::XNVP> { static constexpr extra_data_type value = extra_data_type::unknown_xnvp; };
      template<> struct type_for_class<unknown::XPSL> { static constexpr extra_data_type value = extra_data_type::unknown_xpsl; };
      template<> struct type_for_class<unknown::XROO> { static constexpr extra_data_type value = extra_data_type::unknown_xroo; };
      template<> struct type_for_class<unknown::XUSE> { static constexpr extra_data_type value = extra_data_type::unknown_xuse; };
      template<> struct type_for_class<unknown::XWCS> { static constexpr extra_data_type value = extra_data_type::unknown_xwcs; };
      template<> struct type_for_class<unknown::XWLT> { static constexpr extra_data_type value = extra_data_type::unknown_xwlt; };
      template<> struct type_for_class<unknown::XWNT> { static constexpr extra_data_type value = extra_data_type::unknown_xwnt; };
      template<> struct type_for_class<deprecated::XCET> { static constexpr extra_data_type value = extra_data_type::deprecated_xcet; };
      template<> struct type_for_class<deprecated::XDCR> { static constexpr extra_data_type value = extra_data_type::deprecated_xdcr; };
      template<> struct type_for_class<deprecated::XHRS> { static constexpr extra_data_type value = extra_data_type::deprecated_xhrs; };
      template<> struct type_for_class<deprecated::XIBS> { static constexpr extra_data_type value = extra_data_type::deprecated_xibs; };
      template<> struct type_for_class<deprecated::XPCI> { static constexpr extra_data_type value = extra_data_type::deprecated_xpci; };
      template<> struct type_for_class<deprecated::XRAD> { static constexpr extra_data_type value = extra_data_type::deprecated_xrad; };
      template<> struct type_for_class<deprecated::XRDO> { static constexpr extra_data_type value = extra_data_type::deprecated_xrdo; };
      template<> struct type_for_class<deprecated::XSED> { static constexpr extra_data_type value = extra_data_type::deprecated_xsed; };
      template<> struct type_for_class<deprecated::XSOL> { static constexpr extra_data_type value = extra_data_type::deprecated_xsol; };
      #pragma region A
         template<> struct type_for_class<action> { static constexpr extra_data_type value = extra_data_type::action; };
         template<> struct type_for_class<activate_parent_data> { static constexpr extra_data_type value = extra_data_type::activate_parent_data; };
         template<> struct type_for_class<activate_ref> { static constexpr extra_data_type value = extra_data_type::activate_ref; };
         template<> struct type_for_class<alpha_cutoff> { static constexpr extra_data_type value = extra_data_type::alpha_cutoff; };
         template<> struct type_for_class<ammo> { static constexpr extra_data_type value = extra_data_type::ammo; };
         template<> struct type_for_class<attach_ref> { static constexpr extra_data_type value = extra_data_type::attach_ref; };
      #pragma endregion
      #pragma region C
         template<> struct type_for_class<cell_acoustic_space> { static constexpr extra_data_type value = extra_data_type::cell_acoustic_space; };
         template<> struct type_for_class<cell_climate> { static constexpr extra_data_type value = extra_data_type::cell_climate; };
         template<> struct type_for_class<cell_grass_data> { static constexpr extra_data_type value = extra_data_type::cell_grass_data; };
         template<> struct type_for_class<cell_imagespace> { static constexpr extra_data_type value = extra_data_type::cell_imagespace; };
         template<> struct type_for_class<cell_music_override> { static constexpr extra_data_type value = extra_data_type::cell_music_override; };
         template<> struct type_for_class<cell_region_list> { static constexpr extra_data_type value = extra_data_type::cell_region_list; };
         template<> struct type_for_class<cell_water_type> { static constexpr extra_data_type value = extra_data_type::cell_water_type; };
         template<> struct type_for_class<charge> { static constexpr extra_data_type value = extra_data_type::charge; };
         template<> struct type_for_class<collision_data> { static constexpr extra_data_type value = extra_data_type::collision_data; };
         template<> struct type_for_class<count> { static constexpr extra_data_type value = extra_data_type::count; };
      #pragma endregion
      template<> struct type_for_class<distant_data> { static constexpr extra_data_type value = extra_data_type::distant_data; };
      #pragma region E
         template<> struct type_for_class<emittance_source> { static constexpr extra_data_type value = extra_data_type::emittance_source; };
         template<> struct type_for_class<enable_state_parent> { static constexpr extra_data_type value = extra_data_type::enable_state_parent; };
         template<> struct type_for_class<encounter_zone> { static constexpr extra_data_type value = extra_data_type::encounter_zone; };
      #pragma endregion
      template<> struct type_for_class<favor_cost> { static constexpr extra_data_type value = extra_data_type::favor_cost; };
      template<> struct type_for_class<global> { static constexpr extra_data_type value = extra_data_type::global; };
      #pragma region H
         template<> struct type_for_class<headtracking_weight> { static constexpr extra_data_type value = extra_data_type::headtracking_weight; };
         template<> struct type_for_class<health> { static constexpr extra_data_type value = extra_data_type::health; };
         template<> struct type_for_class<health_percent> { static constexpr extra_data_type value = extra_data_type::health_percent; };
         template<> struct type_for_class<horse> { static constexpr extra_data_type value = extra_data_type::horse; };
      #pragma endregion
      #pragma region I
         template<> struct type_for_class<ignored_by_sandbox> { static constexpr extra_data_type value = extra_data_type::ignored_by_sandbox; };
         template<> struct type_for_class<interior_lock_list> { static constexpr extra_data_type value = extra_data_type::interior_lock_list; };
      #pragma endregion
      #pragma region L
         template<> struct type_for_class<leveled_creature_modifier> { static constexpr extra_data_type value = extra_data_type::leveled_creature_modifier; };
         template<> struct type_for_class<leveled_item_base> { static constexpr extra_data_type value = extra_data_type::leveled_item_base; };
         template<> struct type_for_class<light> { static constexpr extra_data_type value = extra_data_type::light; };
         template<> struct type_for_class<linked_ref> { static constexpr extra_data_type value = extra_data_type::linked_ref; };
         template<> struct type_for_class<linked_ref_color> { static constexpr extra_data_type value = extra_data_type::linked_ref_color; };
         template<> struct type_for_class<lit_water> { static constexpr extra_data_type value = extra_data_type::lit_water; };
         template<> struct type_for_class<location> { static constexpr extra_data_type value = extra_data_type::location; };
         template<> struct type_for_class<location_ref_type> { static constexpr extra_data_type value = extra_data_type::location_ref_type; };
         template<> struct type_for_class<lock> { static constexpr extra_data_type value = extra_data_type::lock; };
      #pragma endregion
      #pragma region M
         template<> struct type_for_class<map_marker> { static constexpr extra_data_type value = extra_data_type::map_marker; };
         template<> struct type_for_class<merchant_container> { static constexpr extra_data_type value = extra_data_type::merchant_container; };
         template<> struct type_for_class<multibound_bounds> { static constexpr extra_data_type value = extra_data_type::multibound_bounds; };
         template<> struct type_for_class<multibound_ref> { static constexpr extra_data_type value = extra_data_type::multibound_ref; };
      #pragma endregion
      template<> struct type_for_class<navmesh_door_portal> { static constexpr extra_data_type value = extra_data_type::navmesh_door_portal; };
      #pragma region O
         template<> struct type_for_class<occlusion_plane> { static constexpr extra_data_type value = extra_data_type::occlusion_plane; };
         template<> struct type_for_class<occlusion_plane_ref_data> { static constexpr extra_data_type value = extra_data_type::occlusion_plane_ref_data; };
         template<> struct type_for_class<ownership> { static constexpr extra_data_type value = extra_data_type::ownership; };
      #pragma endregion
      #pragma region P
         template<> struct type_for_class<package_start_location> { static constexpr extra_data_type value = extra_data_type::package_start_location; };
         template<> struct type_for_class<patrol_ref_data> { static constexpr extra_data_type value = extra_data_type::patrol_ref_data; };
         template<> struct type_for_class<poison> { static constexpr extra_data_type value = extra_data_type::poison; };
         template<> struct type_for_class<portal> { static constexpr extra_data_type value = extra_data_type::portal; };
         template<> struct type_for_class<portal_origin_and_destination> { static constexpr extra_data_type value = extra_data_type::portal_origin_and_destination; };
         template<> struct type_for_class<primitive> { static constexpr extra_data_type value = extra_data_type::primitive; };
      #pragma endregion
      #pragma region R
         template<> struct type_for_class<radius> { static constexpr extra_data_type value = extra_data_type::radius; };
         template<> struct type_for_class<ragdoll_data> { static constexpr extra_data_type value = extra_data_type::ragdoll_data; };
         template<> struct type_for_class<random_teleport_marker> { static constexpr extra_data_type value = extra_data_type::random_teleport_marker; };
         template<> struct type_for_class<rank> { static constexpr extra_data_type value = extra_data_type::rank; };
         template<> struct type_for_class<reflector_refs> { static constexpr extra_data_type value = extra_data_type::reflector_refs; };
         template<> struct type_for_class<room_ref_data> { static constexpr extra_data_type value = extra_data_type::room_ref_data; };
      #pragma endregion
      #pragma region S
         template<> struct type_for_class<scale> { static constexpr extra_data_type value = extra_data_type::scale; };
         template<> struct type_for_class<spawn_container> { static constexpr extra_data_type value = extra_data_type::spawn_container; };
      #pragma endregion
      #pragma region T
         template<> struct type_for_class<teleport> { static constexpr extra_data_type value = extra_data_type::teleport; };
         template<> struct type_for_class<teleport_name> { static constexpr extra_data_type value = extra_data_type::teleport_name; };
         template<> struct type_for_class<time_left> { static constexpr extra_data_type value = extra_data_type::time_left; };
      #pragma endregion
      #pragma region W
         template<> struct type_for_class<water_current_zone_data> { static constexpr extra_data_type value = extra_data_type::water_current_zone_data; };
         template<> struct type_for_class<water_data> { static constexpr extra_data_type value = extra_data_type::water_data; };
         template<> struct type_for_class<water_environment_map> { static constexpr extra_data_type value = extra_data_type::water_environment_map; };
      #pragma endregion
   #pragma endregion

   #pragma region scalar_use_info_offset_for_class
   //
   // Usage:
   // static constexpr size_t b = scalar_use_info_offset_for_class<water_environment_map>::value;
   //
   template<int t> struct _scalar_use_info_offset_for {
      static constexpr size_t value = class_for_type<(extra_data_type)t>::type::scalar_use_count + _scalar_use_info_offset_for<t - 1>::value;
   };
   template<> struct _scalar_use_info_offset_for<-1> { // stopping point for recursion-based loop template
      static constexpr size_t value = 0;
   };
   template<class C> struct scalar_use_info_offset_for_class : public _scalar_use_info_offset_for<(int)type_for_class<C>::value - 1> {};
   #pragma endregion

   static constexpr size_t scalar_use_count_for_all_types = _scalar_use_info_offset_for<num_extra_data_types>::value;
   static_assert(extra_data_use_info_state::count == scalar_use_count_for_all_types, "Ensure that the use-info state object has enough room for all scalar uses.");

   //
   // TODO:
   //
   //  - we divide extra data use info into "scalar" uses and "list" uses. a scalar use is something 
   //    that the instance only has one of (e.g. XPSN), such that if it appears multiple times in a 
   //    record, the previously-loaded values are overridden. a list use is anything you can have 
   //    several of; currently, extra-data loaders always append and never reset when encountering 
   //    these, so multiple appearances just get concatenated.
   //
   //  - the form use info generator function should create an instance of extra_data_use_info_state to 
   //    hold "scalar" uses
   //
   //  - extra data instances should be passed the extra_data_use_info_state instance and the use info 
   //    builder object. scalar uses go in the former and list uses are directly added to the latter.
   //
   //     - but where in the former?
   //
   //        - use (scalar_use_info_offset_for_class<*this>) to get the start index
   //
   //        - each class has a constexpr value (scalar_use_count) defined (except for the classes that 
   //          have no use info, but perhaps we should explicitly set it to 0 on them, too) so that's 
   //          basically how many slots are alotted to the class
   //
   // A POSSIBLE ALTERNATIVE APPROACH:
   //
   //  - define a struct like this:
   //
   //    struct extra_data_use_info_state {
   //       union {
   //          struct {
   //             struct linked_ref {
   //                form_id_t keyword;
   //                form_id_t ref;
   //             };
   //             struct poison {
   //                form_id_t type;
   //             };
   //          } ids;
   //          std::array<form_id_t, 3> list;
   //       };
   //       static_assert(sizeof(list) == sizeof(ids));
   //    };
   //
   //  - then, have that be the state that's passed down
   //
   //  - avoids the need for a ton of literally-impossible-to-debug metaprogramming garbage
   //
   //     - hey, did you know that we were missing a deprecated::XCET class until we started this? 
   //       neither did i! i only found out because it was breaking the above templates, and it took 
   //       me something like four hours TO find out! C++ templates are an awful metaprogramming 
   //       mechanism! :D :D :D
   //
   //  - it means we have to pre-reserve slots for all extra-data types, but we kinda have to do 
   //    that anyway, don't we?
   //
}