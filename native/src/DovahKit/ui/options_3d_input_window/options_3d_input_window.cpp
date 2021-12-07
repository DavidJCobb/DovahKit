#include "options_3d_input_window.h"
#pragma region Tool options
   #include "tool_options/tool_options_base.h"
   #include "tool_options/tool_options_debug_log.h"
   #include "tool_options/tool_options_debug_placeholder.h"
   #include "tool_options/tool_options_move_camera.h"
   #include "tool_options/tool_options_turn_camera.h"
#pragma endregion
#include "helpers/qt/combobox.h"
#include "dk3d/tools/_all.h"
#include "dk3d/Binding.h"
#include "dk3d/DK3DInputHandler.h"

namespace {
   DK3DToolOptions::Base* spawn_tool_options(DK3D::tool_id id, QWidget* parent = nullptr) {
      switch (id) {
         case DK3D::id_of_tool<DK3DToolOptions::DebugLog::options_type>():
            return new DK3DToolOptions::DebugLog(parent);
         case DK3D::id_of_tool<DK3DToolOptions::DebugPlaceholder::options_type>():
            return new DK3DToolOptions::DebugPlaceholder(parent);
         case DK3D::id_of_tool<DK3DToolOptions::MoveCamera::options_type>():
            return new DK3DToolOptions::MoveCamera(parent);
         case DK3D::id_of_tool<DK3DToolOptions::TurnCamera::options_type>():
            return new DK3DToolOptions::TurnCamera(parent);
      }
      return nullptr;
   }
   QString tool_name(DK3D::tool_id id) {
      switch (id) {
         case DK3D::tools::id_of_none:
            return Options3DInputDialog::tr("None", "tool name");
            //
         case DK3D::id_of_tool<DK3D::tools::debug_log>():
            return Options3DInputDialog::tr("Debug Log", "tool name");
         case DK3D::id_of_tool<DK3D::tools::debug_placeholder>():
            return Options3DInputDialog::tr("Placeholder", "tool name");
         case DK3D::id_of_tool<DK3D::tools::move_camera>():
            return Options3DInputDialog::tr("Move Camera", "tool name");
         case DK3D::id_of_tool<DK3D::tools::turn_camera>():
            return Options3DInputDialog::tr("Turn Camera", "tool name");
      }
      return Options3DInputDialog::tr("<unknown>", "tool name");
   }

