#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/model.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class StaticCollection : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::static_collection;
         StaticCollection(const constructor_params& c) : Form(form_type, c) {};

         struct placed_instance {
            cobb::vector3<float> pos; // DATA+(0x1C*n)+00
            cobb::vector3<float> rot; // DATA+(0x1C*n)+0C
            float                scale = -1; // DATA+(0x1C*n)+18
         };

         struct form_instance_list {
            form_reference_t base_form; // ONAM -> STAT
            std::vector<placed_instance> instances; // DATA
         };

      public:
         components::object_bounds bounds; // OBND
         components::model_ts model; // MODL, MODT, MODS
         components::papyrus_attachment_data script_data; // VMAD
         //
         std::vector<form_instance_list> forms;

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