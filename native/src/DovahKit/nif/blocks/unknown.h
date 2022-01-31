#pragma once
#include <cstdint>
#include <vector>
#include "../block.h"

namespace nifDK::block_types {
   class unknown_block : public block {
      public:
         std::string type_name;
         std::vector<uint8_t> data;

         virtual void parse(file_reader&) override;
   };
}