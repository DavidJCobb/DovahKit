#include "./worldinput2.h"
#include <array>
#include <QBoxLayout>
#include <QComboBox>
#include <QDialog>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QPointer>
#include <QTimer>
#include "editor/subsystems/worldinput2/core.h"

#include "editor/subsystems/worldinput2/algorithms/input_sequence_stringification.h"
#include "editor/subsystems/worldinput2/control_scheme/all_node_headers.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "editor/subsystems/worldedit/tool_system/tools/debug_print.h"
#include "editor/subsystems/worldedit/tool_system/id_of.h"
#include "editor/subsystems/worldedit/tool_system/options_union.h"
#include "editor/subsystems/worldedit/tool_system/tool_results_tuple.h"

namespace {
   namespace worldedit {
      using namespace dovahkit::subsystems::worldedit;
   }
   namespace worldinput {
      using namespace dovahkit::subsystems::worldinput;
   }
   using worldinput::control_scheme;
   using worldinput::control_scheme_action;
   using worldinput::control_scheme_modifier;

   struct testing_tree {
      std::string name;
      worldinput::control_scheme tree;
   };

   worldinput::control_scheme make_tree(worldinput::input_device_type dt, std::initializer_list<worldinput::control_scheme::node*> nodes) {
      control_scheme out(dt);
      for (auto* node : nodes)
         out.top_level_nodes.push_back(node);

      constexpr auto warn = [](const worldinput::control_scheme& out) {
         constexpr auto recurse = [&](const control_scheme::node& node, auto& recurse) -> void {
            if (auto* in = node.as<control_scheme_action>()) {
               if (in->data.input_sequence.is_probably_keyboard_impossible()) {
                  std::string seq;
                  worldinput::algorithms::input_sequence_to_string(in->data.input_sequence, seq);
                  qDebug(
                     "WARNING: One of the test bind trees contains an input sequence that may not be completable on a gamepad.\n   Node:     %s\n   Sequence: %s",
                     qUtf8Printable(in->data.name),
                     seq.c_str()
                  );
               }
            }
            for (auto* child : node.children) {
               recurse(*child, recurse);
            }
         };
         for (auto* n : out.top_level_nodes)
            recurse(*n, recurse);
      };
      if (dt == worldinput::input_device_type::keyboard_mouse) {
         warn(out);
      }

      return out;
   }

   cobb::typed_node<control_scheme::node, control_scheme_action>* make_tool_node(
      const std::string& name,
      worldinput::button_press_type pt,
      const worldinput::input_sequence& sequence
   ) {
      return control_scheme::node::from_data(control_scheme_action{
         .name = name.c_str(),
            //
         .input_sequence    = sequence,
         .button_press_type = pt,
         //
         .tool = {
            .id      = worldedit::tools::id_of<worldedit::tools::debug_print>,
            .options = new worldedit::tools::options_union(
               worldedit::tools::debug_print::options{
                  .text = name
               }
            ),
         },
      });
   }
   cobb::typed_node<control_scheme::node, control_scheme_modifier>* make_modifier_node(
      const std::string& name,
      const worldinput::input_sequence& sequence,
      std::initializer_list<control_scheme::node*> children = {}
   ) {
      auto* node = control_scheme::node::from_data(control_scheme_modifier{
         .name           = name.c_str(),
         .input_sequence = sequence,
      });
      for (auto* child : children) {
         node->append_child(*child);
      }
      return node;
   }

