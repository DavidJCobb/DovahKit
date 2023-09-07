#include "./hold_blocks_press.h"
#include "../bind_list.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput::algorithms {
   extern bool hold_blocks_press(
      const bind_list_item& press,
      const bind_list_item& hold
   ) {
      // A conflict is present if the terminal inputs of the two binds overlap.
      const auto term_p = press.input_sequence.terminal_inputs();
      const auto term_h = hold.input_sequence.terminal_inputs();
      //
      for (const auto& item_p : term_p)
         for (const auto& item_h : term_h)
            if (item_p == item_h)
               return true;
      
      return false;
   }
}