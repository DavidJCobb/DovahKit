#pragma once
#include <array>
#include "../_common.h"
#include "../../data/conditions/comparison_operator.h"
#include "../../data/conditions/parameter_underlying_type.h"
#include "../../data/conditions/run_on_type.h"
#include "../../data/story_manager.h"
#include "./conditions/comparison_data.h"
#include "./conditions/event_parameters.h"
#include "./conditions/parameter.h"
#include "./conditions/working_parameter.h"

namespace dovah {
   namespace conditions {
      struct function_info;
      struct parameter_typeinfo;
   }
   namespace loaded_forms {
      class Package;
      class Quest;
   }
}

namespace dovah::loaded_forms::components {
   namespace conditions {
      using  run_on_type = dovah::conditions::run_on_type;
      class working_condition;
   }

   class condition {
      public:
         struct flag {
            flag() = delete;
            enum type : uint8_t {
               or_linked         = 0x01,
               use_aliases       = 0x02, // force REFR and ACHR arguments to reference alias IDs
               compare_to_global = 0x04,
               use_package_data  = 0x08, // force REFR and ACHR arguments to package data indices (ObjectList and SingleRef packdata types only)
               swap_subject_and_target = 0x10,
            };
         };
         using flags_t = std::underlying_type_t<flag::type>;
         
         using comparison_data = conditions::comparison_data;
         using run_on_type     = conditions::run_on_type;

         struct run_on_data {
            run_on_type type = run_on_type::subject;
            uint32_t    index = -1;
            form_reference_t reference;
         };
         
      protected:
         flags_t  flags    = 0;
         uint16_t function = 0;
         std::array<conditions::parameter, 2> parameters;
         conditions::event_parameters event_parameters; // used instead of (parameters) for GetEventData
         comparison_data comparison;
         run_on_data     run_on;
         
      public:
         const dovah::conditions::parameter_typeinfo* get_argument_type(uint8_t index) const noexcept;
         dovah::conditions::parameter_underlying_type get_argument_underlying_type(uint8_t index) const noexcept;
         
         #pragma region Accessors
            constexpr uint16_t get_function_id() const noexcept { return this->function; }
            [[nodiscard]] const dovah::conditions::function_info* get_function() const noexcept;
            [[nodiscard]] const conditions::working_parameter get_parameter(uint8_t i) const;
         
            constexpr const auto& get_event_parameters() const { return this->event_parameters; }
            constexpr const auto& get_comparison() const noexcept { return this->comparison; }
            constexpr const auto& get_run_on_data() const noexcept { return this->run_on; }
         
            constexpr auto get_flags() const noexcept { return this->flags; }
            constexpr bool test_flags(flags_t f) const noexcept { return (this->flags & f); }
         #pragma endregion

         bool refers_to_form(const form_stub*) const noexcept;
      
         #pragma region Form boilerplate
            bool read(tes_record_reader&, load_order_interfaces::form_load&); // assumes we've already opened a CTDA subrecord
            static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
            void save(tes_record_writer&, load_order_interfaces::form_save&); // call with no subrecord open
            void clone_from(const condition& source, loaded_forms::Form& my_owner) noexcept;
            void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
            void clear(loaded_forms::Form& my_owner);
         #pragma endregion

         void commit(loaded_forms::Form& my_owner, const conditions::working_condition& source);
   };

   class condition_list : public std::vector<condition> {
      public:
         using vector::vector;

         void clear() noexcept = delete;
         void clear(loaded_forms::Form& my_containing_form) noexcept;

         void resize() = delete; // use reserve + push_back/emplace_back

         void append_all_of(loaded_forms::Form& my_containing_form, const std::vector<condition>& other);
         bool read_next(tes_record_reader&, load_order_interfaces::form_load&);

         void append(loaded_forms::Form& my_containing_form, const conditions::working_condition&);
         void append_all_of(loaded_forms::Form& my_containing_form, const std::vector<conditions::working_condition>&);
   };
}
