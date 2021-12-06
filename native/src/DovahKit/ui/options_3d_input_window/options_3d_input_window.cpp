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
      widget->verticalHeader()->setHidden(true);
      //
      if (auto* sm = widget->selectionModel()) {
         QObject::connect(sm, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            if (!current.isValid()) {
               this->state.selected_binding = -1;
            } else {
               this->state.selected_binding = current.row();
            }
            this->bindingSelected();
         });
      }
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
   }
   this->ui.bindOptions->setEnabled(false);
   //
   QObject::connect(this->ui.bindName, &QLineEdit::textEdited, this, [this](const QString& name) {
      if (auto* b = this->selectedBinding())
         b->name = name;
   });
   QObject::connect(this->ui.bindTool, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      if (auto* b = this->selectedBinding()) {
         auto id = (DK3D::tool_id)this->ui.bindTool->currentData().toInt();
         b->function = DK3D::all_tool_instances::get()[id];
         b->params   = DK3D::tools::option_union::construct_for_type(id);
         this->rebuildToolOptions();
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
      auto* bind = new QTableWidgetItem();
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
   this->bindingSelected();
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