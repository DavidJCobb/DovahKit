#pragma once
#include <cstdint>
#include <variant>
#include "../_common.h"
#include "../../data/entry_point_functions.h"

namespace dovah::loaded_forms::structs {
   class perk_entry_point_data {
      public:
         //
         // There is no subrecord for the function ID; that is incorporated into 
         // subrecords unique to the containing form.
         //
         static constexpr const uint32_t subrecord_function_type   = 'EPFT'; // Entry Point Function Type
         static constexpr const uint32_t subrecord_function_data_1 = 'EPFD'; // Entry Point Function Data
         static constexpr const uint32_t subrecord_function_data_2 = 'EPF2'; // Entry Point Function data 2
         static constexpr const uint32_t subrecord_function_data_3 = 'EPF3'; // Entry Point Function data 3

      public:
         struct data_types { // poor man's namespace
            data_types() = delete;

            struct one_float {
               float value = 0;
            };
            struct two_floats {
               float a = 0; // can sometimes be an AV index represented as a float
               float b = 0;
            };
            struct leveled_item {
               form_reference_t form;
            };
            struct activate_choice {
               public:
                  struct flag {
                     flag() = delete;
                     enum type : uint8_t {
                        run_immediately = 1 << 0,
                        replace_default = 1 << 1,
                     };
                  };
                  static constexpr const uint16_t no_fragment = 0xFFFF;

               public:
                  form_reference_t spell;
                  localized_string text;
                  uint8_t          flags = 0;
                  uint16_t         fragment_index = no_fragment;
            };
            struct spell {
               form_reference_t form;
            };
         };

         using data_variant = std::variant<
            std::monostate,
            data_types::one_float,
            data_types::two_floats,
            data_types::leveled_item,
            data_types::activate_choice,
            data_types::spell,
            std::string,
            localized_string
         >;

      protected:
         // Internal function, used during loading. The external `set_type` function is meant 
         // to be used for editing, and will modify Use Info.
         void _set_data_variant_to_type(entry_point_function_type);

      public:
         entry_point_function function = entry_point_function::none;
         data_variant data;

      public:
         bool first_float_is_actor_value() const;

         entry_point_function_type type() const;
         void set_type(loaded_forms::Form& my_owner, entry_point_function_type);

      public: // form management boilerplate
         void load_function_data_type(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, size_t which_effect);
         void load_function_data(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, size_t which_effect);

         void load(tes_subrecord_reader&, load_order_interfaces::form_load& intfc, size_t which_effect);
         void save(tes_record_writer&, load_order_interfaces::form_save& intfc);
         void clone_from(const perk_entry_point_data& original, loaded_forms::Form& owner_of_clone) noexcept;
         void sever_outbound_references_to(form_stub& target, loaded_forms::Form& my_owner) noexcept;
         void clear(loaded_forms::Form& my_owner);
         
         struct use_info_state {
            entry_point_function_type type = entry_point_function_type::none;
            form_id_t form;
               
            void read(tes_subrecord_reader&);
            void commit(form_stub_use_info_builder&);
         };
   };
}