   QString stringify_input(const DK3D::BoundInput& bi) {
      if (bi.is_boolean()) {
         QString mod;
         switch (bi.boolean.type) {
            using _ = DK3D::BooleanInputMod;
            case _::Tap:
               mod = Options3DInputDialog::tr("Tap", "boolean input mod");
               break;
            case _::Hold:
               mod = Options3DInputDialog::tr("Hold", "boolean input mod");
               break;
            case _::While:
               mod = Options3DInputDialog::tr("While", "boolean input mod");
               break;
         }
         QString button;
         if (!bi.boolean.key.empty()) {
            button = bi.boolean.key.toString();
         } else if (bi.boolean.gamepad.button != DK3D::XInputKey::None) {
            switch (bi.boolean.gamepad.button) {
               using _ = DK3D::XInputKey;
               case _::A:
                  button = Options3DInputDialog::tr("A", "XInput button");
                  break;
               case _::B:
                  button = Options3DInputDialog::tr("B", "XInput button");
                  break;
               case _::X:
                  button = Options3DInputDialog::tr("X", "XInput button");
                  break;
               case _::Y:
                  button = Options3DInputDialog::tr("Y", "XInput button");
                  break;
               case _::LT:
                  button = Options3DInputDialog::tr("LT", "XInput button");
                  break;
               case _::RT:
                  button = Options3DInputDialog::tr("RT", "XInput button");
                  break;
               case _::LB:
                  button = Options3DInputDialog::tr("LB", "XInput button");
                  break;
               case _::RB:
                  button = Options3DInputDialog::tr("RB", "XInput button");
                  break;
               case _::LS:
                  button = Options3DInputDialog::tr("LS-Click", "XInput button");
                  break;
               case _::RS:
                  button = Options3DInputDialog::tr("RS-Click", "XInput button");
                  break;
               case _::DPadUp:
                  button = Options3DInputDialog::tr("D-Pad Up", "XInput button");
                  break;
               case _::DPadDown:
                  button = Options3DInputDialog::tr("D-Pad Down", "XInput button");
                  break;
               case _::DPadLeft:
                  button = Options3DInputDialog::tr("D-Pad Left", "XInput button");
                  break;
               case _::DPadRight:
                  button = Options3DInputDialog::tr("D-Pad Right", "XInput button");
                  break;
               case _::Back:
                  button = Options3DInputDialog::tr("Back", "XInput button");
                  break;
               case _::Start:
                  button = Options3DInputDialog::tr("Start", "XInput button");
                  break;
            }
         } else if (bi.boolean.mouse.button != Qt::MouseButton::NoButton) {
            switch (bi.boolean.mouse.button) {
               using _ = Qt::MouseButton;
               case _::LeftButton:
                  button = Options3DInputDialog::tr("LMB", "mouse button");
                  break;
               case _::RightButton:
                  button = Options3DInputDialog::tr("RMB", "mouse button");
                  break;
               case _::MiddleButton:
                  button = Options3DInputDialog::tr("MMB", "mouse button");
                  break;
               case _::XButton1:
                  button = Options3DInputDialog::tr("Mouse-X1", "mouse button");
                  break;
               case _::XButton2:
                  button = Options3DInputDialog::tr("Mouse-X2", "mouse button");
                  break;
            }
         }
         //
         return Options3DInputDialog::tr("%1 %2", "format string for button inputs").arg(mod).arg(button);
      }
      if (bi.is_scalar()) {
         QString control;
         //
         bool has_axis = false;
         switch (bi.scalar.input) {
            using _ = DK3D::ScalarControl;
            case _::MouseMove:
               control  = Options3DInputDialog::tr("Mouse-Move", "scalar control");
               has_axis = true;
               break;
            case _::XInput_LS:
               control  = Options3DInputDialog::tr("LS", "scalar control");
               has_axis = true;
               break;
            case _::XInput_RS:
               control  = Options3DInputDialog::tr("RS", "scalar control");
               has_axis = true;
               break;
            case _::XInput_LT:
               control = Options3DInputDialog::tr("LT", "scalar control");
               break;
            case _::XInput_RT:
               control = Options3DInputDialog::tr("RT", "scalar control");
               break;
         }
         if (has_axis) {
            QString axis;
            switch (bi.scalar.axis) {
               using _ = DK3D::Axis2D;
               case _::X:
                  axis = Options3DInputDialog::tr("Left/Right", "scalar axis");
                  break;
               case _::Y:
                  axis = Options3DInputDialog::tr("Up/Down", "scalar axis");
                  break;
            }
            if (!axis.isEmpty())
               return Options3DInputDialog::tr("%1 %2", "scalar input format string").arg(control).arg(axis);
         }
         return control;
      }
      if (bi.is_vector()) {
         QString control;
         switch (bi.vector.input) {
            using _ = DK3D::VectorControl;
            case _::MouseMove:
               return Options3DInputDialog::tr("Mouse-Move", "vector control");
            case _::XInput_LS:
               return Options3DInputDialog::tr("LS", "vector control");
            case _::XInput_RS:
               return Options3DInputDialog::tr("RS", "vector control");
         }
      }
      return Options3DInputDialog::tr("<none>");
   }
}

Options3DInputDialog* Options3DInputDialog::current_instance = nullptr;

