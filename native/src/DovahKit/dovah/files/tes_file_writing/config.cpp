#include "config.h"

namespace dovah::tes_file_writing {
   /*static*/ write_config write_config::for_skyrim_classic() {
      write_config out;
      out.output_game    = game::skyrim_classic;
      out.record_version = 43;
      return out;
   }
   /*static*/ write_config write_config::for_skyrim_special() {
      write_config out;
      out.output_game    = game::skyrim_special;
      out.record_version = 44;
      return out;
   }
   /*static*/ write_config write_config::for_game(dovah::game g) {
      switch (g) {
         case game::skyrim_classic: return for_skyrim_classic();
         case game::skyrim_special: return for_skyrim_special();
      }
      return write_config();
   }
}