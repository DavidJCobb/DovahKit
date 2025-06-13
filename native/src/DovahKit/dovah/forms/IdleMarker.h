#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class IdleMarker : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::idle_marker;
         IdleMarker(const constructor_params& c) : Form(form_type, c) {};

         static constexpr const size_t max_idles_count = std::numeric_limits<uint8_t>::max();

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               child_can_use = 0x20000000,
            };
         };

         struct flag {
            enum type : uint8_t {
               run_in_sequence    = 1 << 0,
               do_once            = 1 << 2,
               ignored_by_sandbox = 1 << 4,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;

      public:
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         flags_t flags = 0;
         float   timer = 0;
         std::vector<form_reference_t> idles;

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