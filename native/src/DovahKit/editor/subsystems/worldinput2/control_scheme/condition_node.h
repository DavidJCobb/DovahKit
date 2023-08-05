#pragma once
#include <QString>
#include "./condition.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput2 {
   class control_scheme_condition_node {
      public:
         static constexpr const size_t max_name_length = 1023;

      public:
         QString name;
         control_scheme_condition data;

         bool operator==(const control_scheme_condition_node&) const noexcept;

         void stream(cobb::bitstreams::reader&);
         void stream(cobb::bitstreams::writer&) const;
   };
}