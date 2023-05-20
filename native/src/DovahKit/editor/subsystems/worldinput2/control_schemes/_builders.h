#pragma once
#include <initializer_list>
#include "../algorithms/input_sequence_stringification.h"
#include "../bind_tree/nodes/bound_tool.h"
#include "../bind_tree/nodes/editor_mode.h"
#include "../bind_tree/nodes/modifier.h"
#include "../bind_tree/nodes/root.h"
#include "../bind_tree/node.h"
#include "../bind_tree/tree.h"

#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"

namespace dovahkit::subsystems::worldinput2::default_control_schemes::build {
   inline binds::tree tree(worldinput2::input_device_type dt, std::initializer_list<binds::node*> nodes) {
      worldinput2::binds::tree out(dt);
      for (auto* node : nodes)
         out.root->append(*node);
      return out;
   }

   // TODO: This will only properly handle stringified input sequences for keyboards; fix that
   template<typename Sequence, typename Options> requires (
      !std::is_same_v<Options, worldedit::tools::tool_id> &&
      (
         std::is_same_v<Sequence, const char*>
         || std::is_same_v<Sequence, inputs::xinput_button>
         || std::is_same_v<Sequence, cobb::keyboard::key>
      )
   )
   inline worldinput2::binds::nodes::bound_tool* tool_node(
      const std::string& name,
      worldinput2::button_press_type pt,
      Sequence sequence,
      const Options& options
   ) {
      auto* node = new worldinput2::binds::nodes::bound_tool;
      node->name = name.c_str();
      node->button_press_type = pt;
      if constexpr (std::is_same_v<Sequence, const char*>) {
         node->input_sequence = algorithms::input_sequence_from_string(sequence);
      } else if constexpr (std::is_same_v<Sequence, inputs::xinput_button>) {
         auto* g = node->input_sequence.root = new input_sequence::group;
         g->type           = input_sequence::group_type::single_control;
         g->button.gamepad = sequence;
      } else if constexpr (std::is_same_v<Sequence, cobb::keyboard::key>) {
         auto* g = node->input_sequence.root = new input_sequence::group;
         g->type       = input_sequence::group_type::single_control;
         g->button.key = sequence;
      }
      //
      node->tool.id      = worldedit::tools::id_of<Options>;
      node->tool.options = new worldedit::tools::options_union(options);
      //
      return node;
   }
   
   // TODO: This will only properly handle stringified input sequences for keyboards; fix that
   template<typename Sequence> requires (
      std::is_same_v<Sequence, const char*>
      || std::is_same_v<Sequence, inputs::xinput_button>
      || std::is_same_v<Sequence, cobb::keyboard::key>
   )
   inline worldinput2::binds::nodes::bound_tool* tool_node(
      const std::string& name,
      worldinput2::button_press_type pt,
      Sequence sequence,
      worldedit::tools::tool_id tool_id
   ) {
      auto* node = new worldinput2::binds::nodes::bound_tool;
      node->name = name.c_str();
      node->button_press_type = pt;
      if constexpr (std::is_same_v<Sequence, const char*>) {
         node->input_sequence = algorithms::input_sequence_from_string(sequence);
      } else if constexpr (std::is_same_v<Sequence, inputs::xinput_button>) {
         auto* g = node->input_sequence.root = new input_sequence::group;
         g->type           = input_sequence::group_type::single_control;
         g->button.gamepad = sequence;
      } else if constexpr (std::is_same_v<Sequence, cobb::keyboard::key>) {
         auto* g = node->input_sequence.root = new input_sequence::group;
         g->type       = input_sequence::group_type::single_control;
         g->button.key = sequence;
      }
      //
      node->tool.id      = tool_id;
      node->tool.options = new worldedit::tools::options_union();
      //
      return node;
   }


   inline worldinput2::binds::nodes::editor_mode* editor_mode_node(
      worldedit::editor_mode em,
      std::initializer_list<worldinput2::binds::node*> children = {}
   ) {
      auto* node = new worldinput2::binds::nodes::editor_mode(em);
      for (auto* child : children) {
         node->append(*child);
      }
      return node;
   }
   inline worldinput2::binds::nodes::modifier* modifier_node(
      const std::string& name,
      const char* sequence,
      std::initializer_list<worldinput2::binds::node*> children = {}
   ) {
      auto* node = new worldinput2::binds::nodes::modifier;
      node->name = name.c_str();
      node->input_sequence = algorithms::input_sequence_from_string(sequence);
      for (auto* child : children) {
         node->append(*child);
      }
      return node;
   }

}