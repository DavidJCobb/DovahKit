#pragma once
#include <cstdint>
#include <string>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

namespace dovah::loaded_forms {
   class Activator : public Form {
      public:
         static constexpr form_type_t form_type = form_type::activator;
         Activator() : Form(form_type) {};

         struct activator_flag {
            activator_flag() = delete;
            enum type : uint16_t {
               no_displacement    = 0x0001,
               ignored_by_sandbox = 0x0002,
            };
         };
         using activator_flags_t = std::underlying_type_t<activator_flag::type>;

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               has_tree_lod              = 0x00000040,
               must_update_anims         = 0x00000100,
               hide_from_local_map       = 0x00000200,
               has_distant_lod           = 0x00008000,
               random_anim_start         = 0x00010000,
               dangerous                 = 0x00020000, // for water activators only?
               ignore_object_interaction = 0x00100000,
               is_marker                 = 0x00800000,
               obstacle                  = 0x02000000,
               navmesh_generation_filter = 0x04000000,
               navmesh_generation_obb    = 0x08000000,
               child_can_use             = 0x20000000,
               navmesh_generation_ground = 0x40000000,
            };
         };

         components::papyrus_attachment_data script_data; // VMAD
         components::object_bounds bounds; // OBND
         components::model model; // MODL, MODT, MODS
         components::destruction_stage_data destruction_data; // DEST
         components::keyword_list keywords; // KSIZ, KWDA
         localized_string  name; // FULL
         color_t           marker_color; // CNAM
         form_reference_t  looping_sound;
         form_reference_t  activation_sound;
         form_reference_t  water_type;
         form_reference_t  interact_keyword;
         localized_string  activation_verb;
         activator_flags_t activator_flags;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}