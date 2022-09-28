#pragma once
#include "bhkSerializable.h"
#include "../types/hkWorldObjCinfoProperty.h"
#include "../types/HavokFilter.h"
#include "./bhkShape.h"

namespace nifDK::block_types {
   class bhkShape;

   class bhkWorldObject : public bhkSerializable {
      public:
         static constexpr const char* const type_name = "bhkWorldObject";
      public:
         enum class broad_phase_type : uint8_t {
            invalid = 0,
            entity  = 1,
            phantom = 2,
            border  = 3,
         };

         bhkShape*   shape = nullptr; // 00
         uint32_t    unk04;  // 04
         HavokFilter filter; // 08
         uint32_t    pad0C;  // 0C
         broad_phase_type broad_phase = broad_phase_type::entity; // 10
         uint8_t     pad11[3];
         hkWorldObjCinfoProperty cinfo; // 14

         virtual void parse(file_reader&) override;
   };
}