#pragma once
#include "../base.h"
#include <QString>

namespace editor_script::tasks::s2m {
   class log_message : public cross_thread_task {
      //
      // Message for sending text to the main window to be displayed in a script output pane.
      //
      public:
         QString text;
         //
      protected:
         virtual void _exec_impl() override;
   };
}