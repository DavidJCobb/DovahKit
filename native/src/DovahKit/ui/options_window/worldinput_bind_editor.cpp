#include "./worldinput_bind_editor.h"
#include "helpers/qt/combobox.h"

#include "editor/subsystems/worldinput2/bind_tree/nodes/bound_tool.h"
#include "editor/subsystems/worldinput2/bind_tree/nodes/modifier.h"

#include "ui/models/worldinput/DKWorldinputInputSequenceModel.h"
#include "widgets/widget-dialogs/DKWorldinputButtonPickDialog.h"

WorldinputBindEditDialog::WorldinputBindEditDialog(input_device_type device_type, QWidget* parent) : QDialog(parent), device_type(device_type) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   this->ui.name->setMaxLength(dovahkit::subsystems::worldinput2::binds::nodes::abstract_input_node::max_name_length);

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
            if (dovahkit::subsystems::worldinput2::range_input_control_has_multiple_axes(ctrl)) {
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
         treeview->expandAll();
      }
      #pragma region Treeview hierarchy edit buttons
         QObject::connect(this->ui.inputSeqAddButton, &QPushButton::clicked, this, [this]() {
            auto created = this->_getModel()->addButtonTo(this->_getFirstSeqSelection());
            if (created.has_value()) {
               auto* sm = this->ui.treeView->selectionModel();
               if (sm)
                  sm->select(created.value(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.inputSeqAddGroup, &QPushButton::clicked, this, [this]() {
            auto created = this->_getModel()->addGroupTo(this->_getFirstSeqSelection());
            if (created.has_value()) {
               auto* sm = this->ui.treeView->selectionModel();
               if (sm)
                  sm->select(created.value(), QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.inputSeqMoveUp, &QPushButton::clicked, this, [this]() {
            this->_getModel()->moveItems(this->_getSeqSelection(), -1);
         });
         QObject::connect(this->ui.inputSeqMoveDown, &QPushButton::clicked, this, [this]() {
            this->_getModel()->moveItems(this->_getSeqSelection(), 1);
         });
         QObject::connect(this->ui.inputSeqDelete, &QPushButton::clicked, this, [this]() {
            this->_getModel()->deleteItems(this->_getSeqSelection());
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
               auto* model     = this->_getModel();
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
            auto* model     = this->_getModel();
            auto  selection = this->_getFirstSeqSelection();
            auto  info_opt  = model->infoFor(selection);
            if (!info_opt.has_value())
               return;

            auto& info = info_opt.value();

            auto* modal = new DKWorldinputButtonPickDialog(this);
            modal->setButton(DKWorldinputButtonPickDialog::Button{
               .key     = info.button.vk,
               .mouse   = info.button.mouse,
               .gamepad = info.button.xinput,
            });
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
               auto b = modal->button();
               info.button = {
                  .mouse  = b.mouse,
                  .vk     = b.key.vk,
                  .xinput = b.gamepad,
               };
               model->replaceInfoFor(selection, info);
            }
            delete modal;
         });
         QObject::connect(this->ui.inputSeqButtonMakeRaycastAssoc, &QPushButton::clicked, this, [this]() {
            auto* model     = this->_getModel();
            auto  selection = this->_getFirstSeqSelection();
            this->ui.inputSeqButtonMakeRaycastAssoc->setChecked(model->setRaycastAssociatedButton(selection));
         });
         QObject::connect(this->ui.treeView->selectionModel(), &QItemSelectionModel::currentChanged, this, [this](const QModelIndex& current, const QModelIndex& previous) {
            auto* model    = this->_getModel();
            auto  info_opt = model->infoFor(current);

            this->ui.inputSeqEditSelection->setEnabled(info_opt.has_value());
            if (!info_opt.has_value()) {
               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditButton);
               return;
            }
            const auto& info = info_opt.value();
            if (info.type == dovahkit::subsystems::worldinput2::input_sequence::group_type::single_control) {
               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditButton);
               this->ui.inputSeqButtonMakeRaycastAssoc->setChecked(model->raycastAssociatedButton() == current);
            } else {
               const auto blocker_a = QSignalBlocker(this->ui.inputSeqGroupType);

               this->ui.inputSeqEditSelection->setCurrentWidget(this->ui.inputSeqEditGroup);
               cobb::qt::set_combobox_value(this->ui.inputSeqGroupType, info.type);
            }
         });
         QObject::connect(this->_getModel(), &QAbstractItemModel::modelReset, this, [this]() {
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
      // TODO: Tool and options
   #pragma endregion
}

DKWorldinputInputSequenceModel* WorldinputBindEditDialog::_getModel() {
   auto* model = dynamic_cast<DKWorldinputInputSequenceModel*>(this->ui.treeView->model());
   assert(model);
   return model;
}
QModelIndex WorldinputBindEditDialog::_getFirstSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->currentIndex();
}
QModelIndexList WorldinputBindEditDialog::_getSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->selectedRows();
}

void WorldinputBindEditDialog::initializeFrom(const dovahkit::subsystems::worldinput2::binds::nodes::bound_tool& node) {
   this->ui.name->setText(node.name);
   
   cobb::qt::set_combobox_value(this->ui.buttonPressType,  node.button_press_type);
   cobb::qt::set_combobox_value(this->ui.rangeControlType, node.input_sequence.range.control);
   cobb::qt::set_combobox_value(this->ui.rangeControlAxis, node.input_sequence.range.axes);

   this->_getModel()->overwriteFromSource(node.input_sequence);
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

   // TODO: Tool and options
}
void WorldinputBindEditDialog::overwrite(dovahkit::subsystems::worldinput2::binds::nodes::bound_tool& node) const {
   assert(node.name.size() < std::decay_t<decltype(node)>::max_name_length);
   node.name = this->ui.name->text();

   node.button_press_type = (button_press_type)this->ui.buttonPressType->currentData().toInt();
   node.input_sequence.range.control = (range_input_control) this->ui.rangeControlType->currentData().toInt();
   node.input_sequence.range.axes    = (range_input_axes)    this->ui.rangeControlAxis->currentData().toInt();

   // TODO: Treeview model

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

   // TODO: Tool and options
}