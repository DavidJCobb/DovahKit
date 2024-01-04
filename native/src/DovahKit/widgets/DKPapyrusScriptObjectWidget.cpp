#include "DKPapyrusScriptObjectWidget.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include "dovah/forms/Form.h"
#include "dovah/forms/ObjectReference.h"

#include <QInputDialog> // placeholder
#include <QMessageBox>
namespace {
   constexpr const bool require_complete_implementation = false;
}

#if defined(QT_DESIGNER_LIB)
#include <QAbstractItemModel>
class DKPapyrusScriptObjectListModel : public QAbstractItemModel {
   public:
      using QAbstractItemModel::QAbstractItemModel;

      virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override { return QModelIndex(); }
      virtual QModelIndex parent(const QModelIndex& index) const { return QModelIndex(); }
      virtual int rowCount(const QModelIndex& parent) const override { return 0; }
      virtual int columnCount(const QModelIndex& item) const override { return 3; }
      virtual Qt::ItemFlags flags(const QModelIndex& index) const override { return 0; }
      virtual QVariant data(const QModelIndex& index, int role) const override { return QVariant(); }
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
         if (orientation != Qt::Orientation::Horizontal) {
            return QVariant();
         }
         switch (role) {
            case Qt::DisplayRole:
               if (section == 0) {
                  return tr("Script name");
               }
               break;
         }
         return QVariant();
      }
};
#endif

DKPapyrusScriptObjectWidget::DKPapyrusScriptObjectWidget(QWidget* parent) : QWidget(parent) {
   auto* groupbox = new QGroupBox(this);
   groupbox->setTitle("Scripts");

   {
      auto* layout = new QVBoxLayout();
      this->setLayout(layout);
      layout->addWidget(groupbox);
   }

   auto* view = this->subwidgets.view = new QTableView(this);
   auto* wrap = this->subwidgets.buttons.wrapper = new QWidget(this);
   this->subwidgets.buttons.add           = new QPushButton(wrap);
   this->subwidgets.buttons.properties    = new QPushButton(wrap);
   this->subwidgets.buttons.toggle_delete = new QPushButton(wrap);
   this->subwidgets.buttons.toggle_delete->setText(tr("Remove"));
   //
   {
      auto* layout = new QGridLayout(groupbox);
      layout->addWidget(new QLabel(tr("Papyrus scripts")), 0, 0, 1, 2);
      layout->addWidget(view, 1, 0);
      layout->addWidget(wrap, 1, 1);
      layout->setRowStretch(0, 0);
      layout->setRowStretch(1, 1);

      auto* nested = new QVBoxLayout(wrap);
      nested->addWidget(this->subwidgets.buttons.add);
      nested->addWidget(this->subwidgets.buttons.toggle_delete);
      nested->addWidget(this->subwidgets.buttons.properties);
      nested->addStretch(1);
      
      layout->setContentsMargins({ 0, 0, 0, 0 });
      nested->setContentsMargins({ 0, 0, 0, 0 });
      layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
      nested->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
   }
   #pragma region Tab order
      this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->setFocusProxy(this->subwidgets.view);
      this->setTabOrder(this->subwidgets.view, this->subwidgets.buttons.wrapper);
      //
      this->subwidgets.buttons.wrapper->setFocusPolicy(Qt::FocusPolicy::TabFocus);
      this->subwidgets.buttons.wrapper->setFocusProxy(this->subwidgets.buttons.add);
      this->setTabOrder(this->subwidgets.buttons.add,           this->subwidgets.buttons.toggle_delete);
      this->setTabOrder(this->subwidgets.buttons.toggle_delete, this->subwidgets.buttons.properties);
   #pragma endregion
   //
   this->model = new DKPapyrusScriptObjectListModel(this);
   view->setIconSize(QSize(16, 16));
   view->setModel(this->model);
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(view, &QTableView::doubleClicked, [this, view](const QModelIndex& index) {
         if (!index.isValid())
            return;
         this->subwidgets.view->selectionModel()->select(index, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         if (this->subwidgets.buttons.properties->isEnabled())
            this->_editSelected();
      });
   #endif
   view->setSelectionBehavior(QAbstractItemView::SelectRows);
   view->setSelectionMode(QAbstractItemView::SingleSelection);
   view->setCornerButtonEnabled(false);
   view->setAcceptDrops(true);
   view->setDragDropOverwriteMode(false);
   //
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
         this->_updateButtons();
      });

      QObject::connect(this->subwidgets.buttons.add, &QPushButton::clicked, this, [this]() {
         static_assert(!require_complete_implementation, "Pop a dialog listing all scripts.");

         //
         // TODO: Replace this use of QInputDialog with a custom dialog that lists all Papyrus scripts 
         //       known to DovahKit, with us passing a list of already-attached scripts on this form 
         //       (and its base form, where relevant). Those scripts should be greyed out in the dialog 
         //       and listed as already-attached, and the user should not be allowed to select any of 
         //       them.
         //
         bool ok;
         auto scriptname = QInputDialog::getText(
            this,
            "Add script",
            "Scriptname: ",
            QLineEdit::EchoMode::Normal,
            "",
            &ok
         );
         if (!ok)
            return;

         // TODO: Once we've replaced QInputDialog as described above, it will be tempting to replace 
         //       the below checks with debug-only assertions. However, if the custom dialog for 
         //       choosing a script isn't application-modal, then the user could modify the base form 
         //       out from under us (e.g. via the UI or via Lua) and that could result in them being 
         //       able to choose a script that is already attached. We, uh, should not handle that 
         //       with an assertion failure, lol.
         //
         if (this->model->index(scriptname).isValid()) {
            if (this->vmad.parent)
               QMessageBox::critical(this, "Error", "Script is already attached to this form or to its base form.");
            else
               QMessageBox::critical(this, "Error", "Script is already attached to this form.");
            return;
         }

         auto qmi = this->model->addScript(scriptname);
         assert(qmi.isValid()); // if the operation above can fail, it should throw a detailed exception and we should catch that here
         this->subwidgets.view->selectionModel()->select(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      });
      QObject::connect(this->subwidgets.buttons.properties,    &QPushButton::clicked, this, [this]() { this->_editSelected(); });
      QObject::connect(this->subwidgets.buttons.toggle_delete, &QPushButton::clicked, this, [this]() {
         auto* sm = this->subwidgets.view->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         if (rows.isEmpty())
            return;
         int row = rows[0].row();

         auto raw_info = this->model->row(row);
         if (!raw_info.attached_to)
            return;
         if (!raw_info.target_script)
            return;
         
         if (!raw_info.is_inherited_and_removed()) {
            this->model->removeScript(row);
         } else {
            this->model->undeleteInheritedScript(row);
         }
      });
   #endif
}

