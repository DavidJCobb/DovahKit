#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace dovah {
   class compiled_papyrus_script {
      public:
         struct debug_function {
            std::wstring object;
            std::wstring state;
            std::wstring name; // function name
            uint8_t type;
            std::vector<uint16_t> line_numbers;
         };
         //
         struct variable { // "Variable Type" on UESP
            std::wstring name;
            std::wstring type;
         };
         struct value { // "Variable Data" on UESP
            uint8_t underlying_type;
            union {
               int32_t i = 0;
               float   f;
               uint8_t b;
            };
            std::wstring s;
         };
         struct global_variable { // "Variable" on UESP
            std::wstring name;
            std::wstring type;
            uint32_t     flags;
            value        value;
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
            std::wstring return_type;
            std::wstring docstring;
            uint32_t     flags;
            uint32_t     function_flags; // function::flag
            std::vector<variable> arguments;
            std::vector<variable> locals;
            std::vector<instruction> instructions;
         };
         struct named_function : function {
            std::wstring name;
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
            std::wstring name;
            std::wstring type;
            std::wstring docstring;
            uint32_t     flags;
            uint8_t      property_flags; // property::flag
            std::wstring autovar_name;
            function     getter; // if (read)  flag and no (autovar) flag
            function     setter; // if (write) flag and no (autovar) flag
         };
         struct state {
            std::wstring name; // empty string for default state
            std::vector<named_function> functions;
         };
         struct object {
            std::wstring name;
            std::wstring superclass;
            std::wstring docstring;
            std::wstring auto_state_name;
            std::vector<variable> variables;
            std::vector<property> properties;
            std::vector<state> states;
         };
         //
      protected:
         struct {
            const void* _buffer = nullptr;
            uint32_t    _pos    = 0;
         } file;
         //
         void read(void* to, size_t);
         template<typename T> inline void read(T& v) {
            this->read(&v, sizeof(T));
         }
         //
         void read(debug_function&);
         void read(variable&);
         void read(value&);
         void read(global_variable&);
         void read(instruction&);
         void read(function&);
         void read(named_function&);
         void read(property&);
         void read(state&);
         void read(object&);
         //
      public:
         struct {
            uint8_t major = 3;
            uint8_t minor = 2;
         } version;
         uint16_t game_id = 1;
         //
         std::wstring source_file;
         struct {
            std::wstring username;
            std::wstring computer;
         } author;
   };
}
