#include "./story_event_member_id.h"

namespace dovah {
   static_assert(
      []() -> bool {
         story_event_member_id v('R1');
         return (uint16_t)v == 'R1';
      }()
   );
   static_assert(
      []() -> bool {
         story_event_member_id v('R1');
         auto cs = v.to_string();
         return cs[0] == 'R' && cs[1] == '1';
      }()
   );
   static_assert(
      []() -> bool {
         story_event_member_id v;
         v.from_string("R1");
         return (uint16_t)v == 'R1';
      }()
   );
   static_assert(
      []() -> bool {
         story_event_member_id v;
         v.set_first_char('R');
         v.set_second_char('1');
         return (uint16_t)v == 'R1';
      }()
   );
}