#if !defined(QT_DESIGNER_LIB)
   void DKPapyrusScriptObjectWidget::_editSelected() {
      int row;
      {
         auto* sm = this->subwidgets.view->selectionModel();
         if (!sm)
            return;
         auto rows = sm->selectedRows();
         if (rows.isEmpty())
            return;
         row = rows[0].row();
      }

      static_assert(!require_complete_implementation, "open properties dialog for selected scriptobject");
      static_assert(!require_complete_implementation, "after dialog, call `this->model->refreshScript(scriptname)` in case any properties were edited");
   }
   void DKPapyrusScriptObjectWidget::_updateButtons() {
      int row;
      {
         auto* sm = this->subwidgets.view->selectionModel();
         if (!sm) {
            this->subwidgets.buttons.wrapper->setEnabled(false);
            this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
            return;
         }
         auto rows = sm->selectedRows();
         if (rows.isEmpty()) {
            this->subwidgets.buttons.wrapper->setEnabled(false);
            this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
            return;
         }
         row = rows[0].row();
      }

      auto raw_info = this->model->row(row);
      if (!raw_info.attached_to) {
         this->subwidgets.buttons.wrapper->setEnabled(false);
         this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
         return;
      }
      if (!raw_info.is_inherited_and_removed()) {
         this->subwidgets.buttons.properties->setEnabled(true);
         this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
      } else {
         this->subwidgets.buttons.properties->setEnabled(false);
         this->subwidgets.buttons.toggle_delete->setText(tr("Undelete", "button label to restore attached script inherited and deleted on REFR"));
      }
   }
#endif
#if !defined(QT_DESIGNER_LIB)
   void DKPapyrusScriptObjectWidget::setFormWorkingCopy(working_copy_type* target_form) {
      if (!target_form) {
         this->vmad = {};

         this->subwidgets.buttons.wrapper->setEnabled(false);
         this->model->clearWorkingVMAD();
         return;
      }

      this->vmad = {};
      this->vmad.form = target_form;

      bool loaded_parent = false;
      //
      if (dovah::form_type_info::form_type_is_reference(target_form->stub.formType)) {
         dovah::form_stub* base = ((dovah::loaded_forms::ObjectReference*)target_form)->base_form.get_form_stub();
         if (base) {
            this->vmad.base_form = base->load();
            if (this->vmad.base_form) {
               loaded_parent = true;
            }
         }
      }
      //
      auto* target_vmad = target_form->get_raw_papyrus_data();
      assert(target_vmad != nullptr);
      this->vmad.target = target_vmad;
      if (loaded_parent) {
         auto* parent_vmad = this->vmad.base_form->get_raw_papyrus_data();
         this->vmad.parent = parent_vmad;
         this->model->setWorkingVMAD(*target_form, *target_vmad, *parent_vmad);
      } else {
         this->model->setWorkingVMAD(*target_form, *target_vmad);
      }

      this->subwidgets.buttons.wrapper->setEnabled(true);
   }
   void DKPapyrusScriptObjectWidget::setQuestWorkingCopyAndAliasVMAD(working_copy_type* quest_working_copy, vmad_type& target) {
      if (!quest_working_copy) {
         this->vmad = {};

         this->subwidgets.buttons.wrapper->setEnabled(false);
         this->model->clearWorkingVMAD();
         return;
      }

      this->vmad = {};
      this->vmad.form = quest_working_copy;

      auto* target_vmad = quest_working_copy->get_raw_papyrus_data();
      assert(target_vmad != nullptr);
      this->model->setWorkingVMAD(*quest_working_copy, target);
      this->subwidgets.buttons.wrapper->setEnabled(true);
   }
#endif