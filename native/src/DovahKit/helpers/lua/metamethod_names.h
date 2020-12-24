#pragma once
#include <array>

namespace cobb::lua {
   constexpr std::array metamethod_names = {
      "__add",
      "__band",
      "__bnot",
      "__bor",
      "__bxor",
      "__call",
      "__close",
      "__concat",
      "__div",
      "__eq",
      "__gc",
      "__idiv",
      "__index",
      "__le",
      "__len",
      "__lt",
      "__metatable",
      "__mod",
      "__mode",
      "__mul",
      "__name",
      "__newindex",
      "__pairs",
      "__pow",
      "__shl",
      "__shr",
      "__sub",
      "__tostring",
      "__unm",
   };
   extern bool is_metamethod_name(const char*);
}