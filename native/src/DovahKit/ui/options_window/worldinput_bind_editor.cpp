#include "./worldinput_bind_editor.h"
#include <QAction>
#include <QMenu>
#include "helpers/qt/combobox.h"
#include "widgets/DKHeaderView.h"

#include "editor/subsystems/worldinput2/control_scheme/action.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "ui/models/worldinput/DKWorldinputInputSequenceModel.h"
#include "widgets/widget-dialogs/DKWorldinputButtonPickDialog.h"

#include "./worldedit_tools/get_worldedit_tool_info.h"
#include "./worldedit_tools/options_widget_dispatch_table.h"

WorldinputBindEditDialog::WorldinputBindEditDialog(input_device_type device_type, QWidget* parent) : QDialog(parent), device_type(device_type) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   this->ui.name->setMaxLength(data_type::max_name_length);

   #pragma region Inputs tab
      {
         auto* w_bpt = this->ui.buttonPressType;
         w_bpt->clear();

         w_bpt->addItem(tr("Press"), (int)button_press_type::press);
         w_bpt->addItem(tr("Long Press"), (int)button_press_type::long_press);
         w_bpt->addItem(tr("Hold"), (int)button_press_type::hold);
      }
      {
         auto* w_control = this->ui.rangeControlType;
         auto* w_axes    = this->ui.rangeControlAxis;
         w_control->clear();
         w_axes->clear();

         w_control->addItem(tr("None"), (int)range_input_control::none);
         switch (this->device_type) {
            case input_device_type::keyboard_mouse:
               w_control->addItem(tr("Mouse Move"), (int)range_input_control::mouse_move);
               break;
            case input_device_type::xinput:
               w_control->addItem(tr("Left Stick"),    (int)range_input_control::xinput_ls);
               w_control->addItem(tr("Right Stick"),   (int)range_input_control::xinput_rs);
               w_control->addItem(tr("Left Trigger"),  (int)range_input_control::xinput_lt);
               w_control->addItem(tr("Right Trigger"), (int)range_input_control::xinput_rt);
               break;
         }

         w_axes->addItem(tr("Both Axes"), (int)range_input_axes::all);
         w_axes->addItem(tr("X-Axis"),    (int)range_input_axes::x);
         w_axes->addItem(tr("Y-Axis"),    (int)range_input_axes::y);

         QObject::connect(w_control, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
            if (index < 0) {
               this->ui.rangeControlAxis->setCurrentIndex(0);
               this->ui.rangeControlAxis->setEnabled(false);
               return;
            }
            auto ctrl = (range_input_control)this->ui.rangeControlType->currentData().toInt();
            if (dovahkit::subsystems::worldinput::range_input_control_has_multiple_axes(ctrl)) {
               this->ui.rangeControlAxis->setEnabled(true);
            } else {
               this->ui.rangeControlAxis->setCurrentIndex(0);
               this->ui.rangeControlAxis->setEnabled(false);
            }
         });
      }

      {
         auto* treeview = this->ui.treeView;
         treeview->setModel(new DKWorldinputInputSequenceModel(treeview));
         treeview->setSelectionMode(QAbstractItemView::SelectionMode::ExtendedSelection);
         treeview->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
         treeview->setDragDropMode(QAbstractItemView::DragDropMode::InternalMove);
         treeview->expandAll();

         // QHeaderView sucks, and is bad, so replace it with this
         auto* header = new DKHeaderView(Qt::Orientation::Horizontal, treeview);
         treeview->setHeader(header);

         header->setFlexResizeEnabled(true);
         header->setColumnFlex(0, 1, 0);
         header->setColumnFlex(1, 0, 0, 16);
         header->setSectionResizeMode(QHeaderView::ResizeMode::Fixed);

         #pragma region Treeview context menu
            treeview->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
            {
               auto* item = this->_treeview_context_menu.set_raycast_associated = new QAction(tr("Raycast-associated"), this);
               QObject::connect(item, &QAction::triggered, this, [this]() {
                  auto* model = this->_getInputSequenceModel();
                  auto  qmi   = this->_getFirstSeqSelection();
                  
                  auto info_opt = model->infoFor(qmi);
                  if (!info_opt.has_value())
                     return;

                  const auto& info = info_opt.value();
                  if (info.type != input_sequence::group_type::single_control)
                     return;

                  bool already = model->raycastAssociatedButton() == qmi;
                  model->setRaycastAssociatedButton(already ? QModelIndex{} : qmi);
               });
            }
            QObject::connect(treeview, &QWidget::customContextMenuRequested, this, [this](const QPoint& pos) {
               const auto* opener = this->ui.treeView;
               const auto& items  = this->_treeview_context_menu;

               auto* model = this->_getInputSequenceModel();
               auto  qmi   = this->_getFirstSeqSelection();

               QMenu menu;
               menu.addAction(items.set_raycast_associated);

               {
                  auto* item = items.set_raycast_associated;

                  item->setEnabled(false);
                  item->setVisible(false);
                  auto info_opt = model->infoFor(qmi);
                  if (info_opt.has_value()) {
                     const auto& info = info_opt.value();
                     if (info.type == input_sequence::group_type::single_control) {
                        item->setEnabled(true);
                        item->setVisible(true);
                        if (model->isRaycastAssociatedButton(qmi)) {
                           item->setText(tr("Unset as raycast-associated"));
                        } else {
                           item->setText(tr("Make raycast-associated"));
                        }
                     }
                  }
               }

               if (menu.isEmpty())
                  return; // don't show an empty menu
               menu.exec(opener->mapToGlobal(pos));
            });
         #pragma endregion
      }
      #pragma region Treeview hierarchy edit buttons
         QObject::connect(this->ui.inputSeqAddButton, &QPushButton::clicked, this, [this]() {
            auto created = this->_getInputSequenceModel()->addButtonTo(this->_getFirstSeqSelection());
            if (created.has_value()) {
               auto* sm = this->ui.treeView->selectionModel();
               if (sm)
                  sm->select(created.value(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.inputSeqAddGroup, &QPushButton::clicked, this, [this]() {
            auto created = this->_getInputSequenceModel()->addGroupTo(this->_getFirstSeqSelection());
            if (created.has_value()) {
               auto* sm = this->ui.treeView->selectionModel();
               if (sm)
                  sm->select(created.value(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.inputSeqMoveUp, &QPushButton::clicked, this, [this]() {
            this->_getInputSequenceModel()->moveItems(this->_getSeqSelection(), -1);
         });
         QObject::connect(this->ui.inputSeqMoveDown, &QPushButton::clicked, this, [this]() {
            this->_getInputSequenceModel()->moveItems(this->_getSeqSelection(), 1);
         });
         QObject::connect(this->ui.inputSeqDelete, &QPushButton::clicked, this, [this]() {
            this->_getInputSequenceModel()->deleteItems(this->_getSeqSelection().indexes());
         });
      #pragma endregion
      #pragma region Editing controls for selected treeview item
         this->ui.inputSeqEditSelection->setEnabled(false);
         {
            auto* widget = this->ui.inputSeqGroupType;
            widget->clear();

            widget->addItem(tr("Concurrent and ordered"), (int)input_sequence::group_type::concurrent_ordered);
            widget->addItem(tr("Concurrent and unordered"), (int)input_sequence::group_type::concurrent_unordered);
            widget->addItem(tr("Separate and ordered"), (int)input_sequence::group_type::separated_ordered);

            QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int current) {
               auto* model     = this->_getInputSequenceModel();
               auto  selection = this->_getFirstSeqSelection();
               auto  info_opt  = model->infoFor(selection);
               if (!info_opt.has_value())
                  return;
               info_opt.value().type = (input_sequence::group_type) this->ui.inputSeqGroupType->currentData().toInt();
               model->replaceInfoFor(selection, info_opt.value());
            });
         }
         this->ui.inputSeqButtonMakeRaycastAssoc->setCheckable(true);
         QObject::connect(this->ui.inputSeqButtonRemap, &QPushButton::clicked, this, [this]() {
            auto* model     = this->_getInputSequenceModel();
            auto  selection = this->_getFirstSeqSelection();
            auto  info_opt  = model->infoFor(selection);
            if (!info_opt.has_value())
               return;

            auto& info = info_opt.value();

            auto* modal = new DKWorldinputButtonPickDialog(this);
            modal->setButton(info.button);
            switch (this->device_type) {
               case input_device_type::keyboard_mouse:
                  modal->setGamepadAllowed(false);
                  modal->setMouseAllowed(true);
                  modal->setKeyboardAllowed(true);
                  break;
               case input_device_type::xinput:
                  modal->setGamepadAllowed(true);
                  modal->setMouseAllowed(false);
                  modal->setKeyboardAllowed(false);
                  break;
            }
            modal->setFixedHeight(modal->sizeHint().height()); // shrink height to make up for some controls being hidden
            if (modal->exec() == QDialog::Accepted) {
               info.button = modal->button();
               model->replaceInfoFor(selection, info);
            }
            delete modal;
         });
         QObject::connect(this->ui.inputSeqButtonMakeRaycastAssoc, &QPushButton::clicked, this, [this]() {
            auto* model     = this->_getInputSequenceModel();
            auto  selection = this->_getFirstSeqSelection();
            if (model->isRaycastAssociatedButton(selection)) {
               this->ui.inputSeqButtonMakeRaycastAssoc->setChecked(false == model->setRaycastAssociatedButton(QModelIndex{}));
            } else {
               this->ui.inputSeqButtonMakeRaycastAssoc->setChecked(true  == model->setRaycastAssociatedButton(selection));
            }
         });
         QObject::connect(this->ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            auto* model    = this->_getInputSequenceModel();
            auto  info_opt = model->infoFor(current);

            this->ui.inputSeqEditSelection->setEnabled(info_opt.has_value());
            if (!info_opt.has_value()) {
               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditButton);
               return;
            }
            const auto& info = info_opt.value();
            if (info.type == dovahkit::subsystems::worldinput::input_sequence::group_type::single_control) {
               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditButton);
               this->ui.inputSeqButtonMakeRaycastAssoc->setChecked(model->isRaycastAssociatedButton(current));
            } else {
               const auto blocker_a = QSignalBlocker(this->ui.inputSeqGroupType);

               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditGroup);
               cobb::qt::set_combobox_value(this->ui.inputSeqGroupType, info.type);
            }
         });
         QObject::connect(this->_getInputSequenceModel(), &QAbstractItemModel::modelReset, this, [this]() {
            this->ui.inputSeqEditSelection->setEnabled(false);
         });
      #pragma endregion
   #pragma endregion
   #pragma region Raycast tab
      {
         auto* widget = this->ui.raycastReqTimingType;
         widget->clear();
         widget->addItem(tr("Button pressed"), (int)raycast_requirement::timing_type::on_control_down);
         widget->addItem(tr("Per frame while entering inputs"), (int)raycast_requirement::timing_type::per_frame);
      }
      QObject::connect(this->ui.raycastReqTargetGizmoEnable, &QCheckBox::toggled, this, [this](bool checked) {
         this->ui.raycastReqTargetGizmoAxis->setEnabled(checked);
         this->ui.raycastReqTargetGizmoMode->setEnabled(checked);
      });
      {
         auto* widget = this->ui.raycastReqTargetGizmoMode;
         widget->clear();
         widget->addItem(tr("Translate"), (int)gizmo_mode::translate);
         widget->addItem(tr("Rotate"),    (int)gizmo_mode::rotate);
         widget->addItem(tr("Scale"),     (int)gizmo_mode::scale);
      }
      {
         auto* widget = this->ui.raycastReqTargetGizmoAxis;
         widget->clear();
         widget->addItem(tr("X"), (int)axis3D::x);
         widget->addItem(tr("Y"), (int)axis3D::y);
         widget->addItem(tr("Z"), (int)axis3D::z);
      }
      {
         auto* widget = this->ui.raycastReqSelected;
         widget->clear();
         widget->addItem(tr("Doesn't matter"), (int)optional_yn::unspecified);
         widget->addItem(tr("No"),  (int)optional_yn::no);
         widget->addItem(tr("Yes"), (int)optional_yn::yes);
      }
   #pragma endregion
   #pragma region Tool tab
      {
         auto* widget = this->ui.toolSelector;
         widget->clear();

         const auto& list = worldedit_tool_info::get_all_info();
         for (const auto& item : list) {
            widget->addItem(item.name, (int)item.id);
         }
         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget]() {
            auto id = widget->currentData().toInt();
            this->_setUpToolOptionsUI(id);
         });
      }
   #pragma endregion
}

