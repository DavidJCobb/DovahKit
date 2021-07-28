#pragma once
#include <string>
#include <vector>
#include <QObject>
#include <QVariant>

namespace dovahscript::impl {
   class script_event {
      public:
         QObject& target;
         const std::string event_name;
         const std::string listener_name;
         const std::vector<QVariant> params;

         script_event(QObject& t, const char* en, const char* ln, const std::vector<QVariant>& p) : target(t), event_name(en), listener_name(ln), params(p) {}
   };
}