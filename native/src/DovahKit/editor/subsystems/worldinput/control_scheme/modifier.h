#pragma once
#include <QString>
#include "../input_sequence.h"

namespace cobb::bitstreams {
   class reader;
   class writer;
}

namespace dovahkit::subsystems::worldinput {
   class control_scheme_modifier {
      public:
         static constexpr const size_t max_name_length = 1023;

      public:
         QString name;
         typename input_sequence input_sequence;

         bool operator==(const control_scheme_modifier&) const noexcept;

         void stream(cobb::bitstreams::reader&);
         void stream(cobb::bitstreams::writer&) const;

         void assert_validity() const;
   };
}