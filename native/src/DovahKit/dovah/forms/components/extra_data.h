#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../_common.h"
#include "../../../helpers/vector3.h"

//
// SUBRECORDS THAT BEGIN WITH 'X' BUT AREN'T EXTRA-DATA:
//
//  - CELL/XCLW
//

namespace dovah::loaded_forms::components {
   enum class extra_data_type {
      //
      // As of this writing, this list was just copied directly from the game engine 
      // with unknown/unseen entries removed. Once we've finished identifying and 
      // writing code for all extra-data subrecords, we should remove any entries in 
      // this list that don't correspond to known subrecords.
      //
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
         virtual void        save(tes_record_writer&) = 0;
         //
         // Your override for (load) must check the incoming subrecord's signature. When 
         // loading any extra-data subrecord, an (extra_data_list) will blindly call the 
         // (load) override on all existing extra-data objects; it only goes through the 
         // extra-data factory (which matches signatures to extra-data subclasses) if 
         // all existing extra-data objects reject the signature.
         //
         virtual bool load(tes_record_reader&) = 0;
   };
   class extra_data_list {
      public:
         using load_result = extra_data_load_result;
         using list_t      = std::vector<basic_extra_data*>;
      protected:
         list_t content; // contents are owned. list should only allow one of each type.
         //
      public:
         inline const list_t& get_items() const noexcept { return this->content; };
         bool insert(basic_extra_data*); // returns (true) if the insertion succeeded.
         void remove(basic_extra_data*);
         //
         load_result load(tes_record_reader&);
   };

   template<uint32_t signature, extra_data_type et, int bytecount> class buffer_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a fixed-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::array<uint8_t, bytecount> bytes;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            subrecord.read(this->bytes.data(), this->bytes.size());
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->bytes.data());
            subrecord.close();
         }
   };
   template<uint32_t signature, extra_data_type et> class binary_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a variable-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::vector<uint8_t> bytes;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            auto size = subrecord.size();
            this->bytes.resize(size);
            subrecord.read(this->bytes.data(), size);
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->bytes.data(), this->bytes.size());
            subrecord.close();
         }
   };
   template<uint32_t signature, extra_data_type et> class empty_extra_data : public basic_extra_data {
      //
      // Use for when the game actively ignores the content of a subrecord, treating its mere 
      // presence as cause to create or modify some data at run-time.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.close();
         }
   };
   template<uint32_t signature, extra_data_type et> class float_extra_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = signature;
         //
         float value = 0.0F;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() == signature) {
               subrecord.read(this->value);
               return load_result::succeeded;
            }
            return load_result::unrecognized;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->value);
            subrecord.close();
         }
   };
   template<uint32_t signature, extra_data_type et> class formID_extra_data : public basic_extra_data {
      public:
         static constexpr uint32_t signature = signature;
         //
         form_id_t formID;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() == signature) {
               subrecord.read(this->value);
               return load_result::succeeded;
            }
            return load_result::unrecognized;
         }
         virtual void save(tes_record_writer& record) override {
            record.write_formID_subrecord(signature, this->formID);
         }
   };
   template<uint32_t signature, extra_data_type et> class string_extra_data : public basic_extra_data {
      //
      // Use for when the game loads the entire content of a variable-length subrecord as binary 
      // data.
      //
      public:
         static constexpr uint32_t signature = signature;
         //
         std::string value;
         //
         virtual extra_data_type get_type() const noexcept { return et; }
         virtual bool load(tes_subrecord_reader& subrecord) override {
            if (subrecord.signature() != signature)
               return load_result::unrecognized;
            auto size = subrecord.size();
            this->value.resize(size);
            subrecord.read(this->value.data(), size);
            auto length = this->value.find_last_not_of('\0');
            if (length != std::string::npos && length != size)
               this->value.resize(length);
            return load_result::succeeded;
         }
         virtual void save(tes_record_writer& record) override {
            auto& subrecord = record.open_next_subrecord(signature);
            subrecord.write(this->value.data(), this->value.size() + 1);
            subrecord.close();
         }
   };
}