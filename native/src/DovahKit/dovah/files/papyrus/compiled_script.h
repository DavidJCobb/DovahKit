#pragma once
#include <array>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>
#include "../../../helpers/endian.h"

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
         enum class function_type : uint8_t {
            normal = 0,
            getter = 1,
            setter = 2,
            // UESP says that "3" can appear here, too?
         };
         enum class raw_type : uint8_t {
            none    = 0,
            object  = 1,
            string  = 2,
            integer = 3,
            float32 = 4,
            boolean = 5,
         };
         //
         struct debug_function {
            std::string   object;
            std::string   state;
            std::string   name; // function name
            function_type type; // valid values range from 0 to 3
            std::vector<uint16_t> line_numbers; // maps instructions in the function to their original line numbers in the source code
         };
         struct user_flag {
            //
            // Flags like "Hidden" and "Conditional" don't have fixed bits. Instead, they're 
            // encoded into the script's "user_flags" section and given bits there. Then, each 
            // object that can have these flags will have a flags-mask whose bits should match 
            // the bits given to user-flags in the header.
            //
            std::string name;
            uint8_t bit_index;
         };
         //
         struct variable { // "Variable Type" on UESP
            std::string name;
            std::string type;
         };
         struct value { // "Variable Data" on UESP
            raw_type underlying_type;
            union {
               int32_t i = 0;
               float   f;
               uint8_t b;
            };
            std::string s;
         };
         struct global_variable { // "Variable" on UESP
            std::string name;
            std::string type;
            uint32_t    flags; // user_flags
            value       value;
         };
         //
         struct instruction {
            uint8_t opcode;
            std::vector<value> operands;
         };
         struct function {
            struct flag {
               flag() = delete;
               enum {
                  global = 0x01,
                  native = 0x02,
               };
            };
            //
            std::string return_type;
            std::string docstring;
            uint32_t    flags; // user_flags
            uint8_t     function_flags; // function::flag
            std::vector<variable> arguments;
            std::vector<variable> locals;
            std::vector<instruction> instructions;
         };
         struct named_function : function {
            std::string name;
         };
         //
         struct property {
            struct flag {
               flag() = delete;
               enum {
                  read    = 0x01,
                  write   = 0x02,
                  autovar = 0x04,
               };
            };
            //
            std::string name;
            std::string type;
            std::string docstring;
            uint32_t    flags; // user_flags
            uint8_t     property_flags; // property::flag
            std::string autovar_name;
            function    getter; // if (read)  flag and no (autovar) flag
            function    setter; // if (write) flag and no (autovar) flag
         };
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
         //
         void read_file(const void* buffer, size_t size);
   };
}
