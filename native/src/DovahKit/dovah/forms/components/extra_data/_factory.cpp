#include "_factory.h"
#include <array>
#include "../../../../helpers/static_string.h"

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

namespace dovah {
   struct extra_data_use_info_state;
}
namespace {
   using namespace dovah::loaded_forms::components;
   using namespace dovah::loaded_forms::components::extra;
   using tes_record_reader    = dovah::tes_file_reading::record;
   using tes_subrecord_reader = dovah::tes_file_reading::subrecord;
   
   template<class edc> basic_extra_data* _create() { return new edc; }
   using _factory_t = basic_extra_data*(*)();
   
   template<class edc> void _use_info(tes_record_reader& record, dovah::form_stub_use_info_builder& uib, dovah::extra_data_use_info_state& state) { edc::generate_use_info(record, uib, state); };
   using _use_info_t = void(*)(tes_record_reader&, dovah::form_stub_use_info_builder&, dovah::extra_data_use_info_state&);

   struct _handlers {
      _factory_t  construct = nullptr;
      _use_info_t use_info  = nullptr;
      
      template<class edc> static constexpr _handlers make() {
         _handlers instance;
         instance.construct = _create<edc>;
         instance.use_info  = _use_info<edc>;
         return instance;
      }
   };

   struct _entry {
      extra_data_type type;
      uint32_t  signature;
      _handlers handlers;
   };

