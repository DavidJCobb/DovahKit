#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../_common.h"
#include "../../../helpers/vector3.h"
#include "../../form_stub.h"

//
// SUBRECORDS THAT BEGIN WITH 'X' BUT AREN'T EXTRA-DATA:
//
//  - CELL/XCLW
//

namespace dovah::loaded_forms::components {
   enum class extra_data_type {
      unknown_xcza,
      unknown_xczc,
      unknown_xczr,
      unknown_xedl,
      unknown_xenc,
      unknown_xlmb,
      unknown_xnvp,
      unknown_xpsl,
      unknown_xroo,
      unknown_xuse,
      unknown_xwcs, // probably related to XWCN+XWCU, but not used by the game
      unknown_xwlt,
      unknown_xwnt,
      //
      deprecated_xcet, // XCET // (FO3)  Unknown.            Ignored by the game's extra-data loader.
      deprecated_xdcr, // XDCR // (FO3)  Decal Reference.    Ignored by the game's extra-data loader.
      deprecated_xhrs, // XHRS // (TES4) Horse.              Ignored by the game's extra-data loader; use XHOR instead.
      deprecated_xibs, // XIBS // (FO3)  Ignored By Sandbox. Ignored by the game's extra-data loader; use XIS2 instead.
      deprecated_xpci, // XPCI // (TES4) Unknown.            Ignored by the game's extra-data loader.
      deprecated_xrad, // XRAD // (FO3)  Radiation.          Ignored by the game's extra-data loader.
      deprecated_xrdo, // XRDO // (FO3)  Radio data.         Ignored by the game's extra-data loader.
      deprecated_xsed, // XSED // (FO3)  SpeedTree Seed.     Discarded by the game's extra-data loader.
      deprecated_xsol, // XSOL // (TES4) Soul.               Ignored by the game's extra-data loader.
      //
      action,                    // XACT
      activate_parent_data,      // XAPD, XAPR
      activate_ref,              // XACR
      alpha_cutoff,              // XALP
      ammo,                      // XAMT > XAMC-optional
      attach_ref,                // XATR
      cell_acoustic_space,       // XCAS
      cell_climate,              // XCCM // Cell climate for interior cells that use a sky region
      cell_grass_data,           // XCGD
      cell_imagespace,           // XCIM
      cell_music_override,       // XCMO
      cell_region_list,          // XCLR // "Cell List of Regions"
      cell_water_type,           // XCWT
      charge,                    // XCHG
      collision_data,            // XTRI
      count,                     // XCNT
      distant_data,              // XLOD
      emittance_source,          // XEMI
      enable_state_parent,       // XESP
      encounter_zone,            // XEZN
      favor_cost,                // XFVC
      global,                    // XGLB
      headtracking_weight,       // XHTW
      health,                    // XHLT
      health_percent,            // XHLP
      horse,                     // XHOR
      ignored_by_sandbox,        // XIS2 (the XIBS subrecord is checked for, but deprecated)
      interior_lock_list,        // XILL
      leveled_creature_modifier, // XLCM
      leveled_item_base,         // XLIB
      light,                     // XLIG
      linked_ref,                // XLKR
      linked_ref_color,          // XCLP // Editor-only?
      lit_water,                 // XLTW
      location,                  // XLCN
      location_ref_type,         // XLRT
      lock,                      // XLOC
      map_marker,                // XMRK > FNAM > FULL > TNAM
      merchant_container,        // XMRC // Deprecated now that merchant containers belong to factions. CELL and REFR call into the game's extra-data loader for this, but that just ignores it.
      multibound_bounds,         // XMBO
      multibound_ref,            // XMBR
      navmesh_door_portal,       // XNDP
      occlusion_plane,           // XOCP (discarded by the game after load)
      occlusion_plane_ref_data,  // XORD
      ownership,                 // XOWN
      package_start_location,    // XPSL
      patrol_ref_data,           // XPRD, (XPPA > INAM-optional > PDTO or TNAM)-optional
      poison,                    // XPSN > XPSC-optional
      portal,                    // XPTL
      portal_origin_and_destination, // XPOD
      primitive,                 // XPRM
      radius,                    // XRDS
      ragdoll_data,              // XRGB, XRGD
      random_teleport_marker,    // XRTM
      rank,                      // XRNK
      reflector_refs,            // XPWR
      room_ref_data,             // XRMR
      scale,                     // XSCL
      spawn_container,           // XSPC
      teleport,                  // XTEL
      teleport_name,             // XTNM
      time_left,                 // XTIM
      water_current_zone_data,   // XCVL, XCVR
      water_data,                // XWCN > * (XWCN appears to "eat" the next subrecord, whatever it be)
      water_environment_map,     // XWEM
      //
      zzz_enum_count // why is there not a better way to do this
   };
   static constexpr int num_extra_data_types = (int)extra_data_type::zzz_enum_count;

