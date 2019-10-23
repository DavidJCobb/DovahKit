#pragma once
#include <cstdint>
#include <string>
#include "types.h"
#include "components.h"
#include "papyrus.h"

class TESPluginRecord;

class TESActorBase : public TESForm {
   //
   // Intentionally minimal for now.
   //
   public:
      TESActorBase() : TESForm(FormType::ActorBase) {};

      std::string editorID;
      LStringRef  name;

      void load(TESPluginRecord&);
};