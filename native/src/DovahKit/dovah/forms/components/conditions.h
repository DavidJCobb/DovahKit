#pragma once
#include "../_common.h"
#include "conditions/arg_value.h"
#include "conditions/arg_types.h"

namespace dovah::loaded_forms::components {
   namespace condition_info {
      class function {
         protected:
            enum class _sentinel_is_event {}; // dummy class, for constructor args
            enum class _sentinel_is_dummy {};
            //
         public:
            static constexpr _sentinel_is_event function_uses_event_data = _sentinel_is_event();
            static constexpr _sentinel_is_dummy dummy = _sentinel_is_dummy();
            //
            uint16_t    id = 0xFFFF;
            const char* name = "";
            const char* description = "";
            const bool  valid = true;
            const bool  uses_event_data = false;
            arg_type* const argument_types[2] = { &arg_types::None, &arg_types::None }; // aRrAy Of ReFeReNcE iS nOt AlLoWeD
            //
            function() {}; // needed for std::array, apparently
            function(uint16_t id, const char* name, const char* d) : id(id), name(name), description(d) {};
            function(uint16_t id, const char* name, const char* d, arg_type& a) : id(id), name(name), description(d), argument_types{ &a, &arg_types::None } {};
            function(uint16_t id, const char* name, const char* d, arg_type& a, arg_type& b) : id(id), name(name), description(d), argument_types{ &a, &b } {};
            //
            function(uint16_t id, const char* name, const char* d, _sentinel_is_event) : id(id), name(name), description(d), uses_event_data(true) {};
            //
            function(uint16_t id, _sentinel_is_dummy) : valid(false), id(id), name("Invalid Condition Function"), description("This condition ID is not valid.") {}

            static const function* lookup_by_id(uint16_t) noexcept;
      };

      extern std::array<function, 736> function_list;
      extern std::array<function, 5>   extended_function_list; // SKSE additions
   }

   struct condition {
      enum class run_on_t {
         subject       = 0,
         target        = 1,
         reference     = 2, // i.e. condition::run_on_reference
         combat_target = 3,
         linked_ref    = 4,
         quest_alias   = 5,
         package_data  = 6, // where does (condition) store *which* packdata we're running on?
         event_data    = 7,
      };
      enum class operator_t {
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
            use_aliases       = 0x02,
            compare_to_global = 0x04,
            use_packdata      = 0x08,
            swap_subject_and_target = 0x10,
         };
      };

      uint8_t   type; // flags | (operator << 5)
      float     compare_to_constant;
      form_reference_t compare_to_global;
      uint16_t  function;
      condition_arg_value parameters[2];
      run_on_t  run_on;
      form_reference_t run_on_reference;
      int32_t   run_on_index; // xEdit calls this "Parameter 3." If (run_on == run_on_t::package_data), then this is the Package Data index (within the PACK containing this condition) to run on, and -1 means "NONE."
      //
      uint16_t  eventFunction;
      uint16_t  eventMember;
      form_reference_t eventFormID;

      condition_info::arg_type*           get_argument_type(uint8_t index) const noexcept;
      condition_info::arg_underlying_type get_argument_underlying_type(uint8_t index) const noexcept;
      //
      inline operator_t get_operator() const noexcept { return (operator_t)((this->type >> 5) & 7); }
      inline uint8_t    get_flags()    const noexcept { return this->type & 0x1F; }
      inline void set_flags(uint8_t f) noexcept { this->type = (f & 0x1F) | (this->type & ~0x1F); }
      inline void set_operator(operator_t op) noexcept { this->type = (this->type & 0x1F) | (((uint8_t)op & 7) << 5); }
      inline void set_flags_and_operator(uint8_t f, operator_t op) noexcept { this->type = (f & 0x1F) | ((uint8_t)op & 7) << 5; }
      //
      bool read(tes_record_reader&, load_order_interfaces::form_load&); // assumes we've already opened a CTDA subrecord
      static void generate_use_info(tes_record_reader&, form_stub_use_info_builder&);
      void save(tes_record_writer&, load_order_interfaces::form_save&); // call with no subrecord open
      void clone_from(const condition& original, form_stub& owner_of_clone) noexcept;
      void sever_outbound_references_to(form_stub& target, form_stub& my_owner) noexcept;
      //
      void to_string(std::string& out) const;
   };
}
