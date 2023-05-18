#include "bound_tool.h"

#include "../../debugging.h"
#include "editor/subsystems/worldedit/tool_system/utils/all_tool_serialization_ids.h"
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
   node* bound_tool::_read_impl(cobb::streams::bitreader& stream) {
      stream.read(name);

      uint8_t bpt;
      stream.read(bpt);
      if (bpt == 0) {
         this->button_press_type = button_press_type::none;
      } else {
         this->button_press_type = (enum button_press_type)(bpt - 1);
      }
      //
      stream.read(input_sequence);

      bool presence;
      stream.read(presence);
      if (presence) {

         cobb::eight_cc code;
         stream.read(code);

         const auto& codes = worldedit::tools::all_tool_serialization_ids;
         for (size_t id = 0; id < codes.size(); ++id) {
            if (codes[id] == code) {
               this->tool.id = (worldedit::tools::tool_id)id;
               break;
            }
         }
         if (this->tool.id != worldedit::tools::id_of_none) {
            auto* ou = new worldedit::tools::options_union;
            this->tool.options = ou;
            ou->read(stream);
         }
      }
   }
   node* bound_tool::_write_impl(cobb::streams::bitwriter& stream) const {
      stream.write(name);

      uint8_t bpt = (int8_t)(this->button_press_type) + 1;
      stream.write(bpt);
      //
      stream.write(input_sequence);

      bool presence = (this->tool.id != worldedit::tools::id_of_none);
      stream.write(presence);
      if (presence) {
         const auto& codes = worldedit::tools::all_tool_serialization_ids;
         assert(this->tool.id < codes.size());
         stream.write(codes[(size_t)this->tool.id]);

         if (this->tool.options != nullptr) {
            ((worldedit::tools::options_union*)this->tool.options)->write(stream);
         } else {
            stream.write(false);
         }
      }
   }
}