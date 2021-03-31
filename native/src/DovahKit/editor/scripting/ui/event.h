#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <QVariant>
#include <QWidget>

namespace editor_script {
   class ui_event {
      public:
         QWidget& widget;
         const std::string event_name;
         const std::string listener_name;
         const std::vector<QVariant> params;

         ui_event(QWidget& w, const char* en, const char* ln, const std::vector<QVariant>& p) : widget(w), event_name(en), listener_name(ln), params(p) {}
   };

   struct ui_event_queue {
      protected:
         std::vector<ui_event*> list;
         std::recursive_mutex   lock;
      public:
         void clear();              // script thread should call this when doing cleanup
         void process();            // script thread should call this to process pending events
         void push_back(ui_event*); // main thread should call this to send events to lua

         void forget_about(QWidget&);
   };
}