#include "input.h"
#include "../../worldinput.h"

namespace dovahkit::subsystems::worldinput::binds::nodes {
   bool input::check_still_active(core& ih) const {
      if (!this->is_modifier)
         return false;
      auto r = ih.inputResultOf(this->mapping);
      if (this->mapping.is_button()) {
         if (this->mapping.button.press_type == button_press_type::while_down)
            return r.active();
         //
         // If it's not a "while" modifier, then treat it like a toggle. This function is 
         // called when it's currently active, so if the input is activated again, then it's 
         // being toggled off.
         //
         return !r.active();
      }
      return false;
   }

   node* input::_clone_impl() const {
      auto* copy = new input;
      copy->name        = this->name;
      copy->is_modifier = this->is_modifier;
      copy->mapping     = this->mapping;
      copy->tool        = this->tool;
      copy->params      = this->params;
      return copy;
   }
}