#pragma once
#include <cstdint>
#include <string>
#include "types.h"
#include "components.h"
#include "papyrus.h"

class TESPluginFile;

class TESActorBase : public TESForm {
   //
   // Intentionally minimal for now.
   //
   public:
      TESActorBase() : TESForm(kFormType_ActorBase) {};

      std::string editorID;
      LStringRef  name;

      void load(TESPluginFile*);
};