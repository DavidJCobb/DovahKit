#pragma once
#include <cstdint>
#include <string>
#include "Activator.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/harvestable.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

namespace dovah::loaded_forms {
   class TalkingActivator : public Form {
      public:
         //
         // Technically, TACT subclasses ACTI; however, it will be a lot simpler and cleaner to represent 
         // it as its own class, at least for now. Fields that are technically inherited by TACT yet not 
         // used in-game or in the CK will be omitted here.
         //
         static constexpr const enum form_type form_type = form_type::talking_activator;
         TalkingActivator(const constructor_params& c) : Form(form_type, c) {};
      public:
         using activator_flag    = Activator::activator_flag;
         using activator_flags_t = Activator::activator_flags_t;
         
         struct form_flag : public Activator::form_flag {
            enum : uint32_t {
               no_voice_filter      = 1 << 13,
               radio_station        = 1 << 17,
               non_pip_boy          = 1 << 28, // requires "radio station" flag
               continuous_broadcast = 1 << 30, // requires "radio station" flag
            };
         };

      public:
         // Fields inherited from Activator:
         // 
         //  - Activator::looping_sound (ACTI/VNAM) cannot be defined in game data, because TACT/VNAM is 
         //    a different subrecord, and so TACT never defers to the ACTI subrecord handler for VNAM. As 
         //    such, ACTI/VNAM is omitted here.
         //
         components::papyrus_attachment_data script_data;
         components::object_bounds bounds;
         components::model_ts model; // MODL, MODT, MODS
         std::optional<components::destruction_stage_data> destruction_data;
         components::keyword_list keywords;
         localized_string  name;                // FULL
         color_t           marker_color;        // PNAM
         form_reference_t  looping_sound;       // SNAM
         form_reference_t  water_type;          // WNAM
         form_reference_t  interact_keyword;    // KNAM
         localized_string  activation_verb;     // RNAM
         activator_flags_t activator_flags = 0; // FNAM
         //
         form_reference_t voicetype; // VNAM -> VTYP

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}