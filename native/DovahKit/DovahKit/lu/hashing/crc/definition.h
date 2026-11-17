#pragma once
#include <cstddef> // size_t
#include <cstdint>

namespace lu::hashing::crc {
   // A definition that describes the parameters for a CRC algorithm using the Rocksoft Model 
   // proposed by Ross Williams ("A Painless Guide to CRC Error Detection Algorithms," 1993). 
   // This is meant to be passed as a template parameter to various CRC-related code, so we 
   // can use one-size-fits-all field widths here.
   // 
   // For information on the Rocksoft Model, refer to Williams' paper:
   //  - <http://www.ross.net/crc/crcpaper.html>
   //  - <http://www.ross.net/crc/download/crc_v3.txt>
   struct rocksoft_definition {
      uint8_t bitcount;

      // The polynomial.
      size_t  poly;

      // The initial value of the register used to compute the hash.
      size_t  initial;

      // Whether the order of bits within individual input bytes is inverted, such that 
      // the most-significant bit is processed first.
      bool    reflect_input_bytes;

      // Whether the order of bits within the output register is swapped.
      bool    reflect_output_register;

      // A value that the output register is XOR'd with (after `reflect_output_register` 
      // is applied, if applicable).
      size_t  xor_output;

      // The result of hashing the ASCII string "123456789" (i.e. the byte sequence of 
      // 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39).
      size_t  check;
   };
}
