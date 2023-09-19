#pragma once
#include "helpers/keyboard/key.h"
#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "../control_scheme/action.h"
#include "../inputs/button.h"
#include "../input_sequence.h"

namespace dovahkit::subsystems::worldinput::builtin_control_schemes {
   inline input_sequence _single_button_sequence(const inputs::xinput_button key) {
      input_sequence out;

      auto* g = out.root = new input_sequence::group;
      g->type = input_sequence::group_type::single_control;
      g->button.gamepad = key;

      return out;
   }
   inline input_sequence _single_button_sequence(const cobb::keyboard::key& key) {
      input_sequence out;

      auto* g = out.root = new input_sequence::group;
      g->type = input_sequence::group_type::single_control;
      g->button.key = key;

      return out;
   }
   inline input_sequence _single_button_sequence(Qt::MouseButton mb) {
      input_sequence out;

      auto* g = out.root = new input_sequence::group;
      g->type = input_sequence::group_type::single_control;
      g->button.mouse = mb;

      return out;
   }

   template<typename Options>
   inline auto _tool_with_options(const Options& options) {
      decltype(control_scheme_action::tool) out = {};
      out.id      = worldedit::tools::id_of<Options>;
      out.options = new worldedit::tools::options_union(options);
      return out;
   }

   template<typename Tool>
   inline auto _tool_sans_options() {
      decltype(control_scheme_action::tool) out = {};
      out.id      = worldedit::tools::id_of<Tool>;
      out.options = new worldedit::tools::options_union(typename Tool::options{});
      return out;
   }
}