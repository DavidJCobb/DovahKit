#include "_factory.h"
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

namespace {
   using namespace dovah::loaded_forms::components;
   using namespace dovah::loaded_forms::components::extra;
   //
   template<class edc> basic_extra_data* _create() { return new edc; }
   using _factory_t = basic_extra_data*(*)();

   struct _entry {
      uint32_t   signature;
      _factory_t function;
   };

   _entry _factories[] = {
      { unknown::XCZA::signature, _create<unknown::XCZA> },
      { unknown::XCZC::signature, _create<unknown::XCZC> },
      { unknown::XCZR::signature, _create<unknown::XCZR> },
      { unknown::XEDL::signature, _create<unknown::XEDL> },
      { unknown::XENC::signature, _create<unknown::XENC> },
      { unknown::XLMB::signature, _create<unknown::XLMB> },
      { unknown::XNVP::signature, _create<unknown::XNVP> },
      { unknown::XPSL::signature, _create<unknown::XPSL> },
      { unknown::XROO::signature, _create<unknown::XROO> },
      { unknown::XUSE::signature, _create<unknown::XUSE> },
      { unknown::XWLT::signature, _create<unknown::XWLT> },
      { unknown::XWNT::signature, _create<unknown::XWNT> },
      { deprecated::XDCR::signature, _create<deprecated::XDCR> },
      { deprecated::XHRS::signature, _create<deprecated::XHRS> },
      { deprecated::XIBS::signature, _create<deprecated::XIBS> },
      { deprecated::XPCI::signature, _create<deprecated::XPCI> },
      { deprecated::XRAD::signature, _create<deprecated::XRAD> },
      { deprecated::XRDO::signature, _create<deprecated::XRDO> },
      { deprecated::XSED::signature, _create<deprecated::XSED> },
      { deprecated::XSOL::signature, _create<deprecated::XSOL> },
      #pragma region A
         { action::signature,                      _create<action> },
         { activate_parent_data::signature_flags,  _create<activate_parent_data> }, // This extra-data type has multiple signatures...
         { activate_parent_data::signature_parent, _create<activate_parent_data> }, // 
         { activate_ref::signature,                _create<activate_ref> },
         { alpha_cutoff::signature,                _create<alpha_cutoff> },
         { ammo::signature_type,                   _create<ammo> }, // This extra-data type has multiple signatures...
         { ammo::signature_count,                  _create<ammo> }, // 
         { attach_ref::signature,                  _create<attach_ref> },
      #pragma endregion
      #pragma region C
         { cell_acoustic_space::signature, _create<cell_acoustic_space> },
         { cell_climate::signature,        _create<cell_climate> },
         { cell_grass_data::signature,     _create<cell_grass_data> },
         { cell_imagespace::signature,     _create<cell_imagespace> },
         { cell_music_override::signature, _create<cell_music_override> },
         { cell_region_list::signature,    _create<cell_region_list> },
         { cell_water_type::signature,     _create<cell_water_type> },
         { charge::signature,              _create<charge> },
         { collision_data::signature,      _create<collision_data> },
         { count::signature,               _create<count> },
      #pragma endregion
      { distant_data::signature, _create<distant_data> },
      #pragma region E
         { emittance_source::signature,    _create<emittance_source> },
         { enable_state_parent::signature, _create<enable_state_parent> },
         { encounter_zone::signature,      _create<encounter_zone> },
      #pragma endregion
      { favor_cost::signature, _create<favor_cost> },
      { global::signature,     _create<global> },
      #pragma region H
         { headtracking_weight::signature, _create<headtracking_weight> },
         { health::signature,              _create<health> },
         { health_percent::signature,      _create<health_percent> },
         { horse::signature,               _create<horse> },
      #pragma endregion
      #pragma region I
         { ignored_by_sandbox::signature, _create<ignored_by_sandbox> },
         { interior_lock_list::signature, _create<interior_lock_list> },
      #pragma endregion
      #pragma region L
         { leveled_creature_modifier::signature, _create<leveled_creature_modifier> },
         { leveled_item_base::signature,         _create<leveled_item_base> },
         { light::signature,                     _create<light> },
         { linked_ref::signature,                _create<linked_ref> },
         { linked_ref_color::signature,          _create<linked_ref_color> },
         { lit_water::signature,                 _create<lit_water> },
         { location::signature,                  _create<location> },
         { location_ref_type::signature,         _create<location_ref_type> },
         { lock::signature,                      _create<lock> },
      #pragma endregion
      #pragma region M
         { map_marker::signature,         _create<map_marker> },
         { merchant_container::signature, _create<merchant_container> },
         { multibound_bounds::signature,  _create<multibound_bounds> },
         { multibound_ref::signature,     _create<multibound_ref> },
      #pragma endregion
      { navmesh_door_portal::signature, _create<navmesh_door_portal> },
      #pragma region O
         { occlusion_plane::signature,          _create<occlusion_plane> },
         { occlusion_plane_ref_data::signature, _create<occlusion_plane_ref_data> },
         { ownership::signature,                _create<ownership> },
      #pragma endregion
      #pragma region P
         { package_start_location::signature,        _create<package_start_location> },
         { patrol_ref_data::signature_time,          _create<patrol_ref_data> }, // This extra-data type has multiple signatures...
         { patrol_ref_data::signature_event,         _create<patrol_ref_data> }, // 
         { poison::signature_type,                   _create<poison> }, // This extra-data type has multiple signatures...
         { poison::signature_dose,                   _create<poison> }, // 
         { portal::signature,                        _create<portal> },
         { portal_origin_and_destination::signature, _create<portal_origin_and_destination> },
         { primitive::signature,                     _create<primitive> },
      #pragma endregion
      #pragma region R
         { radius::signature,                 _create<radius> },
         { ragdoll_data::signature_base,      _create<ragdoll_data> }, // This extra-data type has multiple signatures...
         { ragdoll_data::signature_biped,     _create<ragdoll_data> }, // 
         { random_teleport_marker::signature, _create<random_teleport_marker> },
         { rank::signature,                   _create<rank> },
         { reflector_refs::signature,         _create<reflector_refs> },
         { room_ref_data::signature,          _create<room_ref_data> },
      #pragma endregion
      #pragma region S
         { scale::signature,           _create<scale> },
         { spawn_container::signature, _create<spawn_container> },
      #pragma endregion
      #pragma region T
         { teleport::signature,      _create<teleport> },
         { teleport_name::signature, _create<teleport_name> },
         { time_left::signature,     _create<time_left> },
      #pragma endregion
      #pragma region W
         { water_current_zone_data::signature_vel_linear,     _create<water_current_zone_data> }, // This extra-data type has multiple signatures...
         { water_current_zone_data::signature_vel_rotational, _create<water_current_zone_data> }, // 
         { water_data::signature_base,                        _create<water_data> }, // XWCN
         { water_environment_map::signature,                  _create<water_environment_map> },
      #pragma endregion
   };
}

namespace dovah::loaded_forms::components {
   basic_extra_data* create_extra_data_for_subrecord(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      for (int i = 0; i < std::extent<decltype(_factories)>::value; ++i) {
         auto& entry = _factories[i];
         if (signature == entry.signature)
            return (entry.function)();
      }
      return nullptr;
   }
}