   enum class extra_data_load_result {
      unrecognized,
      failed,
      succeeded,
      requires_record, // basic_extra_data::load should return this if it needs to grab the immediate next subrecord(s)
   };

   class basic_extra_data {
      public:
         using load_result = extra_data_load_result;
      public:
         virtual extra_data_type get_type() const noexcept = 0;
         //
         virtual load_result load(tes_subrecord_reader&) = 0;
         virtual bool        load(tes_record_reader&) { return false; };
         virtual void        save(tes_record_writer&) = 0;
         //
         // The general process for loading a subrecord works as follows:
         //
         //  - The (extra_data_list) will blindly call (basic_extra_data::load) on all 
         //    of its stored extra-data objects until one of them returns a result code 
         //    other than (extra_data_load_result::unrecognized).
         //
         //  - The (load) function that takes a subrecord will be called. Your override 
         //    must check the incoming subrecord's signature and return the appropriate 
         //    (extra_data_load_result) code.
         //
         //    If your extra-data class needs to load additional subrecords immediately 
         //    after this one, then return (extra_data_load_result::requires_record). 
         //    The (load) function that takes a record will then be called immediately. 
         //    This functionality is needed for certain extra-data types that consume 
         //    subrecords immediately after their first (e.g. XMRK, XWCN).
         //
         //  - If none of an (extra_data_list)'s stored extra-data objects handles the 
         //    subrecord, then it will use a factory that maps subrecord signatures to 
         //    extra-data constructors and create a new extra-data object, before then 
         //    calling (load) on it in the same manner as for existing extra-data 
         //    objects.
         //
         // ---------------------------------------------------------------------------
         //
         // Subclasses must also override (generate_use_info). This static function is 
         // called while a subrecord is open, by way of a factory; this means that if 
         // an extra-data class only has one subrecord signature, its use info builder 
         // doesn't need to check the current subrecord's signature.
         //
         static void generate_use_info(tes_record_reader&, form_stub*) = delete;
         //
         virtual basic_extra_data* clone(form_stub& clone_owner) const noexcept = 0;
         virtual void clear_contained_formIDs(form_stub& my_owner) {}
   };
   class extra_data_list {
      public:
         using load_result = extra_data_load_result;
         using list_t      = std::vector<basic_extra_data*>;
      protected:
         list_t content; // contents are owned. list should only allow one of each type.
         //
      public:
         ~extra_data_list();
         //
         inline const list_t& get_items() const noexcept { return this->content; };
         bool insert(basic_extra_data*); // returns (true) if the insertion succeeded.
         void remove(basic_extra_data*);
         void remove_by_type(extra_data_type);
         //
         load_result load(tes_record_reader&);
         void save(tes_record_writer&);
         void clone_from(const extra_data_list& source, form_stub& owner_of_clone);
         //
         static extra_data_load_result generate_use_info(tes_record_reader&, form_stub*);
         //
         basic_extra_data* lookup_by_type(extra_data_type) const noexcept;
         template<class e> inline e* lookup(extra_data_type et) const noexcept {
            return dynamic_cast<e*>(this->lookup_by_type(et));
         }
         //
         basic_extra_data* get_or_create_by_type(extra_data_type) noexcept;
         template<class e> inline e* get_or_create(extra_data_type et) noexcept {
            return dynamic_cast<e*>(this->get_or_create_by_type(et));
         }
   };
}