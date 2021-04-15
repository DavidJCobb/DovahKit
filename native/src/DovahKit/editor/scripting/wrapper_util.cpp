#include "wrapper_util.h"
#include "wrappers/_all_forms.h"
#include "wrappers/_all_ui.h"

namespace editor_script {
   extern const char* wrap_form(wrapper& out, dovah::form_stub* stub) {
      assert(stub != nullptr && "Wrappers with null pertinent pointers are illegal! Push nil to Lua instead!");
      out.stub = stub;
      out.type = wrapper_type::form_data;
      if (stub) {
         switch (stub->formType) { // TODO: an actual list would maybe be more efficient than a switch-case once we end up with a large number of metatables here
            case dovah::form_type::formlist:
               return wrappers::formlist::metatable_key;
            case dovah::form_type::shout:
               return wrappers::shout::metatable_key;
            case dovah::form_type::topic:
               return wrappers::topic::metatable_key;
            case dovah::form_type::topic_info:
               return wrappers::topic_info::metatable_key;
            case dovah::form_type::quest:
               return wrappers::quest::metatable_key;
            case dovah::form_type::voicetype:
               return wrappers::voicetype::metatable_key;
            case dovah::form_type::word_of_power:
               return wrappers::word_of_power::metatable_key;
         }
      }
      return wrappers::form::metatable_key;
   }
   extern const char* wrap_widget(wrapper& out, QWidget* widget) {
      assert(widget != nullptr && "Wrappers with null pertinent pointers are illegal! Push nil to Lua instead!");
      out.widget = widget;
      out.type   = wrapper_type::ui;
      if (widget) {
         if (qobject_cast<QComboBox*>(widget))
            return wrappers::ui::dropdown::metatable_key;
         if (qobject_cast<QDialog*>(widget))
            return wrappers::ui::window::metatable_key;
         if (qobject_cast<QDoubleSpinBox*>(widget))
            return wrappers::ui::spinbox::metatable_key;
         if (qobject_cast<QLabel*>(widget))
            return wrappers::ui::text::metatable_key;
         if (qobject_cast<QLineEdit*>(widget))
            return wrappers::ui::textbox::metatable_key;
         if (qobject_cast<QProgressBar*>(widget))
            return wrappers::ui::progress_bar::metatable_key;
         if (qobject_cast<QPushButton*>(widget))
            return wrappers::ui::button::metatable_key;
         if (qobject_cast<QRadioButton*>(widget))
            return wrappers::ui::radio_button::metatable_key;
         if (qobject_cast<FormPicker*>(widget))
            return wrappers::ui::formpicker::metatable_key;
      }
      return wrappers::ui::widget::metatable_key; // TODO: use a generic widget metatable
   }
   extern const char* wrap_button_group(wrapper& out, QButtonGroup& group) {
      out.button_group = &group;
      out.type         = wrapper_type::ui_button_group;
      return wrappers::ui::radio_group::metatable_key;
   }
}