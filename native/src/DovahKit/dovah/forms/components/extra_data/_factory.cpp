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
   using form_stub            = dovah::form_stub;
   using tes_record_reader    = dovah::tes_file_reading::record;
   using tes_subrecord_reader = dovah::tes_file_reading::subrecord;
   //
   template<class edc> basic_extra_data* _create() { return new edc; }
   using _factory_t = basic_extra_data*(*)();
   //
   template<class edc> void _use_info(tes_record_reader& record, form_stub* stub) { edc::generate_use_info(record, stub); };
   using _use_info_t = void(*)(tes_record_reader&, form_stub*);

   struct _handlers {
      _factory_t  construct;
      _use_info_t use_info;
      //
      template<class edc> static _handlers make() {
         _handlers instance;
         instance.construct = _create<edc>;
         instance.use_info  = _use_info<edc>;
         return instance;
      }
   };

   struct _entry {
      uint32_t  signature;
      _handlers handlers;
   };

   _entry _factories[] = {
      { unknown::XCZA::signature, _handlers::make<unknown::XCZA>() },
      { unknown::XCZC::signature, _handlers::make<unknown::XCZC>() },
      { unknown::XCZR::signature, _handlers::make<unknown::XCZR>() },
      { unknown::XEDL::signature, _handlers::make<unknown::XEDL>() },
      { unknown::XENC::signature, _handlers::make<unknown::XENC>() },
      { unknown::XLMB::signature, _handlers::make<unknown::XLMB>() },
      { unknown::XNVP::signature, _handlers::make<unknown::XNVP>() },
      { unknown::XPSL::signature, _handlers::make<unknown::XPSL>() },
      { unknown::XROO::signature, _handlers::make<unknown::XROO>() },
      { unknown::XUSE::signature, _handlers::make<unknown::XUSE>() },
      { unknown::XWCS::signature, _handlers::make<unknown::XWCS>() },
      { unknown::XWLT::signature, _handlers::make<unknown::XWLT>() },
      { unknown::XWNT::signature, _handlers::make<unknown::XWNT>() },
      { deprecated::XDCR::signature, _handlers::make<deprecated::XDCR>() },
      { deprecated::XHRS::signature, _handlers::make<deprecated::XHRS>() },
      { deprecated::XIBS::signature, _handlers::make<deprecated::XIBS>() },
      { deprecated::XPCI::signature, _handlers::make<deprecated::XPCI>() },
      { deprecated::XRAD::signature, _handlers::make<deprecated::XRAD>() },
      { deprecated::XRDO::signature, _handlers::make<deprecated::XRDO>() },
      { deprecated::XSED::signature, _handlers::make<deprecated::XSED>() },
      { deprecated::XSOL::signature, _handlers::make<deprecated::XSOL>() },
      #pragma region A
         { action::signature,                      _handlers::make<action>() },
         { activate_parent_data::signature_flags,  _handlers::make<activate_parent_data>() }, // This extra-data type has multiple signatures...
         { activate_parent_data::signature_parent, _handlers::make<activate_parent_data>() }, // 
         { activate_ref::signature,                _handlers::make<activate_ref>() },
         { alpha_cutoff::signature,                _handlers::make<alpha_cutoff>() },
         { ammo::signature_type,                   _handlers::make<ammo>() }, // This extra-data type has multiple signatures...
         { ammo::signature_count,                  _handlers::make<ammo>() }, // 
         { attach_ref::signature,                  _handlers::make<attach_ref>() },
      #pragma endregion
      #pragma region C
         { cell_acoustic_space::signature, _handlers::make<cell_acoustic_space>() },
         { cell_climate::signature,        _handlers::make<cell_climate>() },
         { cell_grass_data::signature,     _handlers::make<cell_grass_data>() },
         { cell_imagespace::signature,     _handlers::make<cell_imagespace>() },
         { cell_music_override::signature, _handlers::make<cell_music_override>() },
         { cell_region_list::signature,    _handlers::make<cell_region_list>() },
         { cell_water_type::signature,     _handlers::make<cell_water_type>() },
         { charge::signature,              _handlers::make<charge>() },
         { collision_data::signature,      _handlers::make<collision_data>() },
         { count::signature,               _handlers::make<count>() },
      #pragma endregion
      { distant_data::signature, _handlers::make<distant_data>() },
      #pragma region E
         { emittance_source::signature,    _handlers::make<emittance_source>() },
         { enable_state_parent::signature, _handlers::make<enable_state_parent>() },
         { encounter_zone::signature,      _handlers::make<encounter_zone>() },
      #pragma endregion
      { favor_cost::signature, _handlers::make<favor_cost>() },
      { global::signature,     _handlers::make<global>() },
      #pragma region H
         { headtracking_weight::signature, _handlers::make<headtracking_weight>() },
         { health::signature,              _handlers::make<health>() },
         { health_percent::signature,      _handlers::make<health_percent>() },
         { horse::signature,               _handlers::make<horse>() },
      #pragma endregion
      #pragma region I
         { ignored_by_sandbox::signature, _handlers::make<ignored_by_sandbox>() },
         { interior_lock_list::signature, _handlers::make<interior_lock_list>() },
      #pragma endregion
      #pragma region L
         { leveled_creature_modifier::signature, _handlers::make<leveled_creature_modifier>() },
         { leveled_item_base::signature,         _handlers::make<leveled_item_base>() },
         { light::signature,                     _handlers::make<light>() },
         { linked_ref::signature,                _handlers::make<linked_ref>() },
         { linked_ref_color::signature,          _handlers::make<linked_ref_color>() },
         { lit_water::signature,                 _handlers::make<lit_water>() },
         { location::signature,                  _handlers::make<location>() },
         { location_ref_type::signature,         _handlers::make<location_ref_type>() },
         { lock::signature,                      _handlers::make<lock>() },
      #pragma endregion
      #pragma region M
         { map_marker::signature,         _handlers::make<map_marker>() },
         { merchant_container::signature, _handlers::make<merchant_container>() },
         { multibound_bounds::signature,  _handlers::make<multibound_bounds>() },
         { multibound_ref::signature,     _handlers::make<multibound_ref>() },
      #pragma endregion
      { navmesh_door_portal::signature, _handlers::make<navmesh_door_portal>() },
      #pragma region O
         { occlusion_plane::signature,          _handlers::make<occlusion_plane>() },
         { occlusion_plane_ref_data::signature, _handlers::make<occlusion_plane_ref_data>() },
         { ownership::signature,                _handlers::make<ownership>() },
      #pragma endregion
      #pragma region P
         { package_start_location::signature,        _handlers::make<package_start_location>() },
         { patrol_ref_data::signature_time,          _handlers::make<patrol_ref_data>() }, // This extra-data type has multiple signatures...
         { patrol_ref_data::signature_event,         _handlers::make<patrol_ref_data>() }, // 
         { poison::signature_type,                   _handlers::make<poison>() }, // This extra-data type has multiple signatures...
         { poison::signature_dose,                   _handlers::make<poison>() }, // 
         { portal::signature,                        _handlers::make<portal>() },
         { portal_origin_and_destination::signature, _handlers::make<portal_origin_and_destination>() },
         { primitive::signature,                     _handlers::make<primitive>() },
      #pragma endregion
      #pragma region R
         { radius::signature,                 _handlers::make<radius>() },
         { ragdoll_data::signature_base,      _handlers::make<ragdoll_data>() }, // This extra-data type has multiple signatures...
         { ragdoll_data::signature_biped,     _handlers::make<ragdoll_data>() }, // 
         { random_teleport_marker::signature, _handlers::make<random_teleport_marker>() },
         { rank::signature,                   _handlers::make<rank>() },
         { reflector_refs::signature,         _handlers::make<reflector_refs>() },
         { room_ref_data::signature,          _handlers::make<room_ref_data>() },
      #pragma endregion
      #pragma region S
         { scale::signature,           _handlers::make<scale>() },
         { spawn_container::signature, _handlers::make<spawn_container>() },
      #pragma endregion
      #pragma region T
         { teleport::signature,      _handlers::make<teleport>() },
         { teleport_name::signature, _handlers::make<teleport_name>() },
         { time_left::signature,     _handlers::make<time_left>() },
      #pragma endregion
      #pragma region W
         { water_current_zone_data::signature_vel_linear,     _handlers::make<water_current_zone_data>() }, // This extra-data type has multiple signatures...
         { water_current_zone_data::signature_vel_rotational, _handlers::make<water_current_zone_data>() }, // 
         { water_data::signature_base,                        _handlers::make<water_data>() }, // This extra-data type has multiple signatures...
         { water_data::signature_vel,                         _handlers::make<water_data>() }, // 
         { water_environment_map::signature,                  _handlers::make<water_environment_map>() },
      #pragma endregion
   };
}

namespace dovah::loaded_forms::components {
   basic_extra_data* create_extra_data_for_subrecord(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      for (int i = 0; i < std::extent<decltype(_factories)>::value; ++i) {
         auto& entry = _factories[i];
         if (signature == entry.signature)
            return (entry.handlers.construct)();
      }
      return nullptr;
   }
   extra_data_load_result generate_extra_data_use_info(tes_record_reader& record, form_stub* stub) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord && "You need to open a subrecord before calling this.");
      auto  signature = subrecord.signature();
      for (int i = 0; i < std::extent<decltype(_factories)>::value; ++i) {
         auto& entry = _factories[i];
         if (signature == entry.signature) {
            (entry.handlers.use_info)(record, stub);
            return extra_data_load_result::succeeded;
         }
      }
      return extra_data_load_result::unrecognized;
   }
}