#include "bound_tool.h"

#include "../../debugging.h"
#include "../../tools/combined_tool_results.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace dovahkit::subsystems::worldinput2::binds::nodes {
   bound_tool::~bound_tool() {
      if (auto*& o = this->tool.options) {
         delete ((worldedit::tools::options_union*)o);
         o = nullptr;
      }
   }

   node* bound_tool::_clone_impl() const {
      auto* copy = new bound_tool;
      copy->name              = this->name;
      copy->button_press_type = this->button_press_type;
      copy->input_sequence    = this->input_sequence;
      //
      copy->tool.id = this->tool.id;
      if (this->tool.options)
         copy->tool.options = ((worldedit::tools::options_union*)this->tool.options)->clone();
      //
      return copy;
   }
}