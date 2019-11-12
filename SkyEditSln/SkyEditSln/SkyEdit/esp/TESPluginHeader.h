#pragma once
#include <string>
#include <vector>

class TESPluginHeader {
   //
   // Class used to extract load-critical information from a file header. 
   // It is advised that if you need to read only the file header, you 
   // use this instead of TESPluginFile, so that you're not mapping the 
   // entire file into memory just to unmap it a few dozen bytes later.
   //
   public:
      enum Flags {
         kFlag_Master = 0x0001,
         kFlag_LocalizedStringTable = 0x0080,
         kFlag_Light  = 0x0200, // SSE only
      };
      //
      bool load(const char* path) noexcept;
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