#pragma once
#include <cstdint>
#include <string>
#include <type_traits>
#include "Form.h"
#include "_common.h"

namespace dovah::loaded_forms {
   class AssociationType : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::association_type;
         AssociationType(const constructor_params& c) : Form(form_type, c) {};

         struct association_flag {
            association_flag() = delete;
            enum type : uint32_t {
               is_family = 0x00000001,
            };
         };
         using association_flags_t = std::underlying_type_t<association_flag::type>;

         association_flags_t flags = 0;
         struct {
            std::string masc; // not localized
            std::string fem;  // not localized
         } referrer;
         struct {
            std::string masc; // not localized
            std::string fem;  // not localized
         } referent;

         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
         //
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_file_writing::record& record, load_order_interfaces::form_save& intfc) override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override {}
         virtual void _clear_impl() noexcept override;
   };
}