   static auto testing_trees = std::array{
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Simple test tree",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press X",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
               make_tool_node(
                  "Long Press Y",
                  worldinput::button_press_type::long_press,
                  worldinput::algorithms::input_sequence_from_string("Y")
               ),
               make_tool_node(
                  "Hold Z",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("Z")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Press/Long Press basic conflict",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press X",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
               make_tool_node(
                  "Long Press X",
                  worldinput::button_press_type::long_press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Press-delays-Hold conflict resolution",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press X",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
               make_tool_node(
                  "Bind #2: Hold X",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
               make_tool_node(
                  "Bind #3: Press [Y + Z]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[Y + Z]")
               ),
               make_tool_node(
                  "Bind #4: Hold Y",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("Y")
               ),
               make_tool_node(
                  "Bind #5: Hold Z",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("Z")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Hold-blocks-Press conflict resolution",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Hold [X + Z]",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("[X + Z]")
               ),
               make_tool_node(
                  "Press X",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Basic gamepad tests",
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Press A",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("A", true)
               ),
               make_tool_node(
                  "Press B",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("B", true)
               ),
               make_tool_node(
                  "Long Press X",
                  worldinput::button_press_type::long_press,
                  worldinput::algorithms::input_sequence_from_string("X", true)
               ),
               make_tool_node(
                  "Hold Y",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("Y", true)
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Modifier conflict rule",
         //
         // Test procedure:
         // 
         //  - Press and hold Gamepad LS.
         //  - Press and hold Gamepad B.
         //  - Press and release Gamepad X.
         // 
         // Desired result: neither bind activates.
         //
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [LS + X]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[LS + X]", true)
               ),
               make_tool_node(
                  "Press [B + X]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[B + X]", true)
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Basic sequence tests",
         //
         // Test procedure:
         // 
         //  - Press and hold A.
         //  - Press and release B.
         //     - The `Press [A + B]` bind should fire.
         //  - Release A.
         //  - Press and hold X.
         //  - Press and hold Y.
         //  - Release X and Y in any order.
         //     - The `Press (X + Y)` bind should fire.
         //  - Press and hold Y.
         //  - Press and hold X.
         //  - Release X and Y in any order.
         //     - The `Press (X + Y)` bind should fire.
         //     = This verifies the lack of ordering requirements on concurrent-and-unordered sequences.
         //  - Press and release J.
         //  - Press and release K.
         //     - The `Press <J + K>` bind should fire.
         //  - Press and release K.
         //  - Press and release J.
         //     - Nothing should happen.
         //     = This verifies ordering for separate-and-ordered sequences.
         //  - Press and release J.
         //  - Press and release A.
         //  - Press and release K.
         //     - Nothing should happen.
         //     = This tests interrupting a separate-and-ordered sequence with unrelated keys.
         //  - Press and release J.
         //  - Press and release J.
         //  - Press and release K.
         //     - The `Press <J + K>` bind should fire.
         //     = This tests interrupting a separate-and-ordered sequence with itself.
         //
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [A + B]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B]")
               ),
               make_tool_node(
                  "Press (X + Y)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(X + Y)")
               ),
               make_tool_node(
                  "Press <J + K>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<J + K>")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Specificity rule",
         //
         // Test procedure:
         // 
         //  - Press and hold LB.
         //  - Pres and release, in order, A, B, X, and Y.
         // 
         // Desired result: only the `Press [A + B + X + Y]` bind activates.
         //
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [A + B + X + Y]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Press (LB + Y)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(LB + Y)", true)
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Identical binds rule",
         //
         // Desired result: both binds activate when pressing X.
         //
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
               make_tool_node(
                  "Bind #2",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Seamless switching between Hold binds",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Hold A",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("A")
               ),
               make_tool_node(
                  "Hold (A + B)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(A + B)")
               ),
               make_tool_node(
                  "Hold (A + B + C)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(A + B + C)")
               ),
               make_tool_node(
                  "Hold (A + C)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(A + C)")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Repeated combo activation",
         //
         // Test procedure:
         // 
         //  - Press and hold X.
         //  - Press and release Z.
         //  - Press and release Z again.
         // 
         // Desired result: the bind activates with each release of the Z key, 
         // without having to release X between attempted activations.
         //
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [X + Z]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[X + Z]")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Specificity rule, incl. across time",
         //
         // Multiple tests. 
         // 
         // Case A1:
         //  - LB down.
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - Y up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case A2:
         //  - LB down.
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - A, B, or X up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //  - Y up.
         //     - Bind 2 does not activate.
         // 
         // Case B:
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - A, B, or X up.
         //     - Bind 1 fires.
         //  - LB down.
         //  - LB up.
         //     - Bind 2 fires.
         // 
         // Case C1:
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - LB down.
         //  - Y up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case C2:
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - LB down.
         //  - A, B, or X up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //  - LB up.
         //     - Bind 2 does not activate.
         // 
         // Case D:
         //  - A down.
         //  - B down.
         //  - LB down.
         //  - X down.
         //  - Y down.
         //  - A, B, or X up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //  - LB up.
         //     - Bind 2 does not activate.
         //  - Y up.
         //     - Bind 2 does not activate.
         // 
         // Case E:
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - LB down.
         //  - Y up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case F:
         //  - LB down.
         //  - A down.
         //  - B down.
         //  - X down.
         //  - Y down.
         //  - LB up.
         //     - Bind 2 fires.
         //  - A, B, X, or Y up.
         //     - Bind 1 fires.
         // 
         // Case G:
         //  - LB down.
         //  - LT down.
         //  - Y down.
         //  - Y up.
         //     - Bind 2 and Bind 3 both lose a same-frame conflict.
         //        = They have equal specificity and went wholly down at the same time.)
         //     - Bind 2 does not activate.
         //     - Bind 3 does not activate.
         // 
         // Case H:
         //  - LB down.
         //  - Y down.
         //  - LT down.
         //  - LB up.
         //     - Bind 2 fires.
         //     - Bind 3 is blocked. (Bind 2 went wholly down before Bind 3, and they have equal specificity.)
         //  - LT up.
         //     - Bind 3 does not activate.
         //
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Bind #1: Press [A + B + X + Y]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Bind #2: Press (Y + LB)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(Y + LB)", true)
               ),
               make_tool_node(
                  "Bind #3: Press (Y + LT)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(Y + LT)", true)
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Separate-and-ordered interruptions #1",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + D + F>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + D + F>")
               ),
               make_tool_node(
                  "Press <A + S + D + [F + <J + K>]>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + D + [F + <J + K>]>")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Press <A + S + D + [<J + K> + Z]>",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + D + [<J + K> + Z]>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + D + [<J + K> + Z]>")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 1",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold S",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("S")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 2",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold (K + S)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(K + S)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 3",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 4",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (X + <A + S + J>)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(X + <A + S + J>)")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 5",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [X + <A + S + J>]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[X + <A + S + J>]")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press/Hold with common stem",
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [X + Y + A]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[X + Y + A]", true)
               ),
               make_tool_node(
                  "Hold [X + Y + B]",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("[X + Y + B]", true)
               ),
            }
         ),
      },
      testing_tree{
         .name = "Advancing Press past conflict key indefinitely delays Hold?",
         .tree = make_tree(
            worldinput::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [A + B + X + Y]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Hold B",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("B", true)
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold: outlast one Press; advanced past by another",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [A + B]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B]")
               ),
               make_tool_node(
                  "Bind #2: Press [A + B + J]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + B + J]")
               ),
               make_tool_node(
                  "Bind #3: Hold B",
                  worldinput::button_press_type::hold,
                  worldinput::algorithms::input_sequence_from_string("B")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Edge-case: sequencing constraints on first item in group",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [B + [A + B]]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[B + [A + B]]")
                  //
                  // When this test-case was first written, the first item in an ordered  
                  // group was unaware of the timestamp of the group's own previous sibling, 
                  // and so would not disqualify inputs which preceded it. This means that 
                  // this bind, which should ordinarily be impossible, would instead trigger 
                  // if you entered (A + B).
                  // 
                  // We've since fixed that, so now, this sequence should be impossible to 
                  // trigger, no matter which key goes down first and which key is released 
                  // first.
                  //
               ),
            }
         ),
      },
      testing_tree{
         .name = "Redundant binds",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [A + A + B] // equivalent to [A + B]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + A + B]")
                  //
                  // Concurrent-and-ordered groups are stateless and allow you to advance 
                  // through multiple keys in a single frame, if those keys all went down 
                  // on that same frame. As such, when you have multiple of the same key 
                  // in a row, you advance through all of them at once.
                  //
               ),
               make_tool_node(
                  "Bind #2: Press [J + [J + K]] // equivalent to [J + K]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[J + [J + K]]")
                  //
                  // Concurrent-and-ordered groups are stateless and allow you to advance 
                  // through multiple keys in a single frame, if those keys all went down 
                  // on that same frame. What's more: the first item in a nested concurrent-
                  // and-ordered group compares timestamps to the group's previous sibling. 
                  // This means that when you press J, we advance past the lone J key, 
                  // recurse into the nested ISG, and there we advance past its J key as 
                  // well: J needs to have gone down either after or at the same time as 
                  // itself.
                  //
               ),
               make_tool_node(
                  "Bind #3: Press (X + (X + Y)) // equivalent to (X + Y)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(X + (X + Y))")
               ),
               make_tool_node(
                  "Bind #4: Press (Q + [Q + W]) // equivalent to [Q + W]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(Q + [Q + W])")
                  //
                  // The inner ISG requires that Q and W be down on the same frame, and that 
                  // Q have gone down first. The outer ISG requires that constraint to have 
                  // been met, and requires Q to be down, with no regard for when it went 
                  // down in relation to said constraint being met (i.e. before, at the same 
                  // time, or after).
                  //
               ),
               make_tool_node(
                  "Bind #5: Press [E + (R + E)] // equivalent to (E + R)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[E + (R + E)]")
                  //
                  // The outer ISG requires that E go down before or at the same time as 
                  // the inner ISG. The inner ISG goes down when R and E are pressed down, 
                  // regardless of their ordering. The inner ISG reports its "down" time 
                  // as the most recent time at which any of its contents went down. As 
                  // such: if you press E and then R, then the inner ISG goes down last; 
                  // if you press R and the nE, then the inner ISG goes down at the same 
                  // time as the E-key previous sibling.
                  //
               ),
               make_tool_node(
                  "Bind #6: Press [T + <C + V> + <C + V>] // impossible",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[T + <C + V> + <C + V>]")
                  //
                  // If the first separate-and-ordered group is not fully down (i.e. not on its 
                  // last key), then we don't advance past it to update the next separate-and-
                  // ordered group. Therefore, when we finally do reach the next separate-and-
                  // ordered group, it's expecting C to be pressed down again.
                  // 
                  // However, when we release V the first time, the first separate-and-ordered 
                  // group flags as released and (by necessity) clears its ISG-level state and 
                  // the ISG-level state of its descendents: it resets back to its start.
                  // 
                  // This means that if you press and hold T, press and release C, press and 
                  // release V, and then press and release C again, that second C-press will 
                  // be caught by the first separate-and-ordered group, which will "replay" 
                  // again. As such, you can never advance the second separate-and-ordered 
                  // group, making this sequence impossible to complete.
                  //
               ),
               make_tool_node(
                  "Bind #7: Press [1 + <2 + 3> + 3] // equivalent to [1 + <2 + 3>]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[1 + <2 + 3> + 3]")
                  //
                  // This is the same basic principle as Bind 1. To activate this bind, you 
                  // would press and hold 1, press and release 2, press and hold 3,... at 
                  // which point you've advanced past the inner ISG and to the lone 3-key, 
                  // and 3 is down at the same time as itself, so the outer ISG advances to 
                  // its end and successfully goes down.
                  //
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press [A + <J + K> + <J + K>]",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [A + <J + K> + <J + K>]",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("[A + <J + K> + <J + K>]")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press (A + <J + K> + <J + K>)",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (A + <J + K> + <J + K>)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(A + <J + K> + <J + K>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press (A + <B + A>) and Press (X + <X + Y>)",
         .tree = make_tree(
            worldinput::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (A + <B + A>)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(A + <B + A>)")
               ),
               make_tool_node(
                  "Press (X + <X + Y>)",
                  worldinput::button_press_type::press,
                  worldinput::algorithms::input_sequence_from_string("(X + <X + Y>)")
               ),
            }
         ),
      },
      #pragma region Modifier node tests
         testing_tree{
            .name = "Modifier parity: Press [A + B]",
            .tree = make_tree(
               worldinput::input_device_type::keyboard_mouse,
               {
                  make_modifier_node(
                     "Press [A + ...]",
                     worldinput::algorithms::input_sequence_from_string("A"),
                     {
                        make_tool_node(
                           "Press [A + B]",
                           worldinput::button_press_type::press,
                           worldinput::algorithms::input_sequence_from_string("B")
                        ),
                     }
                  ),
                  make_tool_node(
                     "Press [X + Y]",
                     worldinput::button_press_type::press,
                     worldinput::algorithms::input_sequence_from_string("[X + Y]")
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Modifier parity: Press [(A + B) + C]",
            .tree = make_tree(
               worldinput::input_device_type::keyboard_mouse,
               {
                  make_modifier_node(
                     "Press [(A + B) + ...]",
                     worldinput::algorithms::input_sequence_from_string("(A + B)"),
                     {
                        make_tool_node(
                           "Press [(A + B) + C]",
                           worldinput::button_press_type::press,
                           worldinput::algorithms::input_sequence_from_string("C")
                        ),
                     }
                  ),
                  make_tool_node(
                     "Press [(X + Z) + Y]",
                     worldinput::button_press_type::press,
                     worldinput::algorithms::input_sequence_from_string("[(X + Z) + Y]")
                  ),
               }
            ),
         },
      #pragma endregion
      #pragma region Directional constraint tests
         testing_tree{
            .name = "Directional constraint tests 1",
            .tree = make_tree(
               worldinput::input_device_type::keyboard_mouse,
               {
                  make_tool_node(
                     "Hold A :: Mouse Move",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string("A :: Mouse Move")
                  ),
                  make_tool_node(
                     "Press B :: Mouse Move",
                     worldinput::button_press_type::press,
                     worldinput::algorithms::input_sequence_from_string("B :: Mouse Move")
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Directional constraint tests 2",
            .tree = make_tree(
               worldinput::input_device_type::keyboard_mouse,
               {
                  make_tool_node(
                     "Hold A :: Mouse Move",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string("A :: Mouse Move")
                  ),
                  make_tool_node(
                     "Hold B :: Mouse Move",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string("B :: Mouse Move")
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Directional constraint tests 3 (scalar)",
            .tree = make_tree(
               worldinput::input_device_type::keyboard_mouse,
               {
                  make_tool_node(
                     "Hold A :: Mouse Move X",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string("A :: Mouse Move X")
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Directional constraint tests 4 (gamepad)",
            .tree = make_tree(
               worldinput::input_device_type::xinput,
               {
                  make_tool_node(
                     "Hold A :: Left Stick X",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string("A :: Left Stick X", true)
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Directional constraint test: Left Stick only, no buttons",
            .tree = make_tree(
               worldinput::input_device_type::xinput,
               {
                  make_tool_node(
                     "Hold <none> :: Left Stick",
                     worldinput::button_press_type::hold,
                     worldinput::algorithms::input_sequence_from_string(":: Left Stick", true)
                  ),
               }
            ),
         },
      #pragma endregion
   };
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2::execute(QWidget* from) {
      auto& core = worldinput::core::get();

      static QTimer poll_timer;
      static QPointer<QDialog> test_window = nullptr;
      QObject::connect(&poll_timer, &QTimer::timeout, []() {
         if (!test_window)
            return;
         if (!test_window->isVisible())
            return;

         double elapsed;
         ::worldedit::tool_results_tuple results;

         worldinput::core::get().doPerFrameInputProcessing(elapsed, results);

         auto* label = test_window->findChild<QLabel*>("debug_output");
         if (label) {

            static double no_empty_timer = 0.0;

            QString text;
            if (results.has_member<worldedit::tools::debug_print>()) {
               const auto& data = results.get_member<worldedit::tools::debug_print>();
               text = data.text.c_str();
               text = text.trimmed();
            }

            if (!text.isEmpty()) {
               no_empty_timer = 0.0;
               label->setText(text);
            } else {
               no_empty_timer += elapsed;
               if (no_empty_timer > 0.2) {
                  label->setText(text);
                  no_empty_timer = 0.0;
               }
            }
         }
      });
      if (!test_window) {
         auto* layout = new QVBoxLayout();

         test_window = new QDialog(from);
         test_window->setLayout(layout);

         auto* frame = new QFrame(test_window);
         frame->setObjectName("frame");
         frame->setFocusPolicy(Qt::FocusPolicy::ClickFocus);
         frame->setMinimumSize({ 320, 240 });
         {
            auto* layout = new QVBoxLayout(frame);
            //
            auto* debug = new QLabel(frame);
            debug->setObjectName("debug_output");
            debug->setAlignment(Qt::AlignTop | Qt::AlignLeft);
            debug->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
            debug->setContentsMargins(0, 0, 0, 0);
            debug->setTextFormat(Qt::TextFormat::PlainText);
            //
            layout->addWidget(debug);
         }

         auto* label = new QLabel("Bind tree:", test_window);
         auto* combo = new QComboBox(test_window);
         combo->setObjectName("combo");

         for (auto& entry : testing_trees) {
            combo->addItem(entry.name.c_str());
         }
         QObject::connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged), [combo](int index) {
            auto& entry = testing_trees[index];
            auto& core  = worldinput::core::get();
            core.setBindingsFor(entry.tree);
         });

         layout->addWidget(frame);
         layout->addWidget(label);
         layout->addWidget(combo);
      }
      if (!poll_timer.isActive()) {
         poll_timer.setSingleShot(false);
         poll_timer.setInterval(1);
         poll_timer.start();
      }

      core.setTargetWidget(test_window->findChild<QFrame*>("frame"));
      test_window->show();
      if (auto* combo = test_window->findChild<QComboBox*>("combo")) {
         combo->setCurrentIndex(1);
         combo->setCurrentIndex(0); // when the combobox is initially created, it defaults to this, and so this call on its own wouldn't count as "changing"
      }
   }
}