#pragma once
#include <bit>
#include <cstdint>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
#include "helpers/interned_string_table.h"
#include "dovah/data/papyrus/helpers/name_equals.h"
#include "./_util.h"
#include "./base.h"

#include "../file_header.h"
#include "../function_debug_info.h"
#include "../script_object.h"
#include "../user_flag_definition.h"

namespace dovah::pex::parsers {
   class class_info_collector;

   class class_info_collector : public base<class_info_collector, char> {
      protected:
         struct string_table_config : public cobb::interned_string_tables::default_parameters {
            static constexpr const bool fold_on_store = false;
            static constexpr bool are_equal(const view_type& a, const view_type& b) {
               return dovah::papyrus::helpers::name_equals(a, b);
            }
         };

      public:
         using shared_string_table_type = cobb::interned_string_table<string_table_config>;

         struct class_info {
            std::string_view name;
            std::string_view superclass;
            std::string_view docstring;
            struct {
               bool conditional = false;
               bool hidden      = false;
            } flags;
         };

      protected:
         shared_string_table_type& _shared_strings;
         std::string_view _file_string_table;
         struct {
            uint8_t conditional = 0xFF;
            uint8_t hidden      = 0xFF;
         } user_flags;

         constexpr void _consume_indexed_string(std::string_view& dst) {
            uint16_t index;
            read(index);
            dst = get_tabled_string(index);
         }

      public:
         constexpr class_info_collector(shared_string_table_type& ss) : _shared_strings(ss) {}
         
         constexpr const std::string_view get_tabled_string(uint16_t index) const;
         
      public:
         std::string desired_classname;
         class_info  results;

         constexpr void read_file(const char* buffer, size_t size);
   };
   static_assert(util::is_valid_stream_class<class_info_collector>);
}

#include "./class_info_collector.inl"