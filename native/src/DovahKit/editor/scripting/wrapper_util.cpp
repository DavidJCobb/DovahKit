#include "wrapper_util.h"
#include "wrappers/_all_forms.h"
#include "wrappers/_all_ui.h"

namespace editor_script {
   extern const char* wrap_form(wrapper& out, dovah::form_stub* stub) {
      out.stub = stub;
      out.type = wrapper_type::form_data;
      if (stub) {
         switch (stub->formType) { // TODO: an actual list would maybe be more efficient than a switch-case once we end up with a large number of metatables here
            case dovah::form_type::formlist:
               return wrappers::formlist::metatable_key;
            case dovah::form_type::shout:
               return wrappers::shout::metatable_key;
            case dovah::form_type::voicetype:
               return wrappers::voicetype::metatable_key;
            case dovah::form_type::word_of_power:
               return wrappers::word_of_power::metatable_key;
         }
      }
      return wrappers::form::metatable_key;
   }
   extern const char* wrap_widget(wrapper& out, QWidget* widget) {
      out.widget = widget;
      out.type   = wrapper_type::ui;
      if (widget) {
         if (qobject_cast<QDialog*>(widget))
            return wrappers::ui::window::metatable_key;
      }
      return wrappers::ui::widget::metatable_key; // TODO: use a generic widget metatable
   }
}