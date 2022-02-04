#pragma once
#include "../file.h"
#include "NiObject.h"

namespace nifDK::block_types {
   class NiExtraData : public NiObject {
      public:
         static constexpr const char* const type_name = "NiExtraData";
      public:
         static constexpr auto max_version_for_linked_list = file_version::from_parts<4, 2, 2, 0>;
      public:
         std::string  name;
         NiExtraData* next = nullptr; // old; only used while loading

         virtual void parse(file_reader&) override;
   };
}