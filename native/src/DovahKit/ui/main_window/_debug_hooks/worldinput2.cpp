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
         combo->setCurrentIndex(0);
      }
   }
}