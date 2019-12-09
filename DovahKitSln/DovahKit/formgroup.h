#pragma once
#include <cstdint>
#include "esp/base.h"

class FormStub;

struct CellGridCoords {
   int16_t y;
   int16_t x;
};
class FormGroup {
   public:
      ESPGroupType   type   = ESPGroupType::forms_of_type;
      FormStub*      parent = nullptr;
      uint32_t       offset = 0;
      uint32_t       recordSignature = 0;
      CellGridCoords gridCoords;
};
