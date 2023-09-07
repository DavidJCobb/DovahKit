#include "./action.h"
#include <bit>
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

#include "editor/subsystems/worldedit/tool_system/utils/all_tool_serialization_ids.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace dovahkit::subsystems::worldinput {
   bool control_scheme_action::operator==(const control_scheme_action& other) const noexcept {
      if (this->button_press_type != other.button_press_type)
         return false;

      if (this->tool.id != other.tool.id)
         return false;

      if (this->name != other.name)
         return false;

      if (this->input_sequence != other.input_sequence)
         return false;

      auto* a = (worldedit::tools::options_union*)this->tool.options;
      auto* b = (worldedit::tools::options_union*)other.tool.options;
      //if ((a == nullptr) != (b == nullptr))
      //   //
      //   // This optimization doesn't work, because when reading a control scheme action from a 
      //   // bitstream, we blindly create an options union and allow it to potentially be a no-op.
      //   //
      //   return false;
      if (a) {
         if (*a != *b)
            return false;
      }
      return true;
   }

   void control_scheme_action::stream(cobb::bitstreams::reader& s) {
      s.stream<std::bit_width(max_name_length)>(name);

      uint8_t bpt;
      s.stream(bpt);
      if (bpt == 0) {
         this->button_press_type = button_press_type::none;
      } else {
         this->button_press_type = (enum button_press_type)(bpt - 1);
      }
      //
      s.stream(input_sequence);

      bool presence;
      s.stream(presence);
      if (presence) {

         cobb::eight_cc code;
         s.stream(code);

         const auto& codes = worldedit::tools::all_tool_serialization_ids;
         for (size_t id = 0; id < codes.size(); ++id) {
            if (codes[id] == code) {
               this->tool.id = (worldedit::tools::tool_id)id;
               break;
            }
         }

         auto* ou = new worldedit::tools::options_union;
         *ou = worldedit::tools::options_union::construct_for_type(this->tool.id);
         this->tool.options = ou;
         ou->stream(s);
      }
   }
   void control_scheme_action::stream(cobb::bitstreams::writer& s) const {
      s.stream<std::bit_width(max_name_length)>(name);

      uint8_t bpt = (int8_t)(this->button_press_type) + 1;
      s.stream(bpt);
      //
      s.stream(input_sequence);

      bool presence = (this->tool.id != worldedit::tools::id_of_none);
      s.stream(presence);
      if (presence) {
         const auto& codes = worldedit::tools::all_tool_serialization_ids;
         assert(this->tool.id < codes.size());
         s.stream(codes[(size_t)this->tool.id]);

         if (this->tool.options != nullptr) {
            ((worldedit::tools::options_union*)this->tool.options)->stream(s);
         } else {
            s.stream(false);
         }
      }
   }
}