#pragma once
#include <array>
#include <cstdint>
#include <optional>
#include <variant>
#include "../../../data/conditions/comparison_operator.h"
#include "../../../data/conditions/parameter_underlying_type.h"
#include "../../../data/conditions/run_on_type.h"
#include "./working_comparison.h"
#include "./working_parameter.h"

namespace dovah {
   namespace conditions {
      class parameter_typeinfo;
   }
   namespace loaded_forms {
      namespace components {
         class condition;
      }
      class Form;
   }
   class form_stub;
}

namespace dovah::loaded_forms::components::conditions {
   using comparison_operator = dovah::conditions::comparison_operator;
   using run_on_type         = dovah::conditions::run_on_type;

   enum class parameter_type_override {
      none,
      alias,
      package_data,
   };

   class working_condition {
      public:
         constexpr working_condition() {}
         working_condition(const condition& src);

         constexpr bool operator==(const working_condition&) const noexcept = default;

         struct event_data {
            constexpr bool operator==(const event_data&) const noexcept = default;

            uint16_t   function = 0;
            uint16_t   member   = 0;
            form_stub* form     = nullptr;
         };

      public:
         struct _ {
            constexpr bool operator==(const _&) const noexcept = default; // ugh

            bool or_linked               = false;
            bool swap_subject_and_target = false;
         } flags;
         parameter_type_override override_types_with = parameter_type_override::none; // force REFR and ACHR arguments to refer to entities on the owning quest/package

         uint16_t function = 0;

         std::array<working_parameter, 2> parameters;
         std::optional<event_data> event_parameters; // used instead of `parameters` for GetEventData

         working_comparison comparison;
         struct __ {
            constexpr bool operator==(const __&) const noexcept = default; // ugh

            run_on_type type  = run_on_type::subject;
            std::variant<
               std::monostate, // used for these run-on types: subject, target, linked ref, combat target
               uint32_t,       // owning-quest alias ID, owning-package package-data index, or event-data index
               form_stub*      // used for: reference (note: we encode "Player" as reference)
            > entity;
         } run_on;

      public:
         bool valid() const;

         // does not apply `override_types_with`
         const dovah::conditions::parameter_typeinfo* get_argument_typeinfo(size_t index) const;

         // applies `override_types_with`
         const dovah::conditions::parameter_typeinfo* get_effective_argument_typeinfo(size_t index) const noexcept;

         // applies `override_types_with`
         dovah::conditions::parameter_underlying_type get_argument_underlying_type(size_t index) const noexcept;

         void set_function_id(uint16_t); // handles converting/resetting params as appropriate
         void set_type_override(parameter_type_override); // handles convertnig/resetting params as appropriate
         void reset_parameter(size_t);
         void reset_parameters();
         void reset_run_on_entity();

         bool refers_to_form(const dovah::form_stub*) const noexcept;

         // Returns true if any changes are made to the condition.
         bool sever_outbound_references_to(const dovah::form_stub*) noexcept;
   };
}