Options3DInputDialog::Options3DInputDialog(QWidget* parent) : QDialog(parent) {
   Options3DInputDialog::current_instance = this;
   this->ui.setupUi(this);
   //
   {  // list
      auto* widget = this->ui.bindList;
      widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      widget->setWordWrap(false);
      if (auto* vh = widget->verticalHeader()) {
         vh->setHidden(true);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      if (auto* hh = widget->horizontalHeader()) { // TODO: use my custom "flex" header instead, for better control
         hh->setStretchLastSection(true);
         widget->setColumnCount(3);
         widget->setHorizontalHeaderLabels({ tr("Name"), tr("Tool"), tr("Mapped to") });
      }
      //
      if (auto* sm = widget->selectionModel()) {
         QObject::connect(sm, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            if (!current.isValid()) {
               this->state.selected_binding = -1;
            } else {
               this->state.selected_binding = current.row();
            }
            this->_updateBindListButtons();
            this->bindingSelected();
         });
      }
      QObject::connect(this->ui.buttonNewBind,      &QPushButton::clicked, this, &Options3DInputDialog::addBind);
      QObject::connect(this->ui.buttonDeleteBind,   &QPushButton::clicked, this, &Options3DInputDialog::deleteBind);
      QObject::connect(this->ui.buttonMoveBindUp,   &QPushButton::clicked, this, &Options3DInputDialog::moveBindUp);
      QObject::connect(this->ui.buttonMoveBindDown, &QPushButton::clicked, this, &Options3DInputDialog::moveBindDown);
   }
   {  // tools
      constexpr const char* disamb = "tool name";
      //
      auto* widget = this->ui.bindTool;
      widget->clear();
      auto lambda = [widget](DK3D::tool_id id) {
         widget->addItem(tool_name(id), (int)id);
      };
      lambda(DK3D::tools::id_of_none);
      lambda(DK3D::id_of_tool<DK3D::tools::debug_log>());
      lambda(DK3D::id_of_tool<DK3D::tools::debug_placeholder>());
      lambda(DK3D::id_of_tool<DK3D::tools::move_camera>());
      lambda(DK3D::id_of_tool<DK3D::tools::turn_camera>());
   }
   {  // input
      auto* widget = this->subwidgets.input = new DKBoundInputWidget(this);
      auto* layout = qobject_cast<QGridLayout*>(this->ui.bindOptions->layout());
      assert(layout);
      layout->addWidget(widget, 2, 0, 1, 2);
      QObject::connect(widget, &DKBoundInputWidget::valueChanged, this, [this](const DK3D::BoundInput& bi) {
         if (auto* b = this->selectedBinding()) {
            b->input = bi;
            this->_updateBindListRow(this->state.selected_binding);
         }
      });
   }
   this->ui.bindOptions->setEnabled(false);
   //
   QObject::connect(this->ui.bindName, &QLineEdit::textEdited, this, [this](const QString& name) {
      if (auto* b = this->selectedBinding()) {
         b->name = name;
         this->_updateBindListRow(this->state.selected_binding);
      }
   });
   QObject::connect(this->ui.bindTool, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      if (auto* b = this->selectedBinding()) {
         auto id = (DK3D::tool_id)this->ui.bindTool->currentData().toInt();
         b->function = DK3D::all_tool_instances::get()[id];
         b->params   = DK3D::tools::option_union::construct_for_type(id);
         this->rebuildToolOptions();
         this->_updateBindListRow(this->state.selected_binding);
      }
   });
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, [this]() {
      DK3DInputHandler::get().setBindingsFor(DK3D::InputDevice::KeyboardMouse, this->state.bindings);
      this->accept();
   });
   //
   this->setBindings(DK3DInputHandler::get().bindingsFor(DK3D::InputDevice::KeyboardMouse)); // TODO: make it possible to choose
}

/*static*/ Options3DInputDialog* Options3DInputDialog::open(QWidget* parent) {
   if (!current_instance) {
      current_instance = new Options3DInputDialog(parent);
      current_instance->show();
   } else {
      current_instance->raise();
      current_instance->activateWindow();
   }
   return current_instance;
}
Options3DInputDialog::~Options3DInputDialog() {
   Options3DInputDialog::current_instance = nullptr;
}

void Options3DInputDialog::setBindings(QVector<DK3D::Binding> list) {
   this->state.bindings = list;
   this->state.selected_binding = -1;
   this->_updateBindList();
}

void Options3DInputDialog::_updateBindList() {
   auto* widget = this->ui.bindList;
   auto* sm     = widget->selectionModel();
   auto& list   = this->state.bindings;
   auto  size   = list.size();
   //
   const auto blocker0 = QSignalBlocker(widget);
   const auto blocker1 = QSignalBlocker(sm);
   //
   widget->setRowCount(0);
   widget->setRowCount(size);
   widget->setColumnCount(3);
   auto& all_tool_instances = DK3D::all_tool_instances::get();
   for (int i = 0; i < size; ++i) {
      const auto& binding = list[i];
      //
      DK3D::tool_id id = DK3D::tools::id_of_none;
      if (binding.function)
         id = all_tool_instances.id_of(*binding.function);
      //
      auto* name = new QTableWidgetItem(binding.name);
      auto* tool = new QTableWidgetItem(tool_name(id));
      auto* bind = new QTableWidgetItem(stringify_input(binding.input));
      widget->setItem(i, 0, name);
      widget->setItem(i, 1, tool);
      widget->setItem(i, 2, bind);
   }
   //
   if (sm) {
      if (this->state.selected_binding < 0) {
         sm->clear();
      } else {
         QModelIndex qmi = widget->model()->index(this->state.selected_binding, 0);
         sm->setCurrentIndex(qmi, QItemSelectionModel::ClearAndSelect);
      }
   }
   this->_updateBindListButtons();
   this->bindingSelected();
}
void Options3DInputDialog::_updateBindListRow(int i) {
   auto* widget = this->ui.bindList;
   if (i < 0 || i >= widget->rowCount())
      return;
   const auto& list = this->state.bindings;
   if (i >= list.size())
      return;
   auto& bind = list[i];
   //
   if (auto* item = widget->item(i, 0))
      item->setText(bind.name);
   if (auto* item = widget->item(i, 1)) {
      DK3D::tool_id id = DK3D::tools::id_of_none;
      if (bind.function)
         id = DK3D::all_tool_instances::get().id_of(*bind.function);
      //
      item->setText(tool_name(id));
   }
   if (auto* item = widget->item(i, 2)) {
      item->setText(stringify_input(bind.input));
   }
}
void Options3DInputDialog::_updateBindListButtons() {
   auto& list = this->state.bindings;
   auto  size = list.size();
   auto  i    = this->state.selected_binding;
   if (i < 0) {
      this->ui.buttonDeleteBind->setEnabled(false);
      this->ui.buttonMoveBindUp->setEnabled(false);
      this->ui.buttonMoveBindDown->setEnabled(false);
   } else {
      this->ui.buttonDeleteBind->setEnabled(true);
      this->ui.buttonMoveBindUp->setEnabled(i > 0);
      this->ui.buttonMoveBindDown->setEnabled(i < size - 1);
   }
}
DK3D::Binding* Options3DInputDialog::selectedBinding() {
   if (this->state.selected_binding >= 0) {
      return &this->state.bindings[this->state.selected_binding];
   }
   return nullptr;
}

