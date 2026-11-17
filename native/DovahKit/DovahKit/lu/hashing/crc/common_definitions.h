#pragma once
#include "./definition.h"

namespace lu::hashing::crc::definitions {
   #pragma region CRC-3 variations
      constexpr const rocksoft_definition crc3_gsm = {
         .bitcount                = 3,
         .poly                    = 3,
         .initial                 = 0,
         .reflect_input_bytes     = false,
         .reflect_output_register = false,
         .xor_output              = 7,
         .check                   = 4,
      };
   #pragma endregion
   #pragma region CRC-16 variations
      constexpr const rocksoft_definition crc16_usb = {
         .bitcount                = 16,
         .poly                    = 0x8005,
         .initial                 = 0xFFFF,
         .reflect_input_bytes     = true,
         .reflect_output_register = true,
         .xor_output              = 0xFFFF,
         .check                   = 0xB4C8,
      };
   #pragma endregion
   #pragma region CRC-32 variations
      // The CRC most commonly referred to as "CRC-32," used for a variety of standards including 
      // IEEE 802.3 and IEEE 802.11.
      constexpr const rocksoft_definition crc32_common = {
         .bitcount                = 32,
         .poly                    = 0x04C11DB7,
         .initial                 = 0xFFFFFFFF,
         .reflect_input_bytes     = true,
         .reflect_output_register = true,
         .xor_output              = 0xFFFFFFFF,
         .check                   = 0xCBF43926,
      };
   
      // The CRC-32 hash used by Bethesda Game Studios, as of Skyrim.
      constexpr const rocksoft_definition crc32_bethesda = {
         .bitcount                = 32,
         .poly                    = 0x04C11DB7,
         .initial                 = 0,
         .reflect_input_bytes     = true,
         .reflect_output_register = true,
         .xor_output              = 0,
         .check                   = 0x2DFD2D88,
      };
   
      // Used by iSCSI, NVMe, Btrfs, ext4, et cetera.
      constexpr const rocksoft_definition crc32_castagnoli = {
         .bitcount                = 32,
         .poly                    = 0x1EDC6F41,
         .initial                 = 0xFFFFFFFF,
         .reflect_input_bytes     = true,
         .reflect_output_register = true,
         .xor_output              = 0xFFFFFFFF,
         .check                   = 0xE3069283,
      };
      
      constexpr const rocksoft_definition crc32_jamcrc = {
         .bitcount                = 32,
         .poly                    = 0x04C11DB7,
         .initial                 = 0xFFFFFFFF,
         .reflect_input_bytes     = true,
         .reflect_output_register = true,
         .xor_output              = 0,
         .check                   = 0x340BC6D9,
      };
      
      constexpr const rocksoft_definition crc32_mpeg2 = {
         .bitcount                = 32,
         .poly                    = 0x04C11DB7,
         .initial                 = 0xFFFFFFFF,
         .reflect_input_bytes     = false,
         .reflect_output_register = false,
         .xor_output              = 0,
         .check                   = 0x0376E6E7,
      };
      
      constexpr const rocksoft_definition crc32_posix = {
         .bitcount                = 32,
         .poly                    = 0x04C11DB7,
         .initial                 = 0,
         .reflect_input_bytes     = false,
         .reflect_output_register = false,
         .xor_output              = 0xFFFFFFFF,
         .check                   = 0x765E7680,
      };
   #pragma endregion
}