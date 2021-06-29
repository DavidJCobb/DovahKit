#pragma once
#include "wrapper.h"

namespace editor_script {
   class LuaManagedResource;

   extern const char* wrap_form(wrapper& out, dovah::form_stub*); // returns the appropriate metatable name to use; stub must not be null
   extern const char* wrap_widget(wrapper& out, QWidget*);
   extern const char* wrap_button_group(wrapper& out, QButtonGroup&);
   extern const char* wrap_resource(wrapper& out, LuaManagedResource&);

   // Wrap a form stub in a Lua userdata and push that onto the stack. You will not have an 
   // opportunity to configure the C-side "wrapper" object created in the process.
   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]] 
   extern int wrap_and_push_form(lua_State*, dovah::form_stub*);

   // Wrap a form stub in a Lua userdata and push that onto the stack. You will not have an 
   // opportunity to configure the C-side "wrapper" object created in the process.
   [[nodiscard("When returning a wrapper to Lua from a native API, you must tell the Lua VM how many values you've returned; you should return this function's return value.")]]
   extern int wrap_and_push_form(lua_State*, const dovah::form_reference_t&);

   // Use when creating a new widget. For example, not every QFrame should be a horizontal or 
   // vertical rule, but QFrames that *are* horizontal or vertical rules need to use a specific 
   // metatable whenever they are accessed. Ensure that the string you pass in is not deleted 
   // later (e.g. use a constexpr string or something).
   extern void override_widget_metatable(QWidget*, const char* metatable_key);

   [[nodiscard]] extern dovah::form_stub* pull_form_stub_argument(lua_State*, int arg, dovah::form_type_t ft = dovah::form_type::none);
}