   constexpr const auto _factories = std::array{
      _entry{ extra_data_type::unknown_xcza, unknown::XCZA::signature, _handlers::make<unknown::XCZA>() },
      _entry{ extra_data_type::unknown_xczc, unknown::XCZC::signature, _handlers::make<unknown::XCZC>() },
      _entry{ extra_data_type::unknown_xczr, unknown::XCZR::signature, _handlers::make<unknown::XCZR>() },
      _entry{ extra_data_type::unknown_xedl, unknown::XEDL::signature, _handlers::make<unknown::XEDL>() },
      _entry{ extra_data_type::unknown_xenc, unknown::XENC::signature, _handlers::make<unknown::XENC>() },
      _entry{ extra_data_type::unknown_xlmb, unknown::XLMB::signature, _handlers::make<unknown::XLMB>() },
      _entry{ extra_data_type::unknown_xnvp, unknown::XNVP::signature, _handlers::make<unknown::XNVP>() },
      _entry{ extra_data_type::unknown_xroo, unknown::XROO::signature, _handlers::make<unknown::XROO>() },
      _entry{ extra_data_type::unknown_xuse, unknown::XUSE::signature, _handlers::make<unknown::XUSE>() },
      _entry{ extra_data_type::unknown_xwcs, unknown::XWCS::signature, _handlers::make<unknown::XWCS>() },
      _entry{ extra_data_type::unknown_xwlt, unknown::XWLT::signature, _handlers::make<unknown::XWLT>() },
      _entry{ extra_data_type::unknown_xwnt, unknown::XWNT::signature, _handlers::make<unknown::XWNT>() },
      _entry{ extra_data_type::deprecated_xdcr, deprecated::XDCR::signature, _handlers::make<deprecated::XDCR>() },
      _entry{ extra_data_type::deprecated_xhrs, deprecated::XHRS::signature, _handlers::make<deprecated::XHRS>() },
      _entry{ extra_data_type::deprecated_xibs, deprecated::XIBS::signature, _handlers::make<deprecated::XIBS>() },
      _entry{ extra_data_type::deprecated_xpci, deprecated::XPCI::signature, _handlers::make<deprecated::XPCI>() },
      _entry{ extra_data_type::deprecated_xrad, deprecated::XRAD::signature, _handlers::make<deprecated::XRAD>() },
      _entry{ extra_data_type::deprecated_xrdo, deprecated::XRDO::signature, _handlers::make<deprecated::XRDO>() },
      _entry{ extra_data_type::deprecated_xsed, deprecated::XSED::signature, _handlers::make<deprecated::XSED>() },
      _entry{ extra_data_type::deprecated_xsol, deprecated::XSOL::signature, _handlers::make<deprecated::XSOL>() },
      #pragma region A
         _entry{ extra_data_type::action,               action::signature,                      _handlers::make<action>() },
         _entry{ extra_data_type::activate_parent_data, activate_parent_data::signature_flags,  _handlers::make<activate_parent_data>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::activate_parent_data, activate_parent_data::signature_parent, _handlers::make<activate_parent_data>() }, // 
         _entry{ extra_data_type::activate_ref,         activate_ref::signature,                _handlers::make<activate_ref>() },
         _entry{ extra_data_type::alpha_cutoff,         alpha_cutoff::signature,                _handlers::make<alpha_cutoff>() },
         _entry{ extra_data_type::ammo,                 ammo::signature_type,                   _handlers::make<ammo>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::ammo,                 ammo::signature_count,                  _handlers::make<ammo>() }, // 
         _entry{ extra_data_type::attach_ref,           attach_ref::signature,                  _handlers::make<attach_ref>() },
      #pragma endregion
      #pragma region C
         _entry{ extra_data_type::cell_acoustic_space, cell_acoustic_space::signature, _handlers::make<cell_acoustic_space>() },
         _entry{ extra_data_type::cell_climate,        cell_climate::signature,        _handlers::make<cell_climate>() },
         _entry{ extra_data_type::cell_grass_data,     cell_grass_data::signature,     _handlers::make<cell_grass_data>() },
         _entry{ extra_data_type::cell_imagespace,     cell_imagespace::signature,     _handlers::make<cell_imagespace>() },
         _entry{ extra_data_type::cell_music_override, cell_music_override::signature, _handlers::make<cell_music_override>() },
         _entry{ extra_data_type::cell_region_list,    cell_region_list::signature,    _handlers::make<cell_region_list>() },
         _entry{ extra_data_type::cell_water_type,     cell_water_type::signature,     _handlers::make<cell_water_type>() },
         _entry{ extra_data_type::charge,              charge::signature,              _handlers::make<charge>() },
         _entry{ extra_data_type::collision_data,      collision_data::signature,      _handlers::make<collision_data>() },
         _entry{ extra_data_type::count,               count::signature,               _handlers::make<count>() },
      #pragma endregion
      _entry{ extra_data_type::distant_data, distant_data::signature, _handlers::make<distant_data>() },
      #pragma region E
         _entry{ extra_data_type::emittance_source,    emittance_source::signature,    _handlers::make<emittance_source>() },
         _entry{ extra_data_type::enable_state_parent, enable_state_parent::signature, _handlers::make<enable_state_parent>() },
         _entry{ extra_data_type::encounter_zone,      encounter_zone::signature,      _handlers::make<encounter_zone>() },
      #pragma endregion
      _entry{ extra_data_type::favor_cost, favor_cost::signature, _handlers::make<favor_cost>() },
      _entry{ extra_data_type::global,     global::signature,     _handlers::make<global>() },
      #pragma region H
         _entry{ extra_data_type::headtracking_weight, headtracking_weight::signature, _handlers::make<headtracking_weight>() },
         _entry{ extra_data_type::health,              health::signature,              _handlers::make<health>() },
         _entry{ extra_data_type::health_percent,      health_percent::signature,      _handlers::make<health_percent>() },
         _entry{ extra_data_type::horse,               horse::signature,               _handlers::make<horse>() },
      #pragma endregion
      #pragma region I
         _entry{ extra_data_type::ignored_by_sandbox, ignored_by_sandbox::signature, _handlers::make<ignored_by_sandbox>() },
         _entry{ extra_data_type::interior_lock_list, interior_lock_list::signature, _handlers::make<interior_lock_list>() },
      #pragma endregion
      #pragma region L
         _entry{ extra_data_type::leveled_creature_modifier, leveled_creature_modifier::signature, _handlers::make<leveled_creature_modifier>() },
         _entry{ extra_data_type::leveled_item_base,         leveled_item_base::signature,         _handlers::make<leveled_item_base>() },
         _entry{ extra_data_type::light,                     light::signature,                     _handlers::make<light>() },
         _entry{ extra_data_type::linked_ref,                linked_ref::signature,                _handlers::make<linked_ref>() },
         _entry{ extra_data_type::linked_ref_color,          linked_ref_color::signature,          _handlers::make<linked_ref_color>() },
         _entry{ extra_data_type::lit_water,                 lit_water::signature,                 _handlers::make<lit_water>() },
         _entry{ extra_data_type::location,                  location::signature,                  _handlers::make<location>() },
         _entry{ extra_data_type::location_ref_type,         location_ref_type::signature,         _handlers::make<location_ref_type>() },
         _entry{ extra_data_type::lock,                      lock::signature,                      _handlers::make<lock>() },
      #pragma endregion
      #pragma region M
         _entry{ extra_data_type::map_marker,         map_marker::signature,         _handlers::make<map_marker>() },
         _entry{ extra_data_type::merchant_container, merchant_container::signature, _handlers::make<merchant_container>() },
         _entry{ extra_data_type::multibound_bounds,  multibound_bounds::signature,  _handlers::make<multibound_bounds>() },
         _entry{ extra_data_type::multibound_ref,     multibound_ref::signature,     _handlers::make<multibound_ref>() },
      #pragma endregion
      _entry{ extra_data_type::navmesh_door_portal, navmesh_door_portal::signature, _handlers::make<navmesh_door_portal>() },
      #pragma region O
         _entry{ extra_data_type::occlusion_plane,          occlusion_plane::signature,          _handlers::make<occlusion_plane>() },
         _entry{ extra_data_type::occlusion_plane_ref_data, occlusion_plane_ref_data::signature, _handlers::make<occlusion_plane_ref_data>() },
         _entry{ extra_data_type::ownership,                ownership::signature,                _handlers::make<ownership>() },
      #pragma endregion
      #pragma region P
         _entry{ extra_data_type::package_start_location,        package_start_location::signature,        _handlers::make<package_start_location>() },
         _entry{ extra_data_type::patrol_ref_data,               patrol_ref_data::signature_time,          _handlers::make<patrol_ref_data>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::patrol_ref_data,               patrol_ref_data::signature_event,         _handlers::make<patrol_ref_data>() }, // 
         _entry{ extra_data_type::poison,                        poison::signature_type,                   _handlers::make<poison>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::poison,                        poison::signature_dose,                   _handlers::make<poison>() }, // 
         _entry{ extra_data_type::portal,                        portal::signature,                        _handlers::make<portal>() },
         _entry{ extra_data_type::portal_origin_and_destination, portal_origin_and_destination::signature, _handlers::make<portal_origin_and_destination>() },
         _entry{ extra_data_type::primitive,                     primitive::signature,                     _handlers::make<primitive>() },
      #pragma endregion
      #pragma region R
         _entry{ extra_data_type::radius,                 radius::signature,                 _handlers::make<radius>() },
         _entry{ extra_data_type::ragdoll_data,           ragdoll_data::signature_base,      _handlers::make<ragdoll_data>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::ragdoll_data,           ragdoll_data::signature_biped,     _handlers::make<ragdoll_data>() }, // 
         _entry{ extra_data_type::random_teleport_marker, random_teleport_marker::signature, _handlers::make<random_teleport_marker>() },
         _entry{ extra_data_type::rank,                   rank::signature,                   _handlers::make<rank>() },
         _entry{ extra_data_type::reflector_refs,         reflector_refs::signature,         _handlers::make<reflector_refs>() },
         _entry{ extra_data_type::room_ref_data,          room_ref_data::signature,          _handlers::make<room_ref_data>() },
      #pragma endregion
      #pragma region S
         _entry{ extra_data_type::scale,           scale::signature,           _handlers::make<scale>() },
         _entry{ extra_data_type::spawn_container, spawn_container::signature, _handlers::make<spawn_container>() },
      #pragma endregion
      #pragma region T
         _entry{ extra_data_type::teleport,      teleport::signature,      _handlers::make<teleport>() },
         _entry{ extra_data_type::teleport_name, teleport_name::signature, _handlers::make<teleport_name>() },
         _entry{ extra_data_type::time_left,     time_left::signature,     _handlers::make<time_left>() },
      #pragma endregion
      #pragma region W
         _entry{ extra_data_type::water_current_zone_data, water_current_zone_data::signature_vel_linear,     _handlers::make<water_current_zone_data>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::water_current_zone_data, water_current_zone_data::signature_vel_rotational, _handlers::make<water_current_zone_data>() }, // 
         _entry{ extra_data_type::water_data,              water_data::signature_base,                        _handlers::make<water_data>() }, // This extra-data type has multiple signatures...
         _entry{ extra_data_type::water_data,              water_data::signature_vel,                         _handlers::make<water_data>() }, // 
         _entry{ extra_data_type::water_environment_map,   water_environment_map::signature,                  _handlers::make<water_environment_map>() },
      #pragma endregion
   };

