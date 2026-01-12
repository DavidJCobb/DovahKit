#pragma once
namespace ui::region::fragments {
   class objects;
}

namespace ui::region {
   class fragment_passkey {
      friend fragments::objects;
      private:
         constexpr fragment_passkey() {}
   };
}

