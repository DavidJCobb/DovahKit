#include "./construct_for_subrecord.h"
#include "../types/all.h"
#include "../types/class_array.h"

namespace dovah::loaded_forms::components::extra_data_factories {
   namespace {
      template<typename T>
      extra_data_types::extra_data* _construct() {
         return new T;
      };

      struct _entry {
         uint32_t signature;
         extra_data_types::extra_data*(*construct)();
      };
      
      constexpr const auto _factories = std::array{
         _entry{ extra_data_types::deprecated::XEDL::signature, _construct<extra_data_types::deprecated::XEDL> },
         _entry{ extra_data_types::deprecated::XENC::signature, _construct<extra_data_types::deprecated::XENC> },
         _entry{ extra_data_types::deprecated::XLMB::signature, _construct<extra_data_types::deprecated::XLMB> },
         _entry{ extra_data_types::deprecated::XNVP::signature, _construct<extra_data_types::deprecated::XNVP> },
         _entry{ extra_data_types::deprecated::XROO::signature, _construct<extra_data_types::deprecated::XROO> },
         _entry{ extra_data_types::deprecated::XUSE::signature, _construct<extra_data_types::deprecated::XUSE> },
         _entry{ extra_data_types::deprecated::XWCS::signature, _construct<extra_data_types::deprecated::XWCS> },
         _entry{ extra_data_types::deprecated::XWLT::signature, _construct<extra_data_types::deprecated::XWLT> },
         _entry{ extra_data_types::deprecated::XWNT::signature, _construct<extra_data_types::deprecated::XWNT> },
         _entry{ extra_data_types::deprecated::XDCR::signature, _construct<extra_data_types::deprecated::XDCR> },
         _entry{ extra_data_types::deprecated::XHRS::signature, _construct<extra_data_types::deprecated::XHRS> },
         _entry{ extra_data_types::deprecated::XIBS::signature, _construct<extra_data_types::deprecated::XIBS> },
         _entry{ extra_data_types::deprecated::XPCI::signature, _construct<extra_data_types::deprecated::XPCI> },
         _entry{ extra_data_types::deprecated::XRAD::signature, _construct<extra_data_types::deprecated::XRAD> },
         _entry{ extra_data_types::deprecated::XRDO::signature, _construct<extra_data_types::deprecated::XRDO> },
         _entry{ extra_data_types::deprecated::XSED::signature, _construct<extra_data_types::deprecated::XSED> },
         _entry{ extra_data_types::deprecated::XSOL::signature, _construct<extra_data_types::deprecated::XSOL> },
         #pragma region A
            _entry{ extra_data_types::action::signature,                         _construct<extra_data_types::action> },
            _entry{ extra_data_types::activate_parents::signature_flags,         _construct<extra_data_types::activate_parents> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::activate_parents::signature_parent,        _construct<extra_data_types::activate_parents> }, // 
            _entry{ extra_data_types::activate_parents::signature_parent_legacy, _construct<extra_data_types::activate_parents> },
            _entry{ extra_data_types::alpha_cutoff::signature,                   _construct<extra_data_types::alpha_cutoff> },
            _entry{ extra_data_types::ammo::signature_type,                      _construct<extra_data_types::ammo> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::ammo::signature_count,                     _construct<extra_data_types::ammo> }, // 
            _entry{ extra_data_types::attach_ref::signature,                     _construct<extra_data_types::attach_ref> },
         #pragma endregion
         #pragma region C
            _entry{ extra_data_types::cell_acoustic_space::signature, _construct<extra_data_types::cell_acoustic_space> },
            _entry{ extra_data_types::cell_climate::signature,        _construct<extra_data_types::cell_climate> },
            _entry{ extra_data_types::cell_grass_data::signature,     _construct<extra_data_types::cell_grass_data> },
            _entry{ extra_data_types::cell_imagespace::signature,     _construct<extra_data_types::cell_imagespace> },
            _entry{ extra_data_types::cell_music_override::signature, _construct<extra_data_types::cell_music_override> },
            _entry{ extra_data_types::cell_region_list::signature,    _construct<extra_data_types::cell_region_list> },
            _entry{ extra_data_types::cell_water_type::signature,     _construct<extra_data_types::cell_water_type> },
            _entry{ extra_data_types::charge::signature,              _construct<extra_data_types::charge> },
            _entry{ extra_data_types::collision_data::signature,      _construct<extra_data_types::collision_data> },
            _entry{ extra_data_types::count::signature,               _construct<extra_data_types::count> },
         #pragma endregion
         _entry{ extra_data_types::distant_data::signature, _construct<extra_data_types::distant_data> },
         #pragma region E
            _entry{ extra_data_types::emittance_source::signature,    _construct<extra_data_types::emittance_source> },
            _entry{ extra_data_types::enable_state_parent::signature, _construct<extra_data_types::enable_state_parent> },
            _entry{ extra_data_types::encounter_zone::signature,      _construct<extra_data_types::encounter_zone> },
         #pragma endregion
         _entry{ extra_data_types::favor_cost::signature, _construct<extra_data_types::favor_cost> },
         _entry{ extra_data_types::global::signature,     _construct<extra_data_types::global> },
         #pragma region H
            _entry{ extra_data_types::headtracking_weight::signature, _construct<extra_data_types::headtracking_weight> },
            _entry{ extra_data_types::health::signature,              _construct<extra_data_types::health> },
            _entry{ extra_data_types::health_percent::signature,      _construct<extra_data_types::health_percent> },
            _entry{ extra_data_types::horse::signature,               _construct<extra_data_types::horse> },
         #pragma endregion
         #pragma region I
            _entry{ extra_data_types::ignored_by_sandbox::signature, _construct<extra_data_types::ignored_by_sandbox> },
            _entry{ extra_data_types::interior_lock_list::signature, _construct<extra_data_types::interior_lock_list> },
         #pragma endregion
         #pragma region L
            _entry{ extra_data_types::leveled_creature_modifier::signature, _construct<extra_data_types::leveled_creature_modifier> },
            _entry{ extra_data_types::leveled_item_base::signature,         _construct<extra_data_types::leveled_item_base> },
            _entry{ extra_data_types::light::signature,                     _construct<extra_data_types::light> },
            _entry{ extra_data_types::linked_ref::signature,                _construct<extra_data_types::linked_ref> },
            _entry{ extra_data_types::linked_ref_color::signature,          _construct<extra_data_types::linked_ref_color> },
            _entry{ extra_data_types::lit_water::signature,                 _construct<extra_data_types::lit_water> },
            _entry{ extra_data_types::location::signature,                  _construct<extra_data_types::location> },
            _entry{ extra_data_types::location_ref_type::signature,         _construct<extra_data_types::location_ref_type> },
            _entry{ extra_data_types::lock::signature,                      _construct<extra_data_types::lock> },
         #pragma endregion
         #pragma region M
            _entry{ extra_data_types::map_marker::signature,         _construct<extra_data_types::map_marker> },
            _entry{ extra_data_types::merchant_container::signature, _construct<extra_data_types::merchant_container> },
            _entry{ extra_data_types::multibound_bounds::signature,  _construct<extra_data_types::multibound_bounds> },
            _entry{ extra_data_types::multibound_ref::signature,     _construct<extra_data_types::multibound_ref> },
         #pragma endregion
         _entry{ extra_data_types::navmesh_door_portal::signature, _construct<extra_data_types::navmesh_door_portal> },
         #pragma region O
            _entry{ extra_data_types::occlusion_plane::signature,          _construct<extra_data_types::occlusion_plane> },
            _entry{ extra_data_types::occlusion_plane_ref_data::signature, _construct<extra_data_types::occlusion_plane_ref_data> },
            _entry{ extra_data_types::ownership::signature,                _construct<extra_data_types::ownership> },
         #pragma endregion
         #pragma region P
            _entry{ extra_data_types::package_start_location::signature,        _construct<extra_data_types::package_start_location> },
            _entry{ extra_data_types::patrol_ref_data::signature_time,          _construct<extra_data_types::patrol_ref_data> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::patrol_ref_data::signature_event,         _construct<extra_data_types::patrol_ref_data> }, // 
            _entry{ extra_data_types::poison::signature_type,                   _construct<extra_data_types::poison> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::poison::signature_dose,                   _construct<extra_data_types::poison> }, // 
            _entry{ extra_data_types::portal::signature,                        _construct<extra_data_types::portal> },
            _entry{ extra_data_types::portal_origin_and_destination::signature, _construct<extra_data_types::portal_origin_and_destination> },
            _entry{ extra_data_types::primitive::signature,                     _construct<extra_data_types::primitive> },
         #pragma endregion
         #pragma region R
            _entry{ extra_data_types::radius::signature,                 _construct<extra_data_types::radius> },
            _entry{ extra_data_types::ragdoll_data::signature_base,      _construct<extra_data_types::ragdoll_data> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::ragdoll_data::signature_biped,     _construct<extra_data_types::ragdoll_data> }, // 
            _entry{ extra_data_types::random_teleport_marker::signature, _construct<extra_data_types::random_teleport_marker> },
            _entry{ extra_data_types::rank::signature,                   _construct<extra_data_types::rank> },
            _entry{ extra_data_types::reflector_refs::signature,         _construct<extra_data_types::reflector_refs> },
            _entry{ extra_data_types::room_ref_data::signature,          _construct<extra_data_types::room_ref_data> },
         #pragma endregion
         #pragma region S
            _entry{ extra_data_types::scale::signature,           _construct<extra_data_types::scale> },
            _entry{ extra_data_types::spawn_container::signature, _construct<extra_data_types::spawn_container> },
         #pragma endregion
         #pragma region T
            _entry{ extra_data_types::teleport::signature,      _construct<extra_data_types::teleport> },
            _entry{ extra_data_types::teleport_name::signature, _construct<extra_data_types::teleport_name> },
            _entry{ extra_data_types::time_left::signature,     _construct<extra_data_types::time_left> },
         #pragma endregion
         #pragma region W
            _entry{ extra_data_types::water_current_zone_data::signature_vel_linear,     _construct<extra_data_types::water_current_zone_data> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::water_current_zone_data::signature_vel_rotational, _construct<extra_data_types::water_current_zone_data> }, // 
            _entry{ extra_data_types::water_current_zone_data::signature_zone_cell,      _construct<extra_data_types::water_current_zone_data> }, // 
            _entry{ extra_data_types::water_current_zone_data::signature_zone_ref,       _construct<extra_data_types::water_current_zone_data> }, // 
            _entry{ extra_data_types::water_current_zone_data::signature_zone_action,    _construct<extra_data_types::water_current_zone_data> }, // 
            _entry{ extra_data_types::water_data::signature_base_legacy,                 _construct<extra_data_types::water_data> }, // This extra-data type has multiple signatures...
            _entry{ extra_data_types::water_data::signature_base_modern,                 _construct<extra_data_types::water_data> },
            _entry{ extra_data_types::water_data::signature_vel,                         _construct<extra_data_types::water_data> }, // 
            _entry{ extra_data_types::water_environment_map::signature,                  _construct<extra_data_types::water_environment_map> },
         #pragma endregion
      };
   }

   extern extra_data_types::extra_data* construct_for_subrecord(uint32_t signature) {
      for (auto& f : _factories)
         if (f.signature == signature)
            return (f.construct)();
      return nullptr;
   }
}