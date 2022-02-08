#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>
#include "detailed_notice.h"

namespace nifDK {
   class block;
   class file_reader;
   namespace block_types {
      class NiNode;
   }

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

      void read(file_reader&);
      void unchecked_read(file_reader&);
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

      public:
         file() {}
         ~file();

      protected:
         struct {
            detailed_notice error;
         } results;
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
         //
         std::vector<block*>  all_blocks;
         block_types::NiNode* root_node = nullptr;

         void read(void* data, size_t size);

         inline const detailed_notice& read_error() const { return this->results.error; }

         inline block* block_by_index(int32_t i) const {
            if (i >= this->all_blocks.size())
               return nullptr;
            return this->all_blocks[i];
         }
   };
}