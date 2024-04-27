#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class DefaultObjectManager : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::default_object_manager;
         DefaultObjectManager(const constructor_params& c) : Form(form_type, c) {};

         using signature_t = uint32_t;

         struct entry {
            form_reference_t form;
            bool is_active_file = false; // or is edited
         };

         std::unordered_map<signature_t, entry> entries;

         form_stub* get_entry(signature_t) const noexcept;
         void set_entry(signature_t, form_stub*); // can throw `dovah::exceptions::default_object_assign_failed`
         bool entry_is_edited(signature_t) const noexcept;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc); // TODO: FINISH ME
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
         virtual void _clear_impl() noexcept override;
   };
}