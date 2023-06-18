#include "options_3d_input_window.h"
#include "helpers/qt/combobox.h"

#include "editor/subsystems/worldinput/bind_tree/tree.h"
#include "editor/subsystems/worldinput/bind_tree/node.h"
#include "editor/subsystems/worldinput/bind_tree/nodes/input.h"
#include "editor/subsystems/worldinput/tools/_all.h"
#include "editor/subsystems/worldinput/inputs/bound_input.h"
#include "editor/subsystems/worldinput/core.h"

#include "./tool_options/tool_options_base.h"
#include "./tool_options/_all.h"
#include "./Options3DControlSchemeModel.h"
#include "./localization.h"

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

namespace {
   //
   // TODO: This window should be used for editing a single control scheme; the user should be able to 
   // pick the device and scheme to edit somewhere, so this should be set on the window as a parameter.
   //
   constexpr auto HARDCODED_INPUT_DEVICE_TODO_CHANGE = worldinput::input_device_type::xinput;
}

namespace {
   DK3DToolOptions::Base* spawn_tool_options(worldinput::tool_id id, QWidget* parent = nullptr) {
      DK3DToolOptions::Base* result = nullptr;

      DK3DToolOptions::all::for_each_until_true([id, parent, &result]<typename Current>() {
         constexpr auto current_id = worldinput::id_of_tool<typename Current::tool_type>();
         if (current_id == id) {
            result = new Current(parent);
            return true;
         }
         return false;
      });

      return result;
   }
}

Options3DInputDialog* Options3DInputDialog::current_instance = nullptr;

Options3DInputDialog::Options3DInputDialog(QWidget* parent) : QDialog(parent) {
   Options3DInputDialog::current_instance = this;
   this->ui.setupUi(this);
   //
   this->state.model = new model_type(this);
   {  // tree
      auto* widget = this->ui.tree;
      {
         auto* proxy = new Options3DControlSchemeTreeModel;
         proxy->setSourceModel(this->state.model);
         widget->setModel(proxy);
      }
      widget->setColumnHidden(1, true);
      widget->setColumnHidden(2, true);
      widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      if (auto* hh = widget->header()) {
         hh->setHidden(true);
      }
      //
      if (auto* sm = widget->selectionModel()) {
         QObject::connect(sm, &QItemSelectionModel::currentRowChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            auto* nav   = this->ui.tree;
            auto* proxy = (QSortFilterProxyModel*)nav->model();
            auto  qmi   = proxy->mapToSource(current);
            if (!qmi.isValid()) {
               qmi = this->state.model->index(0, 0); // ensure the "real root" is used
            }
            this->ui.bindList->setRootIndex(qmi);
            this->_updateBindListButtons();
            this->bindingSelected();
         });
      }
   }
   {  // list
      auto* widget = this->ui.bindList;
      widget->setModel(this->state.model);
      this->ui.bindList->setRootIndex(this->state.model->index(0, 0));
      widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      widget->setWordWrap(false);
      if (auto* vh = widget->verticalHeader()) {
         vh->setHidden(true);
         vh->setSectionResizeMode(QHeaderView::ResizeToContents);
      }
      if (auto* hh = widget->horizontalHeader()) { // TODO: use my custom "flex" header instead, for better control
         hh->setStretchLastSection(true);
      }
      //
      QObject::connect(this->state.model, &QAbstractItemModel::modelReset, this, [this]() {
         this->ui.bindList->setRootIndex(this->state.model->index(0, 0));
      });
      if (auto* sm = widget->selectionModel()) {
         QObject::connect(sm, &QItemSelectionModel::selectionChanged, [this](const QItemSelection& selected, const QItemSelection& deselected) {
            //
            // QItemSelectionModel::currentRowChanged and friends seem to fire too early, such that the 
            // current selection (if retrieved from anywhere else) will still be out of date. This isn't 
            // documented anywhere. The selectionChanged signal appears to be the only safe one.
            //
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
      widget->addItem(DKWorldinputLocalization::tool_name(worldinput::tools::id_of_none), (int)worldinput::tools::id_of_none);
      DK3DToolOptions::all::for_each([this, widget]<typename CurrentUI>() {
         constexpr auto tool_id = worldinput::id_of_tool<typename CurrentUI::tool_type>();
         widget->addItem(DKWorldinputLocalization::tool_name(tool_id), (int)tool_id);
      });
   }
   {  // input
      auto* widget = this->subwidgets.input = new DKBoundInputWidget(this);
      auto* layout = qobject_cast<QGridLayout*>(this->ui.bindOptions->layout());
      assert(layout);
      layout->addWidget(widget, 2, 0, 1, 2);
      widget->setInputDevice(HARDCODED_INPUT_DEVICE_TODO_CHANGE);
      QObject::connect(widget, &DKBoundInputWidget::valueChanged, this, [this](const worldinput::inputs::bound_input& bi) {
         auto* widget = this->ui.bindList;
         auto* model  = this->state.model;
         auto* sm     = widget->selectionModel();
         if (!model || !sm)
            return;
         auto rows = sm->selectedRows();
         if (rows.isEmpty())
            return;
         auto qmi = rows[0];
         model->setData(qmi, QVariant::fromValue(bi), model_type::BoundInputRole);
      });
   }
   this->ui.bindOptions->setEnabled(false);
   //
   QObject::connect(this->ui.bindName, &QLineEdit::textEdited, this, [this](const QString& name) {
      auto* widget = this->ui.bindList;
      auto* model  = this->state.model;
      auto* sm     = widget->selectionModel();
      if (!model || !sm)
         return;
      auto rows = sm->selectedRows();
      if (rows.isEmpty())
         return;
      auto qmi = rows[0];
      model->setData(qmi, name, model_type::InputNodeNameRole);
   });
   QObject::connect(this->ui.bindTool, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int i) {
      auto* widget = this->ui.bindList;
      auto* model  = this->state.model;
      auto* sm     = widget->selectionModel();
      if (!model || !sm)
         return;
      auto rows = sm->selectedRows();
      if (rows.isEmpty())
         return;
      auto qmi = rows[0];
      //
      model->setData(qmi, this->ui.bindTool->currentData().toInt(), model_type::InputNodeToolRole);
      this->rebuildToolOptions();
   });
   //
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, [this]() {
      worldinput::core::get().setBindingsFor(HARDCODED_INPUT_DEVICE_TODO_CHANGE, this->state.model->tree());
      this->accept();
   });
   //
   this->setBindings(worldinput::core::get().bindingsFor(HARDCODED_INPUT_DEVICE_TODO_CHANGE));
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

void Options3DInputDialog::setBindings(const worldinput::binds::tree& tree) {
   this->state.model->setTree(tree);
}

void Options3DInputDialog::_updateBindListButtons(const QModelIndex& current) {
   int size = 0;
   int i    = -1;
   if (current.isValid()) {
      auto parent = this->state.model->parent(current);
      //
      i    = current.row();
      size = this->state.model->rowCount(parent);
   } else {
      auto* widget = this->ui.bindList;
      auto* model  = this->state.model;
      auto* sm     = widget->selectionModel();
      if (model && sm) {
         auto rows = sm->selectedRows();
         if (!rows.isEmpty()) {
            auto qmi    = rows[0];
            auto parent = model->parent(qmi);
            //
            i    = qmi.row();
            size = model->rowCount(parent);
         }
      }
   }
   //
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

worldinput::binds::nodes::input* Options3DInputDialog::selectedInputNode() const {
   auto* sm = this->ui.bindList->selectionModel();
   if (!sm)
      return nullptr;
   auto rows = sm->selectedRows();
   if (rows.isEmpty())
      return nullptr;
   auto* raw = this->state.model->node(rows[0]);
   if (raw)
      return raw->as<worldinput::binds::nodes::input>();
   return nullptr;
}

void Options3DInputDialog::bindingSelected() {
   const auto blocker0 = QSignalBlocker(this->ui.bindName);
   const auto blocker1 = QSignalBlocker(this->ui.bindTool);
   const auto blocker2 = QSignalBlocker(this->subwidgets.input);
   //
   const worldinput::binds::nodes::input* node = this->selectedInputNode();
   this->ui.bindOptions->setEnabled(node != nullptr);
   if (!node) {
      this->ui.bindName->setText("");
      this->ui.bindTool->setCurrentIndex(0);
      this->subwidgets.input->setValue(worldinput::inputs::bound_input());
      this->rebuildToolOptions();
      return;
   }
   this->ui.bindName->setText(node->name);
   {
      auto* widget = this->subwidgets.input;
      widget->setValue(node->mapping);
   }
   if (node->tool) {
      this->rebuildToolOptions();
      cobb::qt::set_combobox_value(this->ui.bindTool, worldinput::id_of_tool(node->tool));
   } else {
      cobb::qt::set_combobox_value(this->ui.bindTool, worldinput::tools::id_of_none);
      //
      if (auto* w = this->subwidgets.tool_options) {
         w->setParent(nullptr);
         w->deleteLater();
         this->subwidgets.tool_options = nullptr;
      }
   }
}
void Options3DInputDialog::rebuildToolOptions(worldinput::binds::nodes::input* node) {
   if (!node)
      node = this->selectedInputNode();
   //
   auto*  body = this->ui.bindOptions;
   auto*& to   = this->subwidgets.tool_options;
   if (to) {
      to->setParent(nullptr);
      to->deleteLater();
   }
   if (!node || !node->tool) {
      to = nullptr;
      return;
   }
   //
   auto& list = worldinput::all_tool_instances::get();
   auto  id   = list.id_of(*node->tool);
   //
   to = spawn_tool_options(id, body);
   if (to) {
      to->setControlType(node->mapping.get_control_type());
      to->showOptions(node->params);
      QObject::connect(to, &DK3DToolOptions::Base::edited, this, [this]() {
         auto* b = this->selectedInputNode();
         if (!b)
            return;
         this->subwidgets.tool_options->writeTo(b->params);
         this->state.model->setNodeEdited(b);
      });
      if (auto* layout = qobject_cast<QGridLayout*>(body->layout())) {
         layout->addWidget(to, 3, 0, 1, 2);
      }
   }
}

void Options3DInputDialog::addBind() {
   auto  qmi = this->ui.bindList->rootIndex();
   auto* sm  = this->ui.bindList->selectionModel();
   int   row;
   {
      if (sm) {
         auto rows = sm->selectedRows();
         if (!rows.isEmpty())
            row = rows[0].row();
         else
            row = this->state.model->rowCount(qmi);
      } else {
         row = this->state.model->rowCount(qmi);
      }
   }
   this->state.model->insertRow(row, qmi);
   //
   if (sm) {
      sm->setCurrentIndex(this->state.model->index(row, 0, qmi), QItemSelectionModel::SelectionFlag::ClearAndSelect);
   }
}
void Options3DInputDialog::deleteBind() {
   int row;
   {
      auto* sm = this->ui.bindList->selectionModel();
      if (!sm)
         return;
      auto rows = sm->selectedRows();
      if (rows.isEmpty())
         return;
      row = rows[0].row();
   }
   auto qmi = this->ui.bindList->rootIndex();
   this->state.model->removeRow(row, qmi);
}
void Options3DInputDialog::moveBind(int by) {
   if (!by)
      return;
   int row;
   {
      auto* model = this->ui.bindList->model();
      auto* sm    = this->ui.bindList->selectionModel();
      if (!sm)
         return;
      auto  rows  = sm->selectedRows();
      if (rows.isEmpty())
         return;
      row = rows[0].row();
   }
   auto qmi = this->ui.bindList->rootIndex();
   this->state.model->nonCursedMoveRow(qmi, row, qmi, by);
}
void Options3DInputDialog::moveBindUp() {
   this->moveBind(-1);
}
void Options3DInputDialog::moveBindDown() {
   this->moveBind(1);
}