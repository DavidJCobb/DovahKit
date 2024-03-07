#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Static : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::statik;
         Static(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               never_fades               = 0x00000004,
               has_tree_lod              = 0x00000040,
               addon_lod_object          = 0x00000080,
               hide_from_local_map       = 0x00000200,
               has_distant_lod           = 0x00008000,
               uses_hd_lod_texture       = 0x00020000,
               has_currents              = 0x00080000,
               is_marker                 = 0x00800000,
               obstacle                  = 0x02000000,
               navmesh_generation_filter = 0x04000000,
               navmesh_generation_obb    = 0x08000000,
               show_in_world_map         = 0x10000000,
               child_can_use             = 0x20000000,
               navmesh_generation_ground = 0x40000000,
            };
         };

         struct directional_material_data { // DNAM
            struct flag { // SSE-only
               enum type : uint8_t {
                  is_snow = 0x01,
               };
            };
            using flags_t = std::underlying_type_t<flag::type>;

            float            max_angle = 30;  // 30 - 120
            form_reference_t material_object; // MATO
            flags_t          flags     = 0;   // SSE-only
         };

         components::papyrus_attachment_data script_data; // VMAD
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         directional_material_data directional_material;
         std::array<std::string, 4> distant_lod_paths; // present if "Has Distant LOD" flag is set; each string is a null-terminated char[MAX_PATH]

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}