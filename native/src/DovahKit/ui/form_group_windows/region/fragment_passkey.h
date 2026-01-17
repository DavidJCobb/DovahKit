#pragma once
namespace ui::region::fragments {
   class audio;
   class grass;
   class landscape;
   class map;
   class objects;
   class weather;
}

namespace ui::region {
   class fragment_passkey {
      friend fragments::audio;
      friend fragments::grass;
      friend fragments::landscape;
      friend fragments::map;
      friend fragments::objects;
      friend fragments::weather;
      private:
         constexpr fragment_passkey() {}
   };
}

