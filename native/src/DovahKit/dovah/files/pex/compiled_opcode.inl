#pragma once
#include "./compiled_opcode.h"
#include "./opcode_info.h"
#include "./value.h"

#include "./parsers/exceptions/bad_opcode_id.h"
#include "./parsers/exceptions/bad_opcode_varargs_count.h"

namespace dovah::pex {
   template<typename Stream>
   constexpr void compiled_opcode::read(Stream& stream) {
      stream.read(this->type);
      //
      const auto* info = pex::info_for_opcode(this->type);
      if (!info)
         throw exceptions::bad_opcode_id(stream, (uint8_t)this->type);
      
      this->operands.resize(info->arg_count);
      for (auto& o : this->operands)
         stream.read(o);
      //
      if (info->varargs) {
         value c;
         stream.read(c);
         if (!c.is_of_type(pex::underlying_value_type::integer))
            throw exceptions::bad_opcode_varargs_count(stream, (uint8_t)c.type);
         //
         auto size = info->arg_count + std::get<int32_t>(c.content);
         this->operands.resize(size);
         for (uint8_t i = info->arg_count; i < size; ++i)
            stream.read(this->operands[i]);
      }
   }
   template<typename Stream>
   /*static*/ constexpr void compiled_opcode::skip(Stream& stream) {
      opcode_type type;
      stream.read(type);
      //
      const auto* info = pex::info_for_opcode(type);
      if (!info)
         throw exceptions::bad_opcode_id(stream, (uint8_t)type);

      for (size_t i = 0; i < info->arg_count; ++i)
         value::skip(stream);

      if (!info->varargs)
         return;

      int32_t vararg_count = 0;
      {
         underlying_value_type type;
         stream.read(type);
         if (type != underlying_value_type::integer)
            throw exceptions::bad_opcode_varargs_count(stream, (uint8_t)type);
         stream.read(vararg_count);
      }
      for (size_t i = 0; i < vararg_count; ++i)
         value::skip(stream);
   }
}
