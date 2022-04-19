#pragma once
#include <optional>
#include <string>
#include <type_traits>
#include <vector>
#include "blocks/_DKVulkanInterface.h"
#include "types/NiTransform.h"

namespace nifDK {
   class file;
   class file_reader;

   class block : public virtual block_interfaces::_DKVulkanInterface {
      public:
         virtual ~block() {}
      public:
         virtual void parse(file_reader&) = 0;

         file* owner = nullptr;
   };

   namespace block_types {
      class unknown_block;
   }
}