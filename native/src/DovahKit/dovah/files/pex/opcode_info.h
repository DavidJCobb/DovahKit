#pragma once
#include <array>
#include "./opcode_type.h"

namespace dovah::pex {
   struct opcode_info {
      const char* name;
      opcode_type id;
      uint8_t     arg_count = 0;
      bool        varargs   = false;
   };

   constexpr const auto all_opcode_info = std::array{
      opcode_info{ "nop",   opcode_type::nop },

      opcode_info{ "i_add", opcode_type::i_add, 3 },
      opcode_info{ "f_add", opcode_type::f_add, 3 },
      opcode_info{ "i_sub", opcode_type::i_sub, 3 },
      opcode_info{ "f_sub", opcode_type::f_sub, 3 },
      opcode_info{ "i_mul", opcode_type::i_mul, 3 },
      opcode_info{ "f_mul", opcode_type::f_mul, 3 },
      opcode_info{ "i_div", opcode_type::i_div, 3 },
      opcode_info{ "f_div", opcode_type::f_div, 3 },
      opcode_info{ "i_mod", opcode_type::i_mod, 3 },
      opcode_info{ "b_not", opcode_type::b_not, 2 },
      opcode_info{ "i_neg", opcode_type::i_neg, 2 },
      opcode_info{ "f_neg", opcode_type::f_neg, 2 },

      opcode_info{ "set",   opcode_type::set,   2 },
      opcode_info{ "cast",  opcode_type::cast,  2 },

      opcode_info{ "cmp_eq",  opcode_type::cmp_eq,  3 },
      opcode_info{ "cmp_lt",  opcode_type::cmp_lt,  3 },
      opcode_info{ "cmp_lte", opcode_type::cmp_lte, 3 },
      opcode_info{ "cmp_gt",  opcode_type::cmp_gt,  3 },
      opcode_info{ "cmp_gte", opcode_type::cmp_gte, 3 },

      opcode_info{ "jmp",   opcode_type::jmp,   1 },
      opcode_info{ "jmp_t", opcode_type::jmp_t, 2 },
      opcode_info{ "jmp_f", opcode_type::jmp_f, 2 },

      opcode_info{
         .name      = "call_method",
         .id        = opcode_type::call_method,
         .arg_count = 3,
         .varargs   = true
      },
      opcode_info{
         .name      = "call_super",
         .id        = opcode_type::call_super,
         .arg_count = 2,
         .varargs   = true
      },
      opcode_info{
         .name      = "call_static",
         .id        = opcode_type::call_static,
         .arg_count = 3,
         .varargs   = true
      },

      opcode_info{ "ret", opcode_type::ret, 1 },

      opcode_info{ "strcat", opcode_type::strcat, 3 },

      opcode_info{ "prop_get", opcode_type::prop_get, 3 },
      opcode_info{ "prop_set", opcode_type::prop_set, 3 },

      opcode_info{ "array_create", opcode_type::array_create, 2 },
      opcode_info{ "array_length", opcode_type::array_length, 2 },
      opcode_info{ "array_get",    opcode_type::array_get,    3 },
      opcode_info{ "array_set",    opcode_type::array_set,    3 },
      opcode_info{ "array_find",   opcode_type::array_find,   4 },
      opcode_info{ "array_rfind",  opcode_type::array_rfind,  4 },
   };

   static_assert(
      []() -> bool {
         auto& list = all_opcode_info;
         for (size_t i = 0; i < list.size(); ++i) {
            for (size_t j = i + 1; j < list.size(); ++j) {
               if (list[i].id == list[j].id)
                  return false;
            }
         }
         return true;
      }(),
      "Opcode info: an `opcode_type` value accidentally appears twice."
   );

   constexpr const opcode_info* info_for_opcode(opcode_type id) {
      constexpr bool list_indexed_by_id = []() -> bool {
         for (size_t i = 0; i < all_opcode_info.size(); ++i)
            if (all_opcode_info[i].id != (opcode_type)i)
               return false;
         return true;
      }();

      if constexpr (list_indexed_by_id) {
         if ((size_t)id < all_opcode_info.size())
            return &all_opcode_info[(size_t)id];
      } else {
         for (const auto& item : all_opcode_info)
            if (item.id == id)
               return &item;
      }
      return nullptr;
   }
}
