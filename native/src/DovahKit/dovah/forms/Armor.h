#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/biped_object.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/enchantable.h"
#include "components/keyword_list.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../utils/data_by_sex.h"
#include "../use_info/entry_flags/armor.h"

namespace dovah::loaded_forms {
   class Armor : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::armor;
         Armor(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               non_playable         = 1 <<  2,
               shield               = 1 <<  6,
               visible_when_distant = 1 << 15,
            };
         };

         struct world_model {
            components::model_ts model; // MOD*, MO*T, MO*S (male: 2; female: 4)
            struct {
               std::string inventory; // ICON (male) and ICO2 (female)
               std::string message;   // MICO (male) and MIC2 (female)
            } icons;
         };

      public:
         components::biped_object  biped_object; //  BODT, BOD2
         components::object_bounds bounds; // OBND
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         components::enchantable   enchantable; // EITM, EAMT
         components::keyword_list  keywords;    // KSIZ, KWDA
         components::papyrus_attachment_data script_data; // VMAD
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         std::vector<form_reference_t> armor_addons; // MODL[] -> ARMA
         struct {
            form_reference_t alternate_material; // BAMT -> MATT
            form_reference_t impact_data_set;    // BIDS -> IPDS
         } block_bash;
         form_reference_t equip_type; // ETYP
         form_reference_t race;       // RNAM -> RACE
         std::string      ragdoll_constraint_template; // BMCT
         int32_t          rating = 0; // DNAM
         struct {
            form_reference_t take; // YNAM
            form_reference_t drop; // ZNAM
         } sounds;
         unique_form_reference_t<use_info::entry_flags::armor::template_form> template_armor; // TNAM -> ARMO
         int32_t value = 0;  // DATA+0x00
         float   weight = 0; // DATA+0x04
         data_by_sex<world_model> world_models;

         void copy_data_from_template_armor(form_stub& source);

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}