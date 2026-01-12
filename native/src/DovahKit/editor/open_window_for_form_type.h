#pragma once
#include "dovah/form_types.h"

// To be used for cases where a single window is reused for all forms of a given type, 
// e.g. for Idle Animations.
extern void open_edit_dialog_for_form_type(dovah::form_type);