#pragma once
#include <array>
#include "../_common.h"
#include "../../data/conditions.h"
#include "../../data/story_manager.h"

namespace dovah::loaded_forms {
   class Package;
   class Quest;
}

namespace dovah::loaded_forms::components {
   //
   // Simple struct intended for outside code to work with.
   //
   struct condition_parameter {
      union {
         uint32_t dword = 0;
         float    float32;
         int32_t  integer;
      };
      form_stub*  form = nullptr;
      std::string string;
      //
      condition_parameter_underlying_type underlying = condition_parameter_underlying_type::none;
   };

   //
   // A condition parameter as stored inside of a condition; meant for internal use only. It has 
   // to be defined outside of the (condition) struct so that it can be forward-declared and used 
   // in some of the condition function definition code.
   //
   struct condition_parameter_in_situ {
      union {
         uint32_t dword = 0;
         float    float32;
         int32_t  integer;
      };
      form_reference_t form;
      std::string      string;
      //
      condition_parameter_underlying_type underlying = condition_parameter_underlying_type::none;
   };

   struct condition_event_parameters {
      uint16_t function;
      uint16_t member;
      form_reference_t form;
   };

   class working_condition;

   class condition {
      public:
         enum class run_on_type : uint32_t {
            subject       = 0,
            target        = 1,
            reference     = 2, // i.e. condition::run_on_reference
            combat_target = 3,
            linked_ref    = 4,
            quest_alias   = 5,
            package_data  = 6, // where does (condition) store *which* packdata we're running on?
            event_data    = 7,
         };
         enum class operator_type {
            equal            = 0,
            not_equal        = 1,
            greater          = 2,
            greater_or_equal = 3,
            less             = 4,
            less_or_equal    = 5,
         };
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
         //
         struct comparison_data {
            operator_type op = operator_type::equal;
            struct {
               float            constant;
               form_reference_t global;
            } operand;
         };
         struct run_on_data {
            run_on_type type = run_on_type::subject;
            uint32_t    index = -1;
            form_reference_t reference;
         };
         //
      protected:
         flags_t  flags    = 0;
         uint16_t function = 0;
         std::array<condition_parameter_in_situ, 2> parameters;
         condition_event_parameters event_parameters; // used instead of (parameters) for GetEventData
         comparison_data comparison;
         run_on_data     run_on;
         //
      public:
         condition_parameter_type*           get_argument_type(uint8_t index) const noexcept;
         condition_parameter_underlying_type get_argument_underlying_type(uint8_t index) const noexcept;
         
         #pragma region Accessors
         inline uint16_t get_function_id() const noexcept { return this->function; }
         const condition_function* get_function() const noexcept;
         const condition_parameter get_parameter(uint8_t i) const;
         //
         inline const condition_event_parameters& get_event_parameters() const { return this->event_parameters; }
         inline const comparison_data& get_comparison() const noexcept { return this->comparison; }
         inline const run_on_data& get_run_on_data() const noexcept { return this->run_on; }
         //
         inline flags_t get_flags() const noexcept { return this->flags; }
         inline bool test_flags(flags_t f) const noexcept { return (this->flags & f); }
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

         working_condition make_working_copy() const noexcept;
         void commit(loaded_forms::Form& my_owner, working_condition& source);
   };

   class working_condition {
      public:
         using flag    = condition::flag;
         using flags_t = condition::flags_t;
         using operator_type = condition::operator_type;
         using run_on_type   = condition::run_on_type;
         //
         using comparison_data = condition::comparison_data;
         //
         flags_t  flags    = 0;
         uint16_t function = 0;
         std::array<condition_parameter, 2> parameters;
         struct condition_event_parameters {
            uint16_t   function;
            uint16_t   member;
            form_stub* form;
         } event_parameters; // used instead of (parameters) for GetEventData
         struct {
            operator_type op = operator_type::equal;
            struct {
               float      constant;
               form_stub* global = nullptr;
            } operand;
         } comparison;
         struct {
            run_on_type type = run_on_type::subject;
            uint32_t    index = -1;
            form_stub*  reference;
         } run_on;

         condition_parameter_type* get_argument_type(uint8_t index) const noexcept;
         condition_parameter_underlying_type get_argument_underlying_type(uint8_t index) const noexcept;

         bool refers_to_form(const form_stub*) const noexcept;

         void fix_parameter_types(); // ensures that the parameters have the right underlying types (clearing their values if not); generally you'd call this after changing the function
   };

   class condition_list : public std::vector<condition> {
      public:
         using vector::vector;

         void clear() noexcept = delete;
         void clear(loaded_forms::Form& my_owner) noexcept;

         void resize() = delete; // use reserve + push_back/emplace_back

         void append_all_of(loaded_forms::Form& my_owner, const std::vector<condition>& other);
         bool read_next(tes_record_reader&, load_order_interfaces::form_load&);
   };

   struct condition_context {
      //
      // Helper struct for working with conditions.
      //
      form_stub* owner   = nullptr; // the form that contains the conditions
      form_stub* package = nullptr; // (owner) if it's a PACK, or its owning PACK
      form_stub* quest   = nullptr; // (owner) if it's a QUST, or its owning QUST
      struct {
         loaded_form_ptr<loaded_forms::Package> package;
         loaded_form_ptr<loaded_forms::Quest>   quest;
      } loaded;
      bool prefer_working_copy = true;
      //
      condition_context() {}
      condition_context(form_stub&, bool prefer_working_copy = true);
      //
      loaded_forms::Package* get_owning_package() const noexcept; // gets the working copy or, if there isn't one, the form
      loaded_forms::Quest*   get_owning_quest() const noexcept; // gets the working copy or, if there isn't one, the form
   };
}
