#pragma once
#include <cstdint>
#include <vector>
#include "Form.h"
#include "_common.h"
#include "components/bounds.h"
#include "components/papyrus.h"

namespace dovah::loaded_forms {
   class FootstepSet : public Form {
      public:
         static constexpr const enum form_type form_type = form_type::footstep_set;
         FootstepSet(const constructor_params& c) : Form(form_type, c) {};

         enum class footstep_type {
            walk,
            run,
            sprint,
            sneak,
            swim,
         };

      public:
         components::papyrus_attachment_data script_data;
         //
         union _ {
            ~_() { sublists.~array(); }

            std::array<std::vector<form_reference_t>, 5> sublists = {};
            struct {
               std::vector<form_reference_t> walk;
               std::vector<form_reference_t> run;
               std::vector<form_reference_t> sprint;
               std::vector<form_reference_t> sneak;
               std::vector<form_reference_t> swim;
            };
         } footsteps;

      public:
         void load(tes_record_reader&, load_order_interfaces::form_load& intfc);
         static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      protected:
         virtual void _clone_impl(Form* out) const noexcept override;
         virtual void _save_impl(tes_record_writer&, load_order_interfaces::form_save&) override;
         virtual void _clear_impl() noexcept override;
         virtual void _sever_outbound_references_impl(form_stub& other) noexcept override;
   };
}