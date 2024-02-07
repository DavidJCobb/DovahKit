#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../../helpers/endian.h"

#include "../pex/compiled_function.h"
#include "../pex/compiled_opcode.h"
#include "../pex/function_debug_info.h"
#include "../pex/global_variable_declaration.h"
#include "../pex/property_declaration.h"
#include "../pex/underlying_value_type.h"
#include "../pex/user_flag_definition.h"
#include "../pex/value.h"
#include "../pex/variable_declaration.h"

namespace dovah {
   struct papyrus_assembly_opcode {
      const char* name;
      uint8_t     fixed_arg_count;
      bool        varargs;
      //
      static std::array<papyrus_assembly_opcode, 36> list;
   };

   class compiled_papyrus_script {
      public:
         class read_exception : public std::runtime_error {
            public:
               const uint32_t offset;
               //
               explicit read_exception(uint32_t o) : runtime_error(""), offset(o) {}
         };
         class not_a_papyrus_file_exception : public read_exception {
            public:
               explicit not_a_papyrus_file_exception(uint32_t o) : read_exception(o) {}
         };
         class invalid_opcode_exception : public read_exception {
            public:
               const uint8_t opcode;
               //
               explicit invalid_opcode_exception(uint32_t o, uint8_t op) : read_exception(o), opcode(op) {}
         };
         class unexpected_eof_exception : public read_exception {
            public:
               const size_t desired_size;
               //
               explicit unexpected_eof_exception(uint32_t o, size_t s) : read_exception(o), desired_size(s) {}
         };
         class varargs_count_type_exception : public read_exception {
            public:
               const uint8_t type;
               //
               explicit varargs_count_type_exception(uint32_t o, uint8_t t) : read_exception(o), type(t) {}
         };
         //
      public:
         using function_type  = dovah::pex::function_debug_info::function_type;
         using raw_type       = dovah::pex::underlying_value_type;
         //
         using debug_function = dovah::pex::function_debug_info;
         using user_flag      = dovah::pex::user_flag_definition;
         //
         using variable = dovah::pex::variable_declaration;
         using value    = dovah::pex::value;
         using global_variable = dovah::pex::global_variable_declaration;
         //
         using instruction    = dovah::pex::compiled_opcode;
         using function       = dovah::pex::compiled_function;
         using named_function = dovah::pex::compiled_named_function;
         //
         using property = dovah::pex::property_declaration;
         struct state {
            std::string name; // empty string for default state
            std::vector<named_function> functions;
         };
         struct object {
            std::string name;
            std::string superclass;
            std::string docstring;
            uint32_t    user_flags; // user_flags
            std::string auto_state_name;
            std::vector<global_variable> variables;
            std::vector<property> properties;
            std::vector<state> states;

            const state* get_auto_state() const noexcept;
         };
         //
      protected:
         struct {
            const void* _buffer = nullptr;
            uint32_t    _pos    = 0;
            size_t      _size   = 0;
         } file;
         bool _needs_endian_swap = false;
         std::vector<std::string> _string_table;
         //
         void _read(void* to, size_t);
         void _read(std::string&);
         void _read_string_index(std::string&);
         template<typename T> inline void _read(T& v) {
            this->_read(&v, sizeof(T));
            if constexpr (sizeof(T) > 1) {
               if (this->_needs_endian_swap)
                  v = cobb::byteswap(v);
            }
         }
         //
         void _read(debug_function&); // all of these functions can throw exceptions
         void _read(user_flag&);
         void _read(variable&);
         void _read(value&);
         void _read(global_variable&);
         void _read(instruction&);
         void _read(function&);
         void _read(named_function&);
         void _read(property&);
         void _read(state&);
         void _read(object&);
         //
      public:
         struct {
            uint8_t major = 3;
            uint8_t minor = 2;
         } version;
         uint16_t game_id = 1;
         //
         uint64_t     compile_time;
         std::string source_file;
         struct {
            std::string username;
            std::string computer;
         } author;
         struct {
            bool     present = false;
            uint64_t modification_time;
            std::vector<debug_function> functions;
         } debug;
         std::vector<user_flag> user_flags;
         std::vector<object> objects;
         
         void read_file(const void* buffer, size_t size);

         const object* lookup_object(const std::string&) const;
   };
}
