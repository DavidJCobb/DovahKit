#pragma once
#include "../base.h"
#include <vector>
#include <QVariant>
#include <QWidget>

namespace editor_script::tasks::m2s {
   class ui_event : public cross_thread_task {
      public:
         QWidget& widget;
         const std::string event_name;
         const std::string listener_name;
         const std::vector<QVariant> params;

         ui_event(QWidget& w, const char* en, const char* ln, const std::vector<QVariant>& p) : widget(w), event_name(en), listener_name(ln), params(p) {}
         
      protected:
         virtual void _exec_impl() override;
   };
}