   static constexpr uint32_t _find_first_duplicate() {
      constexpr size_t size = _factories.size();
      uint32_t seen[size];
      for (size_t i = 0; i < size; ++i) {
         const auto& entry = _factories[i];
         seen[i] = entry.signature;
         for (size_t j = 0; j < i; ++j)
            if (seen[j] == entry.signature)
               return entry.signature;
      }
      return 0;
   }
   static_assert(_find_first_duplicate() == 0, "Entries in the list are accidentally sharing the same signature!");

   // Visual Studio: Uncomment these lines and then hover over str.string() 
   // to see the string content. (It triggers an IntelliSense error, but this 
   // is about the only way to get IntelliSense to display the chars as actual 
   // glyphs and not as ints.
   // 
   //constexpr auto str = cobb::string_from_four_cc<_dupe>();
   //constexpr auto str_s = std::string(str.string());
}

namespace dovah::loaded_forms::components {
   basic_extra_data* create_extra_data_by_type(extra_data_type t) {
      for (auto& entry : _factories)
         if (t == entry.type)
            return (entry.handlers.construct)();
      return nullptr;
   }
   basic_extra_data* create_extra_data_for_subrecord(tes_subrecord_reader& subrecord) {
      auto signature = subrecord.signature();
      for (auto& entry : _factories)
         if (signature == entry.signature)
            return (entry.handlers.construct)();
      return nullptr;
   }
   extra_data_load_result generate_extra_data_use_info(tes_record_reader& record, form_stub_use_info_builder& uib, extra_data_use_info_state& state) {
      auto& subrecord = record.get_current_subrecord();
      assert(subrecord && "You need to open a subrecord before calling this.");
      auto  signature = subrecord.signature();
      for (auto& entry : _factories) {
         if (signature == entry.signature) {
            (entry.handlers.use_info)(record, uib, state);
            return extra_data_load_result::succeeded;
         }
      }
      return extra_data_load_result::unrecognized;
   }
   extra_data_type get_extra_data_type_for_subrecord(uint32_t signature) {
      for (auto& entry : _factories)
         if (signature == entry.signature)
            return entry.type;
      return extra_data_type::invalid;
   }
}