#pragma once
#include "../dovah/form_stub.h"

// This file allows the use of form stub pointers in QVariants at any point after DovahKitCore has 
// been instantiated.

// IntelliSense doesn't like Q_DECLARE_METATYPE; ignore errors here unless they're compiler errors:
Q_DECLARE_METATYPE(dovah::form_stub*)