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

#include "editor/subsystems/worldinput2/bind_tree/tree.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/bound_tool.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/modifier.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/root.h"

#include "editor/subsystems/worldinput2/tools/combined_tool_results.h"

namespace {
   namespace worldinput2 {
      using namespace dovahkit::subsystems::worldinput2;
   }

   struct testing_tree {
      std::string name;
      worldinput2::binds::tree tree;
   };

   worldinput2::binds::tree make_tree(worldinput2::input_device_type dt, std::initializer_list<worldinput2::binds::node*> nodes) {
      worldinput2::binds::tree out(dt);
      for (auto* node : nodes)
         out.root->append(*node);

      constexpr auto warn = [](const worldinput2::binds::tree& out) {
         constexpr auto recurse = [&](const worldinput2::binds::node& node, auto& recurse) -> void {
            if (auto* in = node.as<worldinput2::binds::nodes::abstract_input_node>()) {
               if (in->input_sequence.is_probably_keyboard_impossible()) {
                  std::string seq;
                  in->input_sequence.debug_stringify(seq);
                  qDebug(
                     "WARNING: One of the test bind trees contains an input sequence that may not be completable on a gamepad.\n   Node:     %s\n   Sequence: %s",
                     qUtf8Printable(in->name),
                     seq.c_str()
                  );
               }
            }
            for (auto* child : node.child_nodes()) {
               recurse(*child, recurse);
            }
         };
         recurse(*out.root, recurse);
      };
      if (dt == worldinput2::input_device_type::keyboard_mouse) {
         warn(out);
      }

      return out;
   }

   worldinput2::binds::nodes::bound_tool* make_tool_node(
      const std::string& name,
      worldinput2::button_press_type pt,
      const worldinput2::input_sequence& sequence
   ) {
      auto* node = new worldinput2::binds::nodes::bound_tool;
      node->name = name.c_str();
      node->button_press_type = pt;
      node->input_sequence    = sequence;
      return node;
   }
   worldinput2::binds::nodes::modifier* make_modifier_node(
      const std::string& name,
      const worldinput2::input_sequence& sequence,
      std::initializer_list<worldinput2::binds::node*> children = {}
   ) {
      auto* node = new worldinput2::binds::nodes::modifier;
      node->name = name.c_str();
      node->button_press_type = worldinput2::button_press_type::hold;
      node->input_sequence    = sequence;
      for (auto* child : children) {
         node->append(*child);
      }
      return node;
   }

