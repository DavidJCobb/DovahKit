#pragma once
#include <cstdint>
#include <optional>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class MovableStatic : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::movable_static;
         MovableStatic(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_lod_mesh_path_length = 260; // MAX_PATH

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               never_fades               = 0x00000004,
               has_tree_lod              = 0x00000040, // presumed; would be inherited from STAT
               addon_lod_object          = 0x00000080, // presumed; would be inherited from STAT
               hide_from_local_map       = 0x00000200,
               has_distant_lod           = 0x00008000,
               uses_hd_lod_texture       = 0x00020000, // presumed; would be inherited from STAT
               has_currents              = 0x00080000,
               is_marker                 = 0x00800000, // presumed; would be inherited from STAT
               obstacle                  = 0x02000000,
               navmesh_generation_filter = 0x04000000,
               navmesh_generation_obb    = 0x08000000,
               show_in_world_map         = 0x10000000, // presumed; would be inherited from STAT
               child_can_use             = 0x20000000, // presumed; would be inherited from STAT
               navmesh_generation_ground = 0x40000000,
            };
         };

         struct flag {
            enum type : uint8_t {
               on_local_map = 1 << 0,
               is_static    = 1 << 2,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

         components::object_bounds bounds; // OBND
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name; // FULL
         //
         flags_t flags = 0; // DATA
         form_reference_t loop_sound; // SNDR

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}