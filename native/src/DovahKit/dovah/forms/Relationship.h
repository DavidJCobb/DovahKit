#pragma once
#include <cstdint>
#include "Form.h"
#include "_common.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class Relationship : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::relationship;
         Relationship(const constructor_params& c) : Form(form_type, c) {};

         struct form_flag : public Form::form_flag {
            enum : uint32_t {
               secret = 0x00000040,
            };
         };

         struct relationship_flag {
            enum type : uint16_t {
               secret = 0x8000,
            };
         };
         using relationship_flags_t = std::underlying_type_t<relationship_flag::type>;

         enum class rank : uint16_t {
            lover,
            ally,
            confidant,
            friend_, // C++ keyword...
            acquaintance,
            rival,
            foe,
            enemy,
            archnemesis,
         };

         components::papyrus_attachment_data script_data; // VMAD
         //
         form_reference_t referrer;
         form_reference_t referent;
         form_reference_t association_type;
         relationship_flags_t flags = 0;
         enum rank            rank = rank::acquaintance;

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