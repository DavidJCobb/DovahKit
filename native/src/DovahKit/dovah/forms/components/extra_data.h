#pragma once
#include <array>
#include <cstdint>
#include <string>
#include "../_common.h"
#include "../../../helpers/vector3.h"

namespace dovah::loaded_forms::components::extra {
   struct activate_ref {
      static constexpr uint32_t signature = 'XACR';
      //
      uint32_t unk00 = 0;
      uint32_t unk04 = 0;
      uint32_t unk08 = 0;
   };
   struct alpha_cutoff {
      static constexpr uint32_t signature = 'XALP';
      //
      uint8_t cutoff;
      uint8_t base;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct applied_poison {
      static constexpr uint32_t signature_type  = 'XPSN';
      static constexpr uint32_t signature_count = 'XPSC';
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct enable_state_parent {
      static constexpr uint32_t signature = 'XESP';
      struct flag {
         flag() = delete;
         enum : uint8_t {
            opposite = 0x01, // uses the opposite state of its parent
            pop_in   = 0x02,
         };
      };
      //
      form_id_t reference;
      uint8_t   flags = 0;
      uint8_t   padding[3];
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct light_data {
      static constexpr uint32_t signature = 'XLIG';
      //
      float    fov;
      float    fade;
      uint32_t unk08;
      float    shadow_depth_bias = 1.0F;
      float    unk10;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct linked_ref {
      static constexpr uint32_t signature = 'XLKR';
      //
      form_id_t keyword; // optional; struct can be 8 bytes (KYWD, ref) or 4 bytes (ref)
      form_id_t ref;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct lock_data { // read as a struct
      static constexpr uint32_t signature = 'XLOC';
      struct flag {
         flag() = delete;
         enum : uint8_t {
            leveled = 0x04,
         };
      };
      //
      uint8_t          level; // from 0 to 255; thresholds are: novice = 1; apprentice = 25; adept = 50; expert = 75; master = 100; requires key = 255
      uint8_t          pad01[3];
      struct_form_id_t key;
      uint8_t          flags = 0;
      uint8_t          pad09[3];
      uint32_t         unk0C;
      uint32_t         unk10;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct multibound_halfwidths {
      static constexpr uint32_t signature = 'XMBO';
      //
      cobb::vector3<float> bounds;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct navmesh_door_portal {
      static constexpr uint32_t signature = 'XNDP';
      //
      form_id_t navmesh;
      int16_t   teleport_marker_triangle;
      uint16_t  unused;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct occlusion_plane_data {
      static constexpr uint32_t signature = 'XOCP';
      //
      float width;
      float height;
      cobb::vector3<float> position;
      struct { // quaternion
         float a;
         float b;
         float c;
         float d;
      } rotation;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct occlusion_plane_ref_data {
      static constexpr uint32_t signature = 'XORD'; // found in code, but not seen in Skyrim.esm. maybe the DLCs?
      //
      uint32_t unk00 = 0;
      uint32_t unk04 = 0;
      uint32_t unk08 = 0;
      uint32_t unk0C = 0;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
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
   struct portal { // unused?
      static constexpr uint32_t signature = 'XPTL';
      //
      float width;
      float height;
      cobb::vector3<float> position;
      struct { // quaternion?
         float a;
         float b;
         float c;
         float d;
      } rotation;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct portal_object_data {
      static constexpr uint32_t signature = 'XPOD';
      //
      form_id_t origin;      // REFR
      form_id_t destination; // REFR
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct primitive {
      static constexpr uint32_t signature = 'XPRM';
      enum class shape_t : uint32_t {
         none,
         box,
         sphere,
         portal_box,
         unknown, // plane, maybe?
      };
      //
      cobb::vector3<float> bounds;
      struct {
         float r;
         float g;
         float b;
      } color;
      float   unk18;
      shape_t shape = shape_t::none;
   };
   struct ragdoll_biped_data {
      static constexpr uint32_t signature = 'XRGB';
      //
      std::array<uint8_t, 0xC> bytes; // not floats, ints, or form IDs. hashes, maybe?
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
   struct teleport {
      static constexpr uint32_t signature = 'XTEL';
      struct flag {
         flag() = delete;
         enum : uint32_t {
            no_alarm = 0x01,
         };
      };
      //
      form_id_t door; // REFR
      cobb::vector3<float> position;
      cobb::vector3<float> rotation; // radians
      uint32_t flags;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct water_current_zone_data {
      static constexpr uint32_t signature_vel_linear     = 'XCVL';
      static constexpr uint32_t signature_vel_rotational = 'XCVR';
      //
      struct {
         cobb::vector3<float> linear;     // XCVL
         cobb::vector3<float> rotational; // XCVR
      } velocity;
      //
      void load(tes_subrecord_reader&);
      void save(tes_subrecord_writer&); // open the subrecord before calling
   };
   struct water_data {
      static constexpr uint32_t signature = 'XWCN';
      //
      // ?????
      // See TESV.exe: 0x0041F70A
      //
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
   // XEMI | Emittance              | A for mID of type LIGH or REGN.
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