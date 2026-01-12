#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/extra_data.h"
#include "components/papyrus.h"
#include "../use_info/entry_flags/reference.h"
#include "../../helpers/vector3.h"

namespace dovah::loaded_forms {
   class ObjectReference : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::reference;
         ObjectReference(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum type : uint32_t {
               //
               // Some flags' meanings differ depending on the base form's type. If a line comment after a 
               // value consists of a list of signatures, then it indicates the form types for which that 
               // value is relevant. If the same value is given two different names and only one has a list 
               // of signatures, then the other value applies to all form types not covered by the first.
               //
               hide_from_local_map_a     = 0x00000040, // DOOR
               turn_off_fire             = 0x00000080, // any form whose model has BSX flag 1 << 4 set
               doesnt_light_water        = 0x00000100, // LIGH
               inaccessible              = 0x00000100, // DOOR
               hide_from_local_map_b     = 0x00000200, // ACTI, STAT, TREE
               casts_shadows             = 0x00000200, // LIGH
               motion_blur               = 0x00000200, // MSTT
               starts_dead               = 0x00000200, // ACHR
               persistent                = 0x00000400,
               disabled                  = 0x00000800,
               is_sky_marker             = 0x00002000, // XMarkerHeading
               visible_when_distant      = 0x00008000, // ACTI, STAT, TREE
               is_full_lod               = 0x00010000, // everything except LIGH. if ref is in an exterior, then setting this should also set "persistent"
               never_fades               = 0x00010000, // LIGH
               doesnt_light_landscape    = 0x00020000, // LIGH
               no_ai_acquire             = 0x02000000, // ACHR, CONT, LIGH, all items
               navmesh_generation_filter = 0x04000000,
               navmesh_generation_obb    = 0x08000000,
               reflected_by_auto_water   = 0x10000000,
               dont_havok_settle         = 0x20000000, // ACHR, ACTI, ADON, DOOR, LIGH, MSTT, STAT, TREE, all items
               navmesh_generation_ground = 0x40000000, // ACTI (accidentally overlaps `no_respawn`), CONT, general
               no_respawn                = 0x40000000, // ACTI, ADON, DOOR, LIGH, MSTT, STAT, TREE, all items
               multibound                = 0x80000000,
            };
         };

         components::extra_data_list         extra_data;
         components::papyrus_attachment_data script_data; // VMAD
         unique_form_reference_t<use_info::entry_flags::reference::base_form> base_form; // NAME
         cobb::vector3<float> position; // DATA
         cobb::vector3<float> rotation; // DATA // radians

         // These functions all potentially throw dovah::exceptions::object_reference_move_failed.
         void set_position(cobb::vector3<float> position);
         void set_position_and_parent(cobb::vector3<float> position, form_stub& world_or_cell);
         void set_position_and_cell(cobb::vector3<float> position, form_stub& parent_cell);
         void set_position_and_world(cobb::vector3<float> position, form_stub& parent_world);

         float get_scale() const; // applies in-game precision and range limits; if you want to know the in-ESP value, access the "scale" extra-data yourself
         float get_raw_scale() const;
         void set_scale(float, bool with_limits = false);

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         ObjectReference(enum form_type ft, const constructor_params& c) : Form(ft, c) {}; // for subclasses
         //
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
         virtual bool _friendly_delete_impl(const file_load_order&) noexcept override;
   };
}