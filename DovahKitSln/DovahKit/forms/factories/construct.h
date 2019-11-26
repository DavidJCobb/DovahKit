#pragma once
#include "../types.h"

class TESPluginRecord;

namespace LoadedForms {
   class Form;
}

using LoadedFormFactory = LoadedForms::Form*(*)(TESPluginRecord&);
LoadedFormFactory getLoadedFormFactoryForFormType(formtype_t) noexcept; // can return nullptr