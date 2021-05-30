#pragma once
#include "wrapper.h"

namespace editor_script {
   class LuaManagedResource;

   extern const char* wrap_form(wrapper& out, dovah::form_stub*); // returns the appropriate metatable name to use
   extern const char* wrap_widget(wrapper& out, QWidget*);
   extern const char* wrap_button_group(wrapper& out, QButtonGroup&);
   extern const char* wrap_resource(wrapper& out, LuaManagedResource&);

   // Use when creating a new widget. For example, not every QFrame should be a horizontal or 
   // vertical rule, but QFrames that *are* horizontal or vertical rules need to use a specific 
   // metatable whenever they are accessed. Ensure that the string you pass in is not deleted 
   // later (e.g. use a constexpr string or something).
   extern void override_widget_metatable(QWidget*, const char* metatable_key);
}