void Options3DInputDialog::bindingSelected() {
   const auto blocker0 = QSignalBlocker(this->ui.bindName);
   const auto blocker1 = QSignalBlocker(this->ui.bindTool);
   const auto blocker2 = QSignalBlocker(this->subwidgets.input);
   //
   const DK3D::Binding* binding = this->selectedBinding();
   this->ui.bindOptions->setEnabled(binding != nullptr);
   if (!binding) {
      this->ui.bindName->setText("");
      this->ui.bindTool->setCurrentIndex(0);
      this->subwidgets.input->setValue(DK3D::BoundInput());
      this->rebuildToolOptions();
      return;
   }
   this->ui.bindName->setText(binding->name);
   {
      auto* widget = this->subwidgets.input;
      widget->setValue(binding->input);
   }
   if (binding->function) {
      this->rebuildToolOptions();
   } else {
      cobb::qt::set_combobox_value(this->ui.bindTool, DK3D::tools::id_of_none);
      //
      if (auto* w = this->subwidgets.tool_options) {
         w->setParent(nullptr);
         w->deleteLater();
         this->subwidgets.tool_options = nullptr;
      }
   }
}
void Options3DInputDialog::rebuildToolOptions() {
   const DK3D::Binding* binding = this->selectedBinding();
   auto*  body = this->ui.bindOptions;
   auto*& to   = this->subwidgets.tool_options;
   if (to) {
      to->setParent(nullptr);
      to->deleteLater();
   }
   if (!binding) {
      to = nullptr;
      return;
   }
   //
   auto& list = DK3D::all_tool_instances::get();
   auto  id   = list.id_of(*binding->function);
   //
   to = spawn_tool_options(id, body);
   if (to) {
      to->showOptions(binding->params);
      QObject::connect(to, &DK3DToolOptions::Base::edited, this, [this]() {
         auto* b = this->selectedBinding();
         if (!b)
            return;
         this->subwidgets.tool_options->writeTo(b->params);
      });
      if (auto* layout = qobject_cast<QGridLayout*>(body->layout())) {
         layout->addWidget(to, 3, 0, 1, 2);
      }
   }
}

void Options3DInputDialog::addBind() {
   DK3D::Binding added;
   //
   auto& list = this->state.bindings;
   auto  size = list.size();
   auto  i    = this->state.selected_binding;
   if (i < 0 || i == size - 1) {
      list.push_back(added);
      this->state.selected_binding = size;
   } else {
      list.insert(i + 1, added);
      this->state.selected_binding = i + 1;
   }
   this->_updateBindList();
}
void Options3DInputDialog::deleteBind() {
   auto i = this->state.selected_binding;
   if (i < 0)
      return;
   auto& list = this->state.bindings;
   auto  size = list.size();
   list.removeAt(i);
   if (i == size - 1) {
      --this->state.selected_binding;
   }
   //
   this->_updateBindList();
}
void Options3DInputDialog::moveBind(int by) {
   if (!by)
      return;
   auto  from = this->state.selected_binding;
   if (from < 0)
      return;
   auto& list = this->state.bindings;
   auto  size = list.size();
   //
   int to = from + by;
   if (by < 0) {
      to = std::max(0, to);
   } else {
      to = std::min(size - 1, to);
   }
   list.move(from, to);
   this->state.selected_binding = to;
   this->_updateBindList();
}
void Options3DInputDialog::moveBindUp() {
   this->moveBind(-1);
}
void Options3DInputDialog::moveBindDown() {
   this->moveBind(1);
}