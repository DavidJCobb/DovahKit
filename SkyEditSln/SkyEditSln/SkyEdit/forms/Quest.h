#pragma once
#include <cstdint>
#include <string>
#include "types.h"

class esp_istream;

class TESQuest : public TESForm {
   public:
      TESQuest() : TESForm(77) {};

      std::string editorID;
      std::string name;

      void load(esp_istream&);
};