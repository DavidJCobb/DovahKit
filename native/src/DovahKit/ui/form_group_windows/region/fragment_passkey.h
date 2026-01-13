#pragma once
namespace ui::region::fragments {
   class objects;
   class weather;
}

namespace ui::region {
   class fragment_passkey {
      friend fragments::objects;
      friend fragments::weather;
      private:
         constexpr fragment_passkey() {}
   };
}

