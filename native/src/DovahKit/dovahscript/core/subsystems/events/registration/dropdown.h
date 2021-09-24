#pragma once
#include "../registration.h"
#include <QComboBox>

namespace dovahscript::impl::event_registration {
   class dropdown : public base {
      public:
         using target_type = QComboBox;

         static result register_event(QObject& object, const char* event_name, const char* listener_name);

         static constexpr const std::initializer_list<const char*> event_names = {
            "OnChanged", // The dropdown's selected logical index was changed through some cause other than the script directly setting it or the selected text.
         };
   };
}