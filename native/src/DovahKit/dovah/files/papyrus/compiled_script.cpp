#include "compiled_script.h"
#include <cassert>
#include <stdexcept>

namespace dovah {
   std::array<papyrus_assembly_opcode, 36> papyrus_assembly_opcode::list = {{
      {"nop",  0, false},
      {"iadd", 3, false},
      {"fadd", 3, false},
      {"isub", 3, false},
      {"fsub", 3, false},
      {"imul", 3, false},
      {"fmul", 3, false},
      {"idiv", 3, false},
      {"fdiv", 3, false},
      {"imod", 3, false},
      {"not",  2, false},
      {"ineg", 2, false},
      {"fneg", 2, false},
      {"set",  2, false},
      {"cast", 2, false},
      {"cmp_eq",  3, false},
      {"cmp_lt",  3, false},
      {"cmp_lte", 3, false},
      {"cmp_gt",  3, false},
      {"cmp_gte", 3, false},
      {"jmp",  1, false},
      {"jmpt", 2, false},
      {"jmpf", 2, false},
      {"call_method", 3, true},
      {"call_super",  2, true},
      {"call_static", 3, true},
      {"retn", 1, false},
      {"strcat",   3, false},
      {"prop_get", 3, false},
      {"prop_set", 3, false},
      {"array_create", 2, false},
      {"array_length", 2, false},
      {"array_get",    3, false},
      {"array_set",    3, false},
      {"array_find",   4, false},
      {"array_rfind",  4, false},
   }};

   void compiled_papyrus_script::_read(void* to, size_t s) {
      assert(to);
      if (s + this->file._pos > this->file._size)
         throw unexpected_eof_exception(this->file._pos, s);
      const void* target = (void*)((std::intptr_t)this->file._buffer + this->file._pos);
      memcpy(to, target, s);
      this->file._pos += s;
   }
   void compiled_papyrus_script::_read(std::string& s) {
      uint16_t size;
      this->_read(size);
      s.resize(size);
      for (auto& c : s)
         this->_read(c);
   }
   void compiled_papyrus_script::_read_string_index(std::string& s) {
      uint16_t si = 0;
      this->_read(si);
      s.clear();
      if (si < this->_string_table.size())
         s = this->_string_table[si];
   }

