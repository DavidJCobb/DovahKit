#pragma once
#include <mutex>
#include <string>
#include <vector>
#include <QVariant>
#include <QWidget>

namespace editor_script {
   class ui_event {
      public:
         QObject& target;
         const std::string event_name;
         const std::string listener_name;
         const std::vector<QVariant> params;

         ui_event(QObject& t, const char* en, const char* ln, const std::vector<QVariant>& p) : target(t), event_name(en), listener_name(ln), params(p) {}
   };

   struct ui_event_queue {
      protected:
         std::vector<ui_event*> list;
         mutable std::recursive_mutex lock;
      public:
         void   clear();              // script thread should call this when doing cleanup
         size_t process();            // script thread should call this to process pending events; returns number of events processed
         void   push_back(ui_event*); // main thread should call this to send events to lua

         void forget_about(QObject&);

         size_t size() const noexcept;
   };
}