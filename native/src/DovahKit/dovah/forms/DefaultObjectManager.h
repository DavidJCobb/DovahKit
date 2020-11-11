#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class DefaultObjectManager : public Form {
      public:
         static constexpr form_type_t form_type = form_type::default_object_manager;
         DefaultObjectManager() : Form(form_type) {};

         using signature_t = uint32_t;

         struct entry {
            form_reference_t form;
            bool is_active_file = false;
         };

         std::unordered_map<signature_t, entry> entries;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc); // TODO: FINISH ME
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual bool _clone_impl(Form* out) const noexcept override;
         virtual bool _save_impl(tes_file_writing::record& record) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}