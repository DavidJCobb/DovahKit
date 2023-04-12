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
         .name = "Press/Hold basic conflict",
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press X",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
               make_tool_node(
                  "Hold X",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("X")
               ),
            }
         ),
      },
      testing_tree{
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
      testing_tree{ // 4/11/2023 - PASSES
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
      testing_tree{ // 4/11/2023 - PASSES
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
      testing_tree{
         .name = "Specificity rule",
         //
         // Test procedure:
         // 
         //  - Press and hold Y.
         //  - Pres and release, in sequence, A through E.
         // 
         // Desired result: only the `Press [A + B + C + D + E]` bind activates.
         //
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Press [A + B + C + D + E]",
                  worldinput2::button_press_type::hold,
                  worldinput2::input_sequence::debug_from_string("[A + S + D + F]")
               ),
               make_tool_node(
                  "Press (Y + E)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(G + F)")
               ),
            }
         ),
      },
      testing_tree{
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
      testing_tree{
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
      testing_tree{
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
      testing_tree{
         .name = "Specificity rule, incl. across time",
         //
         // Multiple tests. 
         // 
         // Case A1:
         //  - G down.
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - F up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case A2:
         //  - G down.
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - A, S, or D up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //  - F up.
         //     - Bind 2 does not activate.
         // 
         // Case B:
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - A, S, or D up.
         //     - Bind 1 fires.
         //  - G down.
         //  - G up.
         //     - Bind 2 fires.
         // 
         // Case C1:
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - G down.
         //  - F up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case C2:
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - G down.
         //  - A, S, or D up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //  - G up.
         //     - Bind 2 does not activate.
         // 
         // Case D:
         //  - A down.
         //  - S down.
         //  - G down.
         //  - D down.
         //  - F down.
         //  - A, S, or D up.
         //     - Bind 1 fires.
         //     - Bind 2 is blocked.
         //  - G up.
         //     - Bind 2 does not activate.
         //  - F up.
         //     - Bind 2 does not activate.
         // 
         // Case E:
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - G down.
         //  - F up.
         //     - Bind 1 activates.
         //     - Bind 2 is blocked.
         //     - Bind 2 does not activate.
         // 
         // Case F:
         //  - G down.
         //  - A down.
         //  - S down.
         //  - D down.
         //  - F down.
         //  - G up.
         //     - Bind 2 fires.
         //  - A, S, D, or F up.
         //     - Bind 1 fires.
         // 
         // Case G:
         //  - G down.
         //  - H down.
         //  - F down.
         //  - F up.
         //     - Bind 2 and Bind 3 both lose a same-frame conflict.
         //     - Bind 2 does not activate.
         //     - Bind 3 does not activate.
         // 
         // Case H:
         //  - G down.
         //  - F down.
         //  - H down.
         //  - G up.
         //     - Bind 2 fires.
         //     - Bind 3 is blocked.
         //  - H up.
         //     - Bind 3 does not activate.
         //
         .tree = make_tree(
            worldinput2::input_device_type::keyboard_mouse,
            {
               make_tool_node(
                  "Bind #1: Press [A + S + D + F]",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("[A + S + D + F]")
               ),
               make_tool_node(
                  "Bind #2: Press (G + F)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(G + F)")
               ),
               make_tool_node(
                  "Bind #3: Press (H + F)",
                  worldinput2::button_press_type::press,
                  worldinput2::input_sequence::debug_from_string("(H + F)")
               ),
            }
         ),
      },
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