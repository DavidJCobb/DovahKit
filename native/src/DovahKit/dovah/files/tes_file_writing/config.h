#pragma once
#include <cstdint>
#include "../common.h"
#include "../../core.h"

namespace dovah::tes_file_writing {
   enum class record_compression_policy {
      never,     // never compress records
      threshold, // compress records that are larger than a certain size
      bethesda,  // mimic Skyrim.esm: compress NAVM, NPC_, any CELL that has TVDT, and any LAND in a compressed CELL, all regardless of the records' sizes
   };

   struct write_config {
      record_compression_policy record_compression = record_compression_policy::never;
      uint32_t file_flags     = 0;
      uint16_t record_version = 0; // 0 = same as source file
      uint16_t record_compress_threshold = 0x280; // used if the record compression policy is "threshold"
      union {
         struct {
            uint8_t vc_day;
            uint8_t vc_month;
            uint8_t vc_last_editor;
            uint8_t vc_current_editor;
         };
         uint32_t version_control = 0;
      };
      uint16_t version_control_2 = 0;
      game     output_game       = game::skyrim_classic;

      static write_config for_skyrim_classic();
      static write_config for_skyrim_special();
      static write_config for_game(dovah::game);
   };

}