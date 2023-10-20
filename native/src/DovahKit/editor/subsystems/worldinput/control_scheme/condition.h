#pragma once
#include <QString>
#include "../condition_set.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput {
   class control_scheme_condition {
      public:
         static constexpr const size_t max_name_length = 1023;

      public:
         QString name;
         condition_set data;

         bool operator==(const control_scheme_condition&) const noexcept;

         void stream(cobb::bitstreams::reader&);
         void stream(cobb::bitstreams::writer&) const;
   };
}