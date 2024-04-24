#pragma once
#include <cstdint>
#include <string>
#include "Activator.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "structs/color_dword.h"

namespace dovah::loaded_forms {
   class Flora : public Form {
      public:
         //
         // Technically, FLOR subclasses ACTI; however, it will be a lot simpler and cleaner to represent 
         // it as its own class, at least for now. Fields that are technically inherited by FLOR yet not 
         // used in-game or in the CK will be omitted here.
         //
         static constexpr const enum form_type form_type = form_type::flora;
         Flora(const constructor_params& c) : Form(form_type, c) {};
      public:
         using activator_flag    = Activator::activator_flag;
         using activator_flags_t = Activator::activator_flags_t;
         using form_flag         = Activator::form_flag;
         
         // Fields inherited from Activator:
         // 
         //  - Activator::looping_sound (ACTI/SNAM) cannot be defined in game data, because FLOR/SNAM is 
         //    a different subrecord, and so FLOR never defers to the ACTI subrecord handler for SNAM. As 
         //    such, ACTI/SNAM is omitted here.
         //
         components::papyrus_attachment_data script_data; // VMAD
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::keyword_list keywords; // KSIZ, KWDA
         localized_string  name;                // FULL
         color_t           marker_color;        // CNAM
         form_reference_t  activation_sound;    // VNAM
         form_reference_t  water_type;          // WNAM
         form_reference_t  interact_keyword;    // KNAM
         localized_string  activation_verb;     // RNAM
         activator_flags_t activator_flags = 0; // FNAM
         //
         // Unique fields:
         //
         form_reference_t ingredient;    // PFIG
         form_reference_t harvest_sound; // SNAM; form type SNDR // shadows ACTI/SNAM, so that superclass field will never load
         struct {
            uint8_t spring = 100;
            uint8_t summer = 100;
            uint8_t autumn = 100;
            uint8_t winter = 100;
         } chance_by_season;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}