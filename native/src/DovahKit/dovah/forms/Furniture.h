#pragma once
#include <cstdint>
#include <string>
#include "Activator.h"
#include "_common.h"
#include "../data/skills.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/harvestable.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

namespace dovah::loaded_forms {
   class Furniture : public Form {
      public:
         //
         // Technically, FURN subclasses ACTI; however, it will be a lot simpler and cleaner to represent 
         // it as its own class, at least for now.
         //
         static constexpr const enum form_type form_type = form_type::furniture;
         Furniture(const constructor_params& c) : Form(form_type, c) {};

      public:
         static constexpr const size_t max_marker_model_path_length = 260;
         static constexpr const size_t max_possible_markers = 25; // per bits available in `furniture_flag`

         struct form_flag : public Activator::form_flag {
            enum : uint32_t {
               is_perch          = 1 <<  7,
               must_exit_to_talk = 1 << 28,
            };
         };

         using activator_flag    = Activator::activator_flag;
         using activator_flags_t = Activator::activator_flags_t;

         struct furniture_flag {
            enum type : uint32_t {
               //
               // Bits [0, 23] indicate enabled markers
               //
               disables_activation = 1 << 25,
               is_perch = 1 << 26,
               must_exit_to_talk = 1 << 27,
               nif_has_a_lean_marker = 1 << 28,
               //
               nif_has_a_sit_marker  = 1 << 30,
               nif_has_a_sleep_marker = 1 << 31,
            };
         };
         using furniture_flags_t = std::underlying_type_t<furniture_flag::type>;

         enum class workbench_type : uint8_t {
            none,
            create_object,
            smithing_weapon,
            enchanting,
            enchanting_experiment,
            alchemy,
            alchemy_experiment,
            smithing_armor,
         };

         struct marker_animation_flag {
            enum type : uint16_t {
               sit   = 1 << 0,
               sleep = 1 << 1,
               lean  = 1 << 2,
            };
         };
         struct marker_entry_flag {
            enum type : uint16_t {
               front = 1 << 0,
               back  = 1 << 1,
               right = 1 << 2,
               left  = 1 << 3,
               up    = 1 << 4,
            };
         };
         struct marker_nif_info {
            uint16_t supported_animations = 0b111;   // FNPR+0x00 // cached from the NIF, perhaps so that if the model goes missing, you can still edit things
            uint16_t supported_entry      = 0b11111; // FNPR+0x02 // cached from the NIF, perhaps so that if the model goes missing, you can still edit things
         };

         struct marker {
            uint32_t         index = 0; // ENAM
            uint32_t         disabled_entry_points = 0; // NAM0
            form_reference_t keyword; // FNMK -> KYWD
         };

      public:
         #pragma region Inherited from ACTI
            components::papyrus_attachment_data script_data;
            components::object_bounds bounds;
            components::model_ts model; // MODL, MODT, MODS
            std::optional<components::destruction_stage_data> destruction_data;
            components::keyword_list keywords;

            localized_string  name;                // FULL
            color_t           marker_color;        // PNAM
            form_reference_t  activation_sound;    // VNAM -> SNDR
            form_reference_t  looping_sound;       // SNAM -> SNDR
            form_reference_t  water_type;          // WNAM
            form_reference_t  interact_keyword;    // KNAM -> KYWD
            localized_string  activation_verb;     // RNAM
            activator_flags_t activator_flags = 0; // FNAM
         #pragma endregion
         //
         furniture_flags_t active_markers_and_furn_flags = 0; // MNAM // bitset // also includes flags
         struct {
            workbench_type type  = workbench_type::none;
            std::optional<dovah::skill> skill;
         } workbench; // WBDT
         form_reference_t associated_spell; // NAM1 -> SPEL
         std::vector<marker> markers; // (ENAM+NAM0+FNMK)[]
         std::vector<marker_nif_info> marker_nif_infos; // FNPR[]
         std::string marker_model; // XMRK

      public:
         marker* get_marker(uint32_t index);
         const marker* get_marker(uint32_t index) const;
         marker& get_or_create_marker(uint32_t index);

         constexpr bool is_marker_enabled(uint32_t index) const { return (this->active_markers_and_furn_flags & (1 << index)) != 0; }
         constexpr void set_marker_enabled(uint32_t index, bool enabled) {
            if (index > 24)
               return;
            if (enabled)
               this->active_markers_and_furn_flags |= 1 << index;
            else
               this->active_markers_and_furn_flags &= ~(1 << index);
         }

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}