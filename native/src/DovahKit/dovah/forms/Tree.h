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
   class Tree : public Form {
      public:
         //
         // Technically, FLOR subclasses ACTI; however, it will be a lot simpler and cleaner to represent 
         // it as its own class, at least for now. Fields that are technically inherited by FLOR yet not 
         // used in-game or in the CK will be omitted here.
         //
         static constexpr const enum form_type form_type = form_type::tree;
         Tree(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               has_distant_lod = 0x00008000,
            };
         };

      public:
         components::object_bounds bounds;
         components::harvestable  harvestable; // PFIG, SNAM, PFPC
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data;
         //
         localized_string name; // FULL
         //
         struct {
            float branch_flexibility = 1; // CNAM+0x04
            struct {
               float amplitude = 0.2;    // CNAM+0x10
               float frequency = 0.025;  // CNAM+0x1C
            } back;
            struct {
               float amplitude = 0.6;    // CNAM+0x0C
               float frequency = 0.075;  // CNAM+0x18
            } front;
            struct {
               float flexibility = 1;    // CNAM+0x24
               float amplitude   = 1;    // CNAM+0x28
               float frequency   = 1;    // CNAM+0x2C
            } leaf;
            struct {
               float amplitude = 0.4;    // CNAM+0x14
               float frequency = 0.035;  // CNAM+0x20
            } side;
            struct {
               float flexibility = 1;    // CNAM+0x00
               float amplitude   = 0.03; // CNAM+0x08
            } trunk;
         } tree_data;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);

      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}