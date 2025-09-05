#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/destruction.h"
#include "components/model.h"
#include "components/papyrus.h"
#include "../data/skill_level.h"

namespace dovah::loaded_forms {
   class Apparatus : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::apparatus;
         Apparatus(const constructor_params& c) : Form(form_type, c) {};

         //
         // BGSApparatus is a subclass of TESObjectMISC, but doesn't load all of the 
         // fields. For example, MiscItems have a keyword list, but Apparatus forms 
         // never load it.
         //

      public:
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         std::optional<components::destruction_stage_data> destruction_data; // DEST
         //
         localized_string name        = localized_string(localized_string_type::common);      // FULL
         localized_string description = localized_string(localized_string_type::description); // DESC
         //
         struct {
            std::string inventory; // ICON
            std::string message;   // MICO
         } icons;
         struct {
            form_reference_t take; // YNAM
            form_reference_t drop; // ZNAM
         } sounds;
         skill_level quality = skill_level::novice;
         int32_t value  = 0; // DATA+0x00
         float   weight = 0; // DATA+0x04

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer& record, load_order_interfaces::form_save& intfc) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}