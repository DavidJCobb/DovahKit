#pragma once
#include <cstdint>
#include <string>
#include "./Form.h"
#include "./_common.h"
#include "./components/bounds.h"
#include "./components/destruction.h"
#include "./components/keyword_list.h"
#include "./components/model.h"
#include "./components/papyrus.h"

namespace dovah::loaded_forms {
   class MaterialType : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::material_type;
         MaterialType(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_name_length = 0x104;

         struct flag {
            enum type : uint32_t {
               stairs       = 1 << 0,
               arrows_stick = 1 << 1,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
         
      public:
         components::papyrus_attachment_data script_data; // VMAD
         
         flags_t          flags = 0;    // FNAM
         std::string      name;         // MNAM
         struct {
            float r = 0;
            float g = 0;
            float b = 0;
         } color; // CNAM
         form_reference_t parent;       // PNAM -> MATT
         float            buoyancy = 0; // BNAM
         form_reference_t impact_data_set; // HNAM -> IPDS

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