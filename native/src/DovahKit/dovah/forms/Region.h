#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <vector>
#include "Form.h"
#include "./_common.h"
#include "./components/papyrus.h"
#include "./structs/region/area.h"
#include "./structs/region/generable_content_collection.h"
#include "./structs/color_dword.h"
#include "../use_info/entry_flags/region.h"

namespace dovah::loaded_forms {
   class Region : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::region;
         Region(const constructor_params& c) : Form(form_type, c) {};
         
         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               border_region = 1 << 6,
            };
         };

         using generable_content_collection = structs::region::generable_content_collection;
         using region_area = structs::region::area;

      public:
         components::papyrus_attachment_data script_data; // VMAD
         //
         color_t map_color; // RCLR
         unique_form_reference_t<use_info::entry_flags::region::worldspace> parent_world; // WNAM -> WRLD
         std::vector<region_area> areas; // (RPLI+RPLD[])[]
         std::vector<generable_content_collection> generable_content; // (RDAT+...)[]

      public:
         void delete_invalid_areas();

         // If there's more than one of any given collection type, merge them.
         // Does not attempt to avoid duplicate entries, etc..
         void fold_generable_content();

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