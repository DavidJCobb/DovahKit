#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../_common.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components {
   enum class extra_data_type {
      //
      // As of this writing, this list was just copied directly from the game engine 
      // with unknown/unseen entries removed. Once we've finished identifying and 
      // writing code for all extra-data subrecords, we should remove any entries in 
      // this list that don't correspond to known subrecords.
      //
      action,
      activate_loop_sound,
      activate_parent_data,      // XAPD, XAPR
      activate_ref,              // XACR
      actor_cause,
      alias_instance_array,
      alpha_cutoff,              // XALP
      ammo,                      // XAMT > XAMC-optional
      anim_graph_manager,
      anim_note_receiver,
      ash_pile_ref,
      attach_ref,                // XATR
      attach_ref_children,
      attached_arrows_3d,
      bad_position,
      cached_scale,
      can_talk_to_player,
      cannot_wear,
      cell_acoustic_space,       // XCAS
      cell_climate,              // XCCM // Cell climate for interior cells that use a sky region
      cell_grass_data,           // XCGD
      cell_imagespace,           // XCIM
      cell_music_override,       // XCMO
      cell_region_list,          // XCLR // "Cell List of Regions"
      cell_water_type,           // XCWT
      charge,                    // XCHG
      collision_data,            // XTRI
      combat_style,
      container_changes,
      count,                     // XCNT
      creature_awake_sound,
      decal_group,
      detach_time,
      dismembered_limbs,
      distant_data,              // XLOD
      dropped_item_list,
      editor_id,
      editor_ref_move_data,
      emittance_source,          // XEMI
      enable_state_children,
      enable_state_parent,       // XESP
      enchantment,
      encounter_zone,            // XEZN
      faction_changes,
      favor_cost,                // XFVC
      flags,
      follower,
      follower_swim_breadcrumbs,
      forced_landing_marker,
      forced_target,
      friend_hits,
      from_alias,
      ghost,
      gid_buffer,
      global,                    // XGLB
      group_constraint,
      guarded_ref_data,
      has_no_rumors,
      havok,
      heading_target,
      headtracking_weight,       // XHTW
      health,
      health_percent,            // XHLP
      horse,                     // XHOR
      hotkey,
      ignored_by_sandbox,        // XIS2 (the XIBS subrecord is checked for, but deprecated)
      info_general_topic,
      interaction,
      interior_lock_list,        // XILL
      item_dropper,
      large_ref_owner_cells,
      last_finished_sequence,
      leveled_creature,
      leveled_creature_modifier, // XLCM
      leveled_item,
      leveled_item_base,         // XLIB
      light,                     // XLIG
      light_data,
      linked_ref,                // XLKR
      linked_ref_children,
      lit_water_refs,
      location,                  // XLCN
      location_ref_type,         // XLRT
      lock,                      // XLOC
      magic_caster,
      map_marker,                // XMRK > FNAM > FULL > TNAM
      missing_ref_ids,
      model_swap,
      multibound,
      multibound_bounds,         // XMBO
      multibound_ref,            // XMBR
      navmesh_door_portal,       // XNDP
      north_rotation,
      object_health,
      occlusion_plane,           // XOCP (discarded by the game after load)
      occlusion_plane_ref_data,  // XORD
      occlusion_shape,
      open_close_activate_ref,
      original_reference,
      outfit_item,
      ownership,                 // XOWN
      package,
      package_data,
      package_start_location,    // XPSL
      patrol_ref_data,
      patrol_ref_in_use_data,
      persistent_cell,
      player_crime_list,
      poison,                    // XPSN > XPSC-optional
      portal,                    // XPTL
      portal_origin_and_destination, // XPOD
      primitive,                 // XPRM
      process_middle_low,
      promoted_ref,
      race_data,
      radio_data,
      radius,                    // XRDS
      ragdoll_data,              // XRGB, XRGD
      random_teleport_marker,    // XRTM
      rank,                      // XRNK
      ref_path,
      reference_handle,
      reflected_refs,
      reflector_refs,
      refraction_property,
      reserved_markers,
      room,
      room_ref_data,
      run_once_packages,
      saved_animation,
      saved_havok_data,
      say_topic_info,
      say_topic_info_once_a_day,
      scale,                     // XSCL
      scene_data,
      scripted_anim_dependence,
      seed,
      seen_data,
      should_wear,
      soul,
      sound,
      spawn_container,           // XSPC
      starting_position,
      starting_world_or_cell,
      teleport,                  // XTEL
      teleport_name,             // XTNM
      terminal_state,
      text_display_data,
      time_left,                 // XTIM
      trespass_package,
      unique_id,
      used_markers,
      water_current_zone_data,   // XCVL, XCVR
      water_data,                // XWCN > * (XWCN appears to "eat" the next subrecord, whatever it be)
      water_environment_map,     // XWEM
      water_light_refs,
      weapon_attack_sound,
      weapon_idle_sound,
      worn,
      worn_left,
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

namespace dovah::loaded_forms::components::extra {
   struct placed_water_reflections { // the water is placed, not the reflections
      static constexpr uint32_t signature = 'XPWR';
      enum class type_t : uint32_t {
         reflection,
         refraction,
      };
      //
      form_id_t reference; // REFR
      type_t    type;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct room_marker {
      static constexpr uint32_t signature = 'XRMR';
      struct flag {
         flag() = delete;
         enum : uint8_t {
            has_imagespace        = 0x40, // REFR/INAM
            has_lighting_template = 0x80, // REFR/LNAM
         };
      };
      //
      uint8_t  linked_room_count; // linked rooms are REFR/XLRM (multiple?)
      uint8_t  flags;
      uint16_t unknown;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };

   //
   // LIST OF BASIC EXTRA-DATA:
   //
   // XACT | Action Flag            | A dword enum. See xEdit defs.
   // XAMC | Ammo                   | A dword. Must appear after XAMT, or it will be ignored. Probably "ammo count."
   // XAMT | Ammo                   | A dword.
   // XATR | Attach Ref             | A form ID of any reference form.
   // XCAS | Cell Acoustic Space    | A form ID of type ASPC.
   // XCCM | Cell Climate Region    | A form ID of type REGN.
   // XCGD | Cell Grass Data        | A buffer of grass-related information, loaded all at once and stored somewhere.
   // XCHG | Charge                 | A float.
   // XCIM | Cell Imagespace        | A form ID of type IMGS.
   // XCLR | Cell List of Regions   | A list of region form IDs. No length prefix; just divide the subrecord length by 4.
   // XCMO | Cell Music             | A form ID of type MUSC.
   // XCNT | Count                  | An int32_t.
   // XCWT | Cell Water Type        | A form ID of type WATR.
   // XCZC | Unknown                | The game skips it. xEdit has it listed as a single form ID of any cell. Probably Z-cell.
   // XCZR | Unknown                | The game skips it. xEdit has it listed as a single form ID of any reference form.
   // XEMI | Emittance              | A form ID of type LIGH or REGN.
   // XFVC | Favor Cost             | A float.
   // XHLP | Health Percentage      | A float.
   // XHLT | Health                 | An int32_t.
   // XHOR | Horse                  | A form ID of type ACHR.
   // XHTW | Headtracking Weight    | A float.
   // XILL | Interior Lock List     | A form ID of type FLST or NPC_.
   // XIS2 | Ignored By Sandbox     | The content isn't read; the subrecord's mere presence is enough.
   // XLCM | Leveled Crea. Modifier | An int32_t enum: easy, medium, hard, or very hard.
   // XLCN | Location               | A form ID of type LCTN.
   // XLIB | LeveledItem Base       | A form ID of type LVLI.
   // XLMB |                        | The game skips loading this.
   // XLOD | Distant Data           | Three dwords, likely floats.
   // XLRT | Location Ref Type      | A form ID of a LocRefType.
   // XLTW | Lit Water              | A form ID of a water REFR. This appears on LIGH REFRs. One subrecord per water ref.
   // XMBR | Multibound Ref         | A form ID of a REFR.
   // XOWN | Owner                  | A form ID of type FACT or NPC_.
   // XPRD | Patrol Idle Time       | A float?
   // XPSC | Poison Count           | Amount of poison doses applied to a form. Only works if the form already has ExtraPoison before XPSC is seen.
   // XPSN | Poison                 | A poison form ID of type ALCH. Creates an ExtraPoison -- so, XPSC must appear after this.
   // XRDS | Radius                 | A float.
   // XRNK | Faction Rank           | An int32_t.
   // XRTM | Random Teleport Marker | A form ID of a REFR.
   // XSED |                        | Either a dword or a single byte. Value is loaded but not stored anywhere; it's just discarded.
   // XSPC | Spawn Container        | A form ID of a REFR.
   // XTIM | Time Left              | A dword. Some sort of countdown timer before a bound weapon is unequipped?
   // XTRI | Collision Layer        | A uint32_t.
   // XWEM | Water Environment Map  | A string.
   //
   // LIST OF EXTRA-DATA PENDING ANALYSIS:
   //
   // XAPD | Activate Parents     | Check xEdit defs.
   // XAPR | Activate Parents     | Check xEdit defs.
   // XMRK | Map Marker Data      | Empty subrecord which MUST be followed by multiple related ones; see 00429080 for code to read those.
   // XPPA | Patrol Script Marker | ExtraPatrolRefData::Data; see TESPackage::Data::Load
   // XPSL | Package Start Loc.   | See 0041F41A.
   // XRGD | Ragdoll Data         | 
   //
}