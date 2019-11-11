#pragma once
#include <string>
#include <vector>

class TESPluginHeader {
   //
   // Class used to extract load-critical information from a file header.
   //
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200, // SSE only
      };
      //
      bool load(const char* path);
      //
      std::string name;
      uint32_t    flags = 0;
      std::string authorName;
      std::string description;
      std::vector<std::string> masters;
      //
      bool is_master() const noexcept {
         return (this->flags & kFlag_Master) != 0;
      }
      
};