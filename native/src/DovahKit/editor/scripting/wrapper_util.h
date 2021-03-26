#pragma once
#include "wrapper.h"

namespace editor_script {
   extern const char* wrap_form(wrapper& out, dovah::form_stub*); // returns the appropriate metatable name to use
   extern const char* wrap_widget(wrapper& out, QWidget*);
}