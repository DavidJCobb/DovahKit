#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace nifDK {
   class block;

   struct file_version {
      uint32_t value = 0;

      constexpr file_version() {}
      constexpr file_version(uint32_t v) : value(v) {}

      inline constexpr operator uint32_t() const { return this->value; }

      template<uint8_t major, uint8_t minor, uint8_t c, uint8_t d> static constexpr file_version from_parts = ([]() {
         return ((uint32_t)major << 0x18) | ((uint32_t)minor << 0x10) | ((uint32_t)c << 0x08) | ((uint32_t)d << 0x00);
      })();

      inline std::strong_ordering operator<=>(const file_version& other) const { return this->value <=> other.value; }

      inline constexpr uint8_t major() const { return (this->value >> 0x18); }
      inline constexpr uint8_t minor() const { return (this->value >> 0x10); }
      inline constexpr uint8_t patch() const { return (this->value >> 0x08); }
      inline constexpr uint8_t build() const { return (this->value >> 0x00); }

      static file_version from_string(const std::string&);
   };

   class file {
      public:
         enum class version : uint32_t {
            oblivion_alt_a = file_version::from_parts< 3, 3, 0,  13>, // ?
            morrowind      = file_version::from_parts< 4, 0, 0,   2>,
            oblivion       = file_version::from_parts<10, 0, 1,   2>,
            oblivion_alt_b = file_version::from_parts<10, 1, 0, 101>,
            oblivion_alt_c = file_version::from_parts<10, 1, 0, 106>,
            oblivion_alt_d = file_version::from_parts<20, 0, 0,   5>,
            fallout_3      = file_version::from_parts<20, 2, 0,   7>,
            skyrim         = file_version::from_parts<20, 2, 0,   7>,
         };
         enum class endian : uint8_t {
            big    = 0,
            little = 1,
         };

      //
      // EXPORTINFO
      // 
      // MIN VERSION | DESCRIPTION
      // ------------+---------------------------------------------------------------------------------------------------------------------
      // 10.00.01.02 | uint Unknown
      // --.--.--.-- | ShortString creator
      // --.--.--.-- | ShortString Export Info 1
      // --.--.--.-- | ShortString Export Info 2
      // 
      // 
      // HEADER - https://github.com/niftools/nifxml/blob/f265c56482c728c6877e45d5b5993d3bff83670a/nif.xml#L1259
      // 
      // MIN VERSION | DESCRIPTION
      // ------------+---------------------------------------------------------------------------------------------------------------------
      // --.--.--.-- | "%s File Format, Version %s" ending in "\r\n" or "\n", e.g. "Gamebryo File Format, Version 20.2.0.7\n"
      // 03.01.??.?? | Copyright string, ending in a line break
      // --.--.--.-- | little-endian uint32_t version e.g. 07 00 02 14 i.e. 0x14020007 i.e. 20.2.0.7
      // 20.00.00.04 | EndianType
      // 10.01.00.00 | little-endian uint32_t User Version (for companies that extend the NIF format)
      // 03.03.00.13 | little-endian uint32_t Num Blocks
      // --.--.--.-- | little-endian uint32_t User Version 2 (if (UV >= 10 || UV == 1) && UV != 10.2.0.0)
      // 30.00.00.02 | uint32_t
      // --.--.--.-- | ExportInfo (version 10.0.1.2, or ((version 10.1.0.0) && (UV >= 10 || UV == 1))
      // 10.00.01.00 | uint16_t Num Block Types
      // 10.00.01.00 | SizedString[Num Block Types] Block Types    -- all block types used in the file are listed here
      // 10.00.01.00 | BlockTypeIndex[Num Blocks] Block Type Index -- each block's type is listed here
      // 20.02.00.07 | uint[Num Blocks] -- each block's size?
      // 20.01.00.03 | uint Num Strings
      // 20.01.00.03 | uint Max String Length
      // 20.01.00.03 | SizedString[Num Strings] Strings
      // 10.00.01.00 | uint Unknown
      // 

      public:
         struct {
            std::string  format_name;
            file_version version;
            endian       endianness = endian::little;
            struct {
               uint32_t primary;
               uint32_t secondary = 0;
            } user_versions;
            struct {
               std::string creator;
               std::array<std::string, 2> info;
            } export_data;
         } header;
         std::vector<block*> all_blocks;

         void read(void* data, size_t size);

         inline block* block_by_index(int32_t i) const {
            if (i >= this->all_blocks.size())
               return nullptr;
            return this->all_blocks[i];
         }
   };
}