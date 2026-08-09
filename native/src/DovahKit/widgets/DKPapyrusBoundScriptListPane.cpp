#include "DKPapyrusBoundScriptListPane.h"
#include <QBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#if !defined(QT_PLUGIN)
   #include "dovah/forms/Form.h"
   #include "dovah/forms/ObjectReference.h"
   #include "./widget-dialogs/DKAddPapyrusScriptDialog.h"
   #include "./widget-dialogs/DKBoundScriptDialog.h"
#endif

#if defined(QT_PLUGIN)
#include <QAbstractItemModel>
class DKBoundScriptListModel : public QAbstractItemModel {
   public:
      using QAbstractItemModel::QAbstractItemModel;

      virtual QModelIndex index(int row, int column, const QModelIndex& parent) const override { return {}; }
      virtual QModelIndex parent(const QModelIndex& index) const { return {}; }
      virtual int rowCount(const QModelIndex& parent) const override { return 0; }
      virtual int columnCount(const QModelIndex& item) const override { return 3; }
      virtual Qt::ItemFlags flags(const QModelIndex& index) const override { return {}; }
      virtual QVariant data(const QModelIndex& index, int role) const override { return {}; }
      virtual QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override {
         if (orientation != Qt::Orientation::Horizontal) {
            return {};
         }
         switch (role) {
            case Qt::DisplayRole:
               if (section == 0) {
                  return tr("Script name");
               }
               break;
         }
         return {};
      }
};
#else
namespace vmad {
   using namespace dovah::loaded_forms::components::papyrus;
}
#endif

