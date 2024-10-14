#pragma once
#include <vector>
#include "NiObject.h"
#include "../types/Key.h"
#include "../types/KeyGroup.h"

namespace nifDK::block_types {
   class NiPSysEmitterCtlrData : public NiObject { // "NiObject with name, extra data, and time controller"
      public:
         static constexpr const char* const type_name = "NiPSysEmitterCtlrData";
      public:
         KeyGroup<float> birth_rate_keys;
         std::vector<nifDK::Key<uint8_t>> active_keys;

         virtual void parse(file_reader&) override;
   };
}