   static auto testing_trees = std::array{
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Simple test tree",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press X",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Long Press Y",
                  worldinput2::button_press_type::long_press,
                  worldinput2::input_sequence::debug_from_string("Y")
               ),
               make_tool_node(
                  "Hold Z",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("Z")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Press/Long Press basic conflict",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press X",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Long Press X",
                  worldinput2::button_press_type::long_press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Press-delays-Hold conflict resolution",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press X",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Bind #2: Hold X",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Bind #3: Press [Y + Z]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[Y + Z]")
               ),
               make_tool_node(
                  "Bind #4: Hold Y",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("Y")
               ),
               make_tool_node(
                  "Bind #5: Hold Z",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("Z")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Hold-blocks-Press conflict resolution",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Hold [X + Z]",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("[X + Z]")
               ),
               make_tool_node(
                  "Press X",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/11/2023 - PASSES
         .name = "Basic gamepad tests",
         .tree = make_tree(
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Press A",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("A", true)
               ),
               make_tool_node(
                  "Press B",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("B", true)
               ),
               make_tool_node(
                  "Long Press X",
                  worldinput2::button_press_type::long_press,
                  worldinput2::input_sequence::debug_from_string("X", true)
               ),
               make_tool_node(
                  "Hold Y",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("Y", true)
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
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [LS + X]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[LS + X]", true)
               ),
               make_tool_node(
                  "Press [B + X]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[B + X]", true)
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
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [A + B]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B]")
               ),
               make_tool_node(
                  "Press (X + Y)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(X + Y)")
               ),
               make_tool_node(
                  "Press <J + K>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<J + K>")
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
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [A + B + X + Y]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Press (LB + Y)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(LB + Y)", true)
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
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Bind #2",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Seamless switching between Hold binds",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Hold A",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("A")
               ),
               make_tool_node(
                  "Hold (A + B)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(A + B)")
               ),
               make_tool_node(
                  "Hold (A + B + C)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(A + B + C)")
               ),
               make_tool_node(
                  "Hold (A + C)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(A + C)")
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
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [X + Z]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[X + Z]")
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
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Bind #1: Press [A + B + X + Y]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Bind #2: Press (Y + LB)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(Y + LB)", true)
               ),
               make_tool_node(
                  "Bind #3: Press (Y + LT)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(Y + LT)", true)
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Separate-and-ordered interruptions #1",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + D + F>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + D + F>")
               ),
               make_tool_node(
                  "Press <A + S + D + [F + <J + K>]>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + D + [F + <J + K>]>")
               ),
            }
         ),
      },
      testing_tree{ // 4/14/2023 - PASSES
         .name = "Press <A + S + D + [<J + K> + Z]>",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + D + [<J + K> + Z]>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + D + [<J + K> + Z]>")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 1",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold S",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("S")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 2",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold (K + S)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(K + S)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 3",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press <A + S + J>",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("<A + S + J>")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 4",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (X + <A + S + J>)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(X + <A + S + J>)")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold detail 5",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [X + <A + S + J>]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[X + <A + S + J>]")
               ),
               make_tool_node(
                  "Hold (K + <A + S>)",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("(K + <A + S>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press/Hold with common stem",
         .tree = make_tree(
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [X + Y + A]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[X + Y + A]", true)
               ),
               make_tool_node(
                  "Hold [X + Y + B]",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("[X + Y + B]", true)
               ),
            }
         ),
      },
      testing_tree{
         .name = "Advancing Press past conflict key indefinitely delays Hold?",
         .tree = make_tree(
            worldinput2::input_device_type::xinput,
            {
               make_tool_node(
                  "Press [A + B + X + Y]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B + X + Y]", true)
               ),
               make_tool_node(
                  "Hold B",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("B", true)
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press-delays-Hold: outlast one Press; advanced past by another",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [A + B]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B]")
               ),
               make_tool_node(
                  "Bind #2: Press [A + B + J]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + B + J]")
               ),
               make_tool_node(
                  "Bind #3: Hold B",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("B")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Edge-case: sequencing constraints on first item in group",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [B + [A + B]]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[B + [A + B]]")
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
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [A + A + B] // equivalent to [A + B]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + A + B]")
                  //
                  // Concurrent-and-ordered groups are stateless and allow you to advance 
                  // through multiple keys in a single frame, if those keys all went down 
                  // on that same frame. As such, when you have multiple of the same key 
                  // in a row, you advance through all of them at once.
                  //
               ),
               make_tool_node(
                  "Bind #2: Press [J + [J + K]] // equivalent to [J + K]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[J + [J + K]]")
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
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(X + (X + Y))")
               ),
               make_tool_node(
                  "Bind #4: Press (Q + [Q + W]) // equivalent to [Q + W]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(Q + [Q + W])")
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
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[E + (R + E)]")
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
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[T + <C + V> + <C + V>]")
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
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[1 + <2 + 3> + 3]")
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
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [A + <J + K> + <J + K>]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + <J + K> + <J + K>]")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press (A + <J + K> + <J + K>)",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (A + <J + K> + <J + K>)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(A + <J + K> + <J + K>)")
               ),
            }
         ),
      },
      testing_tree{
         .name = "Press (A + <B + A>) and Press (X + <X + Y>)",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press (A + <B + A>)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(A + <B + A>)")
               ),
               make_tool_node(
                  "Press (X + <X + Y>)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(X + <X + Y>)")
               ),
            }
         ),
      },
      #pragma region Modifier node tests
         testing_tree{
            .name = "Modifier parity: Press [A + B]",
            .tree = make_tree(
               worldinput2::input_device_type::keyboard_mouse,
               {
                  make_modifier_node(
                     "Press [A + ...]",
                     worldinput2::input_sequence::debug_from_string("A"),
                     {
                        make_tool_node(
                           "Press [A + B]",
                           worldinput2::button_press_type::press,
                           worldinput2::input_sequence::debug_from_string("B")
                        ),
                     }
                  ),
                  make_tool_node(
                     "Press [X + Y]",
                     worldinput2::button_press_type::press,
                     worldinput2::input_sequence::debug_from_string("[X + Y]")
                  ),
               }
            ),
         },
         testing_tree{
            .name = "Modifier parity: Press [(A + B) + C]",
            .tree = make_tree(
               worldinput2::input_device_type::keyboard_mouse,
               {
                  make_modifier_node(
                     "Press [(A + B) + ...]",
                     worldinput2::input_sequence::debug_from_string("(A + B)"),
                     {
                        make_tool_node(
                           "Press [(A + B) + C]",
                           worldinput2::button_press_type::press,
                           worldinput2::input_sequence::debug_from_string("C")
                        ),
                     }
                  ),
                  make_tool_node(
                     "Press [(X + Z) + Y]",
                     worldinput2::button_press_type::press,
                     worldinput2::input_sequence::debug_from_string("[(X + Z) + Y]")
                  ),
               }
            ),
         },
      #pragma endregion
   };
}

namespace DovahKitDebug::features {
   /*static*/ void worldinput2::execute(QWidget* from) {
      auto& core = dovahkit::subsystems::worldinput2::core::get();

      static QTimer poll_timer;
      static QPointer<QDialog> test_window = nullptr;
      QObject::connect(&poll_timer, &QTimer::timeout, []() {
         if (!test_window)
            return;
         if (!test_window->isVisible())
            return;

         double elapsed;
         ::worldinput2::combined_tool_results results;

         dovahkit::subsystems::worldinput2::core::get().doPerFrameInputProcessing(elapsed, results);

         auto* label = test_window->findChild<QLabel*>("debug_output");
         if (label) {

            static double no_empty_timer = 0.0;

            QString text = results.data.c_str();
            text = text.trimmed();

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
            auto& core  = dovahkit::subsystems::worldinput2::core::get();
            core.setBindingsFor(entry.tree.device_type, entry.tree);
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