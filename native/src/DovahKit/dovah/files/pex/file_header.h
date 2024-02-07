#pragma once
#include <cstdint>
#include <string>
#include "./game_id.h"

namespace dovah::pex {
   struct file_header {
      struct {
         uint8_t major = 3;
         uint8_t minor = 2;
      } version;
      game_id game = (game_id)0;
      //
      uint64_t    compile_time;
      std::string source_file;
      struct {
         std::string username;
         std::string computer;
      } author;

      template<typename Stream>
      constexpr void read(Stream&);
      //
      template<typename Stream>
      static constexpr void skip(Stream&);
   };
}

#include "./file_header.inl"