#include "bound_tool.h"
#include <bit> // std::bit_width
#include "helpers/bitstreams/reader.h"
#include "helpers/bitstreams/writer.h"

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
   void bound_tool::_read_impl(cobb::bitstreams::reader& s) {
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
         if (this->tool.id != worldedit::tools::id_of_none) {
            auto* ou = new worldedit::tools::options_union;
            *ou = worldedit::tools::options_union::construct_for_type(this->tool.id);
            this->tool.options = ou;
            ou->stream(s);
         }
      }
   }
   void bound_tool::_write_impl(cobb::bitstreams::writer& s) const {
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

   bool bound_tool::_compare_impl(const node& other) const {
      assert(other.type == my_type);
      const auto& casted = static_cast<const bound_tool&>(other);

      if (this->button_press_type != casted.button_press_type)
         return false;

      if (this->tool.id != casted.tool.id)
         return false;

      if (this->name != casted.name)
         return false;

      {
         auto* opt_a = this->tool.options;
         auto* opt_b = casted.tool.options;

         bool has_a = (opt_a && opt_a->id() != worldedit::tools::id_of_none);
         bool has_b = (opt_b && opt_b->id() != worldedit::tools::id_of_none);
         if (has_a != has_b)
            return false;

         if (has_a) {
            auto* full_a = (worldedit::tools::options_union*)opt_a;
            auto* full_b = (worldedit::tools::options_union*)opt_b;

            if (*full_a != *full_b)
               return false;
         }
      }

      if (this->input_sequence != casted.input_sequence)
         return false;

      return true;
   }
}