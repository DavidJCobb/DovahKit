#include "./worldinput_modifier_editor.h"
#include <QAction>
#include <QMenu>
#include "helpers/qt/combobox.h"
#include "widgets/DKHeaderView.h"

#include "editor/subsystems/worldinput2/control_scheme/modifier.h"
#include "editor/subsystems/worldinput2/control_scheme.h"

#include "ui/models/worldinput/DKWorldinputInputSequenceModel.h"
#include "widgets/widget-dialogs/DKWorldinputButtonPickDialog.h"

WorldinputModifierEditDialog::WorldinputModifierEditDialog(input_device_type device_type, QWidget* parent) : QDialog(parent), device_type(device_type) {
   this->ui.setupUi(this);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   this->ui.name->setMaxLength(data_type::max_name_length);

   #pragma region Inputs tab
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
}

const DKWorldinputInputSequenceModel* WorldinputModifierEditDialog::_getInputSequenceModel() const {
   auto* model = dynamic_cast<const DKWorldinputInputSequenceModel*>(this->ui.treeView->model());
   assert(model);
   return model;
}
QModelIndex WorldinputModifierEditDialog::_getFirstSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->currentIndex();
}
const QItemSelection WorldinputModifierEditDialog::_getSeqSelection() {
   auto* sm = this->ui.treeView->selectionModel();
   if (!sm)
      return {};
   return sm->selection();
}

void WorldinputModifierEditDialog::initializeFrom(const data_type& node) {
   this->ui.name->setText(node.name);

   this->_getInputSequenceModel()->overwriteFromSource(node.input_sequence);
   this->ui.treeView->expandAll();
}
void WorldinputModifierEditDialog::overwrite(data_type& node) const {
   assert(node.name.size() < data_type::max_name_length);
   node.name = this->ui.name->text();

   this->_getInputSequenceModel()->overwriteDestination(node.input_sequence);
}