   void compiled_papyrus_script::_read(debug_function& data) {
      this->_read_string_index(data.object);
      this->_read_string_index(data.state);
      this->_read_string_index(data.name);
      this->_read(data.type);
      //
      uint16_t count;
      this->_read(count);
      data.line_numbers.resize(count);
      for (uint16_t i = 0; i < count; ++i)
         this->_read(data.line_numbers[i]);
   }
   void compiled_papyrus_script::_read(user_flag& data) {
      this->_read_string_index(data.name);
      this->_read(data.bit_index);
   }
   void compiled_papyrus_script::_read(variable& data) {
      this->_read_string_index(data.name);
      this->_read_string_index(data.type);
   }
   void compiled_papyrus_script::_read(value& data) {
      this->_read(data.underlying_type);
      switch (data.underlying_type) {
         case raw_type::object:
         case raw_type::string:
            this->_read_string_index(data.s);
            break;
         case raw_type::integer:
            this->_read(data.i);
            break;
         case raw_type::float32:
            this->_read(data.f);
            break;
         case raw_type::boolean:
            this->_read(data.b);
            break;
      }
   }
   void compiled_papyrus_script::_read(global_variable& data) {
      this->_read_string_index(data.name);
      this->_read_string_index(data.type);
      this->_read(data.flags);
      this->_read(data.value);
   }
   void compiled_papyrus_script::_read(instruction& data) {
      auto pos = this->file._pos;
      this->_read(data.opcode);
      if (data.opcode >= papyrus_assembly_opcode::list.size())
         throw invalid_opcode_exception(pos, data.opcode);
      auto& definition = papyrus_assembly_opcode::list[data.opcode];
      //
      data.operands.resize(definition.fixed_arg_count);
      for (auto& o : data.operands)
         this->_read(o);
      //
      if (definition.varargs) {
         pos = this->file._pos;
         value c;
         this->_read(c);
         if (c.underlying_type != raw_type::integer)
            throw varargs_count_type_exception(pos, (uint8_t)c.underlying_type);
         #if _DEBUG
            if (c.i > 10)
               __debugbreak(); // suspicious vararg count
         #endif
         //
         auto size = definition.fixed_arg_count + c.i;
         data.operands.resize(size);
         for (uint8_t i = definition.fixed_arg_count; i < size; ++i)
            this->_read(data.operands[i]);
      }
   }
   void compiled_papyrus_script::_read(function& data) {
      this->_read_string_index(data.return_type);
      this->_read_string_index(data.docstring);
      this->_read(data.flags);
      this->_read(data.function_flags);
      //
      uint16_t count;
      //
      this->_read(count);
      data.arguments.resize(count);
      for (auto& e : data.arguments)
         this->_read(e);
      //
      this->_read(count);
      data.locals.resize(count);
      for (auto& e : data.locals)
         this->_read(e);
      //
      this->_read(count);
      data.instructions.resize(count);
      for (auto& e : data.instructions)
         this->_read(e);
   }
   void compiled_papyrus_script::_read(named_function& data) {
      this->_read_string_index(data.name);
      //
      function* cast = &data;
      this->_read(*cast);
   }
   void compiled_papyrus_script::_read(property& data) {
      this->_read_string_index(data.name);
      this->_read_string_index(data.type);
      this->_read_string_index(data.docstring);
      this->_read(data.flags);
      this->_read(data.property_flags);
      if (data.property_flags & 4)
         this->_read_string_index(data.autovar_name);
      if ((data.property_flags & 5) == 1)
         this->_read(data.getter);
      if ((data.property_flags & 6) == 2)
         this->_read(data.setter);
   }
   void compiled_papyrus_script::_read(state& data) {
      this->_read_string_index(data.name);
      //
      uint16_t count;
      this->_read(count);
      data.functions.resize(count);
      for (uint16_t i = 0; i < count; ++i)
         this->_read(data.functions[i]);
   }
   void compiled_papyrus_script::_read(object& data) {
      this->_read_string_index(data.name);
      //
      uint32_t base = this->file._pos;
      uint32_t size;
      this->_read(size);
      //
      this->_read_string_index(data.superclass);
      this->_read_string_index(data.docstring);
      this->_read(data.user_flags);
      this->_read_string_index(data.auto_state_name);
      //
      uint16_t count;
      //
      this->_read(count);
      data.variables.resize(count);
      for (auto& e : data.variables)
         this->_read(e);
      //
      this->_read(count);
      data.properties.resize(count);
      for (auto& e : data.properties)
         this->_read(e);
      //
      this->_read(count);
      data.states.resize(count);
      for (auto& e : data.states)
         this->_read(e);
      //
      // Done.
      //
      if (this->file._pos != base + size) {
         // TODO: warn?
      }
   }

   void compiled_papyrus_script::read_file(const void* buffer, size_t size) {
      this->file._pos    = 0;
      this->file._buffer = buffer;
      this->file._size   = size;
      //
      uint32_t magic;
      this->_read(magic);
      if (magic != 0xFA57C0DE) {
         if (magic != _byteswap_ulong(0xFA57C0DE))
            throw not_a_papyrus_file_exception(0);
         this->_needs_endian_swap = true;
      }
      this->_read(this->version.major);
      this->_read(this->version.minor);
      this->_read(this->game_id);
      this->_read(this->compile_time);
      this->_read(this->source_file);
      this->_read(this->author.username);
      this->_read(this->author.computer);
      //
      // String table:
      //
      uint16_t count;
      this->_read(count);
      this->_string_table.resize(count);
      for (auto& s : this->_string_table)
         this->_read(s);
      //
      this->_read(this->debug.present);
      if (this->debug.present) {
         this->_read(this->debug.modification_time);
         this->_read(count);
         this->debug.functions.resize(count);
         for (auto& f : this->debug.functions)
            this->_read(f);
      }
      //
      this->_read(count);
      this->user_flags.resize(count);
      for (auto& u : this->user_flags)
         this->_read(u);
      //
      this->_read(count);
      this->objects.resize(count);
      for (auto& o : this->objects)
         this->_read(o);
   }
}