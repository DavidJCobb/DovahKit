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

#include "./util/file_string_table.h"
#include "../file_header.h"
#include "../function_debug_info.h"
#include "../script_object.h"
#include "../user_flag_definition.h"

namespace dovah::pex::parsers {
   class function_collector;

   class function_collector : public base<function_collector, char> {
      protected:
         struct string_table_config : public cobb::interned_string_tables::default_parameters {
            static constexpr const bool fold_on_store = false;
            static constexpr bool are_equal(const view_type& a, const view_type& b) {
               return dovah::papyrus::helpers::name_equals(a, b);
            }
         };

      public:
         using retained_string_table_type = cobb::interned_string_table<string_table_config>;
         using string_index_type = uint16_t;

         struct argument_info {
            std::string_view name;
            std::string_view type;
         };
         struct function_info {
            std::string_view name;
            std::string_view return_type;
            std::string_view docstring;
            bool is_global = false;
            bool is_native = false;

            uint16_t arg_count = 0;
            std::vector<argument_info> arg_info;
         };
         struct class_info {
            std::string_view name;
            std::string_view superclass;
            std::vector<function_info> functions;

            constexpr const function_info* lookup_function(std::string_view name) const;
         };

      protected:
         retained_string_table_type& _retained_strings;
         util::file_string_table     _file_string_table;
         
         constexpr void _skip_indexed_string() {
            skip_bytes(sizeof(string_index_type));
         }
         constexpr void _consume_indexed_string(std::string_view& dst) {
            string_index_type index;
            read(index);
            dst = get_tabled_string(index);
         }
         constexpr void _consume_and_retain_indexed_string(std::string_view& dst) {
            _consume_indexed_string(dst);
            _retain_string(dst);
         }
         constexpr void _retain_string(std::string_view& dst) {
            dst = _retained_strings.get_or_insert(dst);
         }

      public:
         constexpr function_collector(retained_string_table_type& ss) : _retained_strings(ss) {}
         
         constexpr const std::string_view get_tabled_string(string_index_type index) const;
         
      public:
         bool        collect_arguments = true;
         std::string desired_classname;
         class_info  results;

         constexpr void read_file(const char* buffer, size_t size);
   };
   static_assert(util::is_valid_stream_class<function_collector>);
}

#include "./function_collector.inl"