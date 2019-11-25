#pragma once
#include "../types.h"

class FormStub;
class TESPluginRecord;

using FormOutboundUsesBuilder = void(*)(TESPluginRecord&, FormStub*);
FormOutboundUsesBuilder getOutboundUsesBuilderForFormType(formtype_t) noexcept; // can return nullptr