DKPapyrusBoundScriptListPane::DKPapyrusBoundScriptListPane(QWidget* parent) : QWidget(parent) {
   auto* wrapper_for_all = this->subwidgets.wrapper = new QWidget(this);

   {
      auto* layout = new QVBoxLayout();
      this->setLayout(layout);
      layout->setContentsMargins(0, 0, 0, 0);
      layout->addWidget(wrapper_for_all);
   }

   auto* view = this->subwidgets.view = new QTableView(this);
   auto* wrap = this->subwidgets.buttons.wrapper = new QWidget(this);
   this->subwidgets.buttons.add           = new QPushButton(tr("Add"),        wrap);
   this->subwidgets.buttons.properties    = new QPushButton(tr("Properties"), wrap);
   this->subwidgets.buttons.toggle_delete = new QPushButton(tr("Remove"),     wrap);
   //
   {
      auto* layout = new QGridLayout(wrapper_for_all);
      layout->setContentsMargins(0, 0, 0, 0);
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
   this->model = new DKBoundScriptListModel(this);
   view->setIconSize(QSize(16, 16));
   view->setModel(this->model);
   #if !defined(QT_PLUGIN)
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
   if (auto* header = view->horizontalHeader()) {
      header->setStretchLastSection(true);
   }
   if (auto* vh = view->verticalHeader()) {
      vh->setSectionResizeMode(QHeaderView::ResizeToContents); // needed for sane row sizing
      vh->setVisible(false);
   }
   //
   this->_updateGroupbox();
   //
   #if !defined(QT_PLUGIN)
      QObject::connect(view->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this](const QItemSelection& selected, const QItemSelection& deselected) {
         this->_updateButtons();
      });

      QObject::connect(this->subwidgets.buttons.add, &QPushButton::clicked, this, [this]() {
         QString scriptname;
         {
            auto* dialog = new DKAddPapyrusScriptDialog(this);
            dialog->setAlreadyAttachedScripts(this->model->getAllBoundScriptNames());
            dialog->setTargetType(this->vmad.form->stub.form_type);
            dialog->exec();
            if (dialog->result() == QDialog::Accepted) {
               scriptname = QString::fromStdString(dialog->resultScriptname());
            } else {
               return;
            }
         }

         // It may be tempting to replace the below checks with debug-only assertions. However, if 
         // the custom dialog for choosing a script isn't application-modal, then the user could 
         // modify the base form out from under us (e.g. via the UI or via Lua) and that could 
         // result in them being able to choose a script that is already attached. We, uh, should 
         // not handle that with an assertion failure, lol.
         //
         if (this->model->index(scriptname).isValid()) {
            if (this->vmad.parent)
               QMessageBox::critical(this, "Error", "Script is already attached to this form or to its base form.");
            else
               QMessageBox::critical(this, "Error", "Script is already attached to this form.");
            return;
         }

         auto qmi = this->model->addScript(scriptname);
         assert(qmi.isValid()); // TODO: if the operation above can fail, it should throw a detailed exception and we should catch that here
         this->subwidgets.view->selectionModel()->select(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         {
            auto name = this->model->boundScriptName(qmi);
            if (!name.isEmpty())
               emit this->scriptAdded(name);
         }
      });
      QObject::connect(this->subwidgets.buttons.properties,    &QPushButton::clicked, this, [this]() { this->_editSelected(); });
      QObject::connect(this->subwidgets.buttons.toggle_delete, &QPushButton::clicked, this, [this]() {
         QModelIndex script_qmi;
         {
            auto* sm = this->subwidgets.view->selectionModel();
            if (!sm)
               return;
            auto rows = sm->selectedRows();
            if (rows.isEmpty())
               return;
            script_qmi = rows[0];
         }

         auto opt = this->model->status(script_qmi);
         if (!opt.has_value())
            return;
         auto name = this->model->boundScriptName(script_qmi);
         if (opt.value() != vmad::script_status::removed) {
            size_t size_prior = this->model->rowCount({});
            this->model->removeScript(script_qmi);
            size_t size_after = this->model->rowCount({});
            emit this->scriptRemoved(name, size_prior == size_after);
         } else {
            this->model->undeleteScript(script_qmi);
            emit this->scriptAdded(name);
         }
         this->_updateButtons();
      });
   #endif
}

void DKPapyrusBoundScriptListPane::_updateGroupbox() {
   this->setUpdatesEnabled(false);

   auto* groupbox = this->subwidgets.groupbox;
   auto* wrapper  = this->subwidgets.wrapper;
   if (this->state.use_groupbox) {
      if (!groupbox) {
         groupbox = this->subwidgets.groupbox = new QGroupBox(this);
         groupbox->setTitle(tr("Scripts"));
         new QGridLayout(groupbox);
      }
      groupbox->layout()->addWidget(wrapper);
      this->layout()->addWidget(groupbox);
   } else {
      this->layout()->addWidget(wrapper);
      this->subwidgets.groupbox = nullptr;
      delete groupbox;
   }

   this->setUpdatesEnabled(true);
}

#if !defined(QT_PLUGIN)
   void DKPapyrusBoundScriptListPane::_editSelected() {
      QModelIndex script_qmi;
      {
         auto* sm = this->subwidgets.view->selectionModel();
         if (sm) {
            auto rows = sm->selectedRows();
            if (!rows.isEmpty())
               script_qmi = rows[0];
         }
      }
      if (!script_qmi.isValid())
         return;

      auto* dialog = new DKBoundScriptDialog(*this, script_qmi);
      if (dialog->loadFailed()) {
         QMessageBox::critical(
            this,
            tr("Error"),
            tr("Failed to load the script. One of the needed PEX files (i.e. its own, or the files for its ancestor classes) is missing or could not be parsed properly.")
         );
         dialog->deleteLater();
         return;
      }
      auto  result = dialog->exec();
      if (result == QDialog::Accepted) {
         //
         // NOTE: The dialog commits its contents itself, for now.
         //
      }
      dialog->deleteLater();
   }
   void DKPapyrusBoundScriptListPane::_updateButtons() {
      QModelIndex script_qmi;
      {
         auto* sm = this->subwidgets.view->selectionModel();
         if (sm) {
            auto rows = sm->selectedRows();
            if (!rows.isEmpty())
               script_qmi = rows[0];
         }
      }
      if (!script_qmi.isValid()) {
         this->subwidgets.buttons.toggle_delete->setEnabled(false);
         this->subwidgets.buttons.properties->setEnabled(false);
         this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
         return;
      }

      auto opt = this->model->status(script_qmi);
      if (!opt.has_value()) {
         this->subwidgets.buttons.toggle_delete->setEnabled(false);
         this->subwidgets.buttons.properties->setEnabled(false);
         this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
         return;
      }
      this->subwidgets.buttons.toggle_delete->setEnabled(true);
      if (opt.value() != vmad::script_status::removed) {
         this->subwidgets.buttons.properties->setEnabled(true);
         this->subwidgets.buttons.toggle_delete->setText(tr("Delete", "button label to delete attached script"));
      } else {
         this->subwidgets.buttons.properties->setEnabled(false);
         this->subwidgets.buttons.toggle_delete->setText(tr("Undelete", "button label to restore attached script inherited and deleted on REFR"));
      }
   }
#endif
#if !defined(QT_PLUGIN)
   void DKPapyrusBoundScriptListPane::setFormWorkingCopy(working_copy_type* target_form) {
      if (!target_form) {
         this->vmad = {};

         this->subwidgets.buttons.wrapper->setEnabled(false);
         this->model->clear();
         emit this->scriptListReset();
         return;
      }

      this->vmad = {};
      this->vmad.form = target_form;

      bool loaded_parent = false;
      //
      if (dovah::form_type_is_reference(target_form->stub.form_type)) {
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
         this->model->initializeFrom(*target_vmad, *parent_vmad);
      } else {
         this->model->initializeFrom(*target_vmad);
      }

      this->subwidgets.buttons.wrapper->setEnabled(true);
      emit this->scriptListReset();
   }
   void DKPapyrusBoundScriptListPane::setQuestWorkingCopyAndAliasVMAD(working_copy_type* quest_working_copy, vmad_type& target) {
      if (!quest_working_copy) {
         this->vmad = {};

         this->subwidgets.buttons.wrapper->setEnabled(false);
         this->model->clear();
         emit this->scriptListReset();
         return;
      }

      this->vmad = {};
      this->vmad.form   = quest_working_copy;
      this->vmad.target = &target;

      this->model->initializeFrom(target);
      this->subwidgets.buttons.wrapper->setEnabled(true);
      emit this->scriptListReset();
   }

   void DKPapyrusBoundScriptListPane::commit() {
      if (!this->vmad.target)
         return;
      if (!this->vmad.form)
         return;
      this->model->commitTo(*this->vmad.target, *this->vmad.form);
   }
#endif


void DKPapyrusBoundScriptListPane::setUsesGroupbox(bool v) {
   if (v == this->state.use_groupbox)
      return;
   this->state.use_groupbox = v;
   this->_updateGroupbox();
}

#if !defined(QT_PLUGIN)
   std::vector<QString> DKPapyrusBoundScriptListPane::allNonDeletedScriptnames() const {
      auto list = this->model->getAllBoundScriptNames(false);

      std::vector<QString> str;
      for (auto& item : list)
         str.push_back(QString::fromUtf8(item.data(), item.size()));
      return str;
   }
   bool DKPapyrusBoundScriptListPane::hasScript(QString name, bool allow_deleted) const {
      auto qmi = this->model->index(name);
      if (!qmi.isValid())
         return false;
      if (!allow_deleted) {
         auto status = this->model->status(qmi);
         if (status == ui::bound_script_models::vmad::script_status::removed) {
            return false;
         }
      }
      return true;
   }
#endif