const DKWorldinputInputSequenceModel* WorldinputBindEditDialog::_getInputSequenceModel() const {
   auto* model = dynamic_cast<const DKWorldinputInputSequenceModel*>(this->ui.treeView->model());
   assert(model);
   return model;
}
QModelIndex WorldinputBindEditDialog::_getFirstSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->currentIndex();
}
const QItemSelection WorldinputBindEditDialog::_getSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->selection();
}

void WorldinputBindEditDialog::initializeFrom(const data_type& node) {
   this->ui.name->setText(node.name);
   
   cobb::qt::set_combobox_value(this->ui.buttonPressType,  node.button_press_type);
   cobb::qt::set_combobox_value(this->ui.rangeControlType, node.input_sequence.range.control);
   cobb::qt::set_combobox_value(this->ui.rangeControlAxis, node.input_sequence.range.axes);

   this->_getInputSequenceModel()->overwriteFromSource(node.input_sequence);
   this->ui.treeView->expandAll();

   {
      auto& src = node.input_sequence.raycast.requirement;

      cobb::qt::set_combobox_value(this->ui.raycastReqTimingType, src.timing);
      this->ui.raycastReqFailOnChange->setChecked(src.fail_if_target_changes);

      const auto blocker_a = QSignalBlocker(this->ui.raycastReqTargetGizmoMode);

      if (src.targets.edit_gizmo_mode != gizmo_mode::none) {
         this->ui.raycastReqTargetGizmoEnable->setChecked(true);
         cobb::qt::set_combobox_value(this->ui.raycastReqTargetGizmoMode, src.targets.edit_gizmo_mode);
         cobb::qt::set_combobox_value(this->ui.raycastReqTargetGizmoAxis, src.targets.edit_gizmo_axis);
      } else {
         this->ui.raycastReqTargetGizmoEnable->setChecked(false);
         cobb::qt::set_combobox_value(this->ui.raycastReqTargetGizmoMode, gizmo_mode::translate);
         cobb::qt::set_combobox_value(this->ui.raycastReqTargetGizmoAxis, src.targets.edit_gizmo_axis);
      }
      this->ui.raycastReqTargetLandscape->setChecked(src.targets.landscapes);
      this->ui.raycastReqTargetNothing->setChecked(src.targets.nothing);
      this->ui.raycastReqTargetRef->setChecked(src.targets.object_references);

      cobb::qt::set_combobox_value(this->ui.raycastReqSelected, src.target_options.selected);
   }

   cobb::qt::set_combobox_value(this->ui.toolSelector, node.tool.id);
   this->_setUpToolOptionsUI(node.tool.id);
   if (this->_tool_options_widget && node.tool.options) {
      const auto& tbl = dovahkit::ui::worldedit::options_widget_dispatch_table;
      for (const auto& entry : tbl) {
         if (entry.id == node.tool.id) {
            (entry.to_ui)(this->_tool_options_widget, *(const dovahkit::subsystems::worldedit::tools::options_union*)node.tool.options);
            break;
         }
      }
   }
}
void WorldinputBindEditDialog::overwrite(data_type& node) const {
   assert(node.name.size() < data_type::max_name_length);
   node.name = this->ui.name->text();

   node.button_press_type = (button_press_type)this->ui.buttonPressType->currentData().toInt();
   node.input_sequence.range.control = (range_input_control) this->ui.rangeControlType->currentData().toInt();
   node.input_sequence.range.axes    = (range_input_axes)    this->ui.rangeControlAxis->currentData().toInt();

   this->_getInputSequenceModel()->overwriteDestination(node.input_sequence);

   {
      auto& dst = node.input_sequence.raycast.requirement;

      dst.timing = (raycast_requirement::timing_type) this->ui.raycastReqTimingType->currentData().toInt();
      dst.fail_if_target_changes = this->ui.raycastReqFailOnChange->isChecked();

      if (this->ui.raycastReqTargetGizmoEnable->isChecked()) {
         dst.targets.edit_gizmo_mode = (gizmo_mode) this->ui.raycastReqTargetGizmoMode->currentData().toInt();
         dst.targets.edit_gizmo_axis = (axis3D)     this->ui.raycastReqTargetGizmoAxis->currentData().toInt();
      } else {
         dst.targets.edit_gizmo_mode = gizmo_mode::none;
         dst.targets.edit_gizmo_axis = axis3D::x;
      }

      dst.targets.landscapes        = this->ui.raycastReqTargetLandscape->isChecked();
      dst.targets.nothing           = this->ui.raycastReqTargetNothing->isChecked();
      dst.targets.object_references = this->ui.raycastReqTargetRef->isChecked();

      dst.target_options.selected = (optional_yn) this->ui.raycastReqSelected->currentData().toInt();
   }

   {
      using options_union = dovahkit::subsystems::worldedit::tools::options_union;

      auto& dst = node.tool;
      dst.id = this->ui.toolSelector->currentData().toInt();
      if (dst.id == dovahkit::subsystems::worldedit::tools::id_of_none) {
         if (dst.options) {
            delete dst.options;
            dst.options = nullptr;
         }
      } else {
         if (this->_tool_options_widget) {
            if (!dst.options) {
               dst.options = new options_union;
            }

            const auto& tbl = dovahkit::ui::worldedit::options_widget_dispatch_table;
            for (const auto& entry : tbl) {
               if (entry.id == dst.id) {
                  (entry.to_data)(this->_tool_options_widget, *(options_union*)dst.options);
                  break;
               }
            }
         } else {
            if (dst.options) {
               delete dst.options;
               dst.options = nullptr;
            }
         }
      }
   }
}

void WorldinputBindEditDialog::_setUpToolOptionsUI(int tool_id) {
   if (auto*& w = this->_tool_options_widget) {
      delete w;
      w = nullptr;
   }

   const auto& tbl = dovahkit::ui::worldedit::options_widget_dispatch_table;
   for (const auto& entry : tbl) {
      if (entry.id == tool_id) {
         this->_tool_options_widget = (entry.make_widget)();
         break;
      }
   }
   if (!this->_tool_options_widget)
      return;

   this->ui.toolOptionsWrapper->layout()->addWidget(this->_tool_options_widget);
}