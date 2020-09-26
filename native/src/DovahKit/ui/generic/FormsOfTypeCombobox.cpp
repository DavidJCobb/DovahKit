#include "FormsOfTypeCombobox.h"
#include <QStandardItemModel>
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include "impl/_FormsOfTypeComboboxProxy.h"

//
// These can incur a performance hit in Debug, but they're mostly fine in Release.
//

FormsOfTypeCombobox::FormsOfTypeCombobox(QWidget* parent) : QComboBox(parent) {
   this->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); // needed for performance with large data sets
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      const auto blocker = QSignalBlocker(this);
      this->setDisabled(true);
      this->clear();
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      const auto blocker = QSignalBlocker(this);
      int index = this->findData(stub->formID);
      if (index < 0)
         return;
      this->setItemText(index, stub->get_editor_id());
   });
};

void FormsOfTypeCombobox::addFormType(dovah::form_type_t ft) {
   this->_formTypes.push_back(ft);
}
bool FormsOfTypeCombobox::allowsFormType(dovah::form_type_t ft) const noexcept {
   return this->_formTypes.indexOf(ft) >= 0;
}
dovah::bare_form_id_t FormsOfTypeCombobox::formID() const noexcept {
   int i = this->currentIndex();
   if (i < 0)
      return 0;
   return this->currentData().value<quint32>();
}
void FormsOfTypeCombobox::populate() {
   const auto blocker = QSignalBlocker(this);
   auto previous_selection = this->currentData().toInt();
   this->clear();
   //
   auto& editor = DovahKitCore::get();
   if (!editor.has_data())
      return;
   auto* model = new QStandardItemModel(this); // we need to do this indirectly instead of using QComboBox::addItem in order to get case-insensitive sorting
   for (auto ft : this->_formTypes) {
      editor.for_each_form_of_type(ft, [model](dovah::form_stub* stub) {
         auto* item = new QStandardItem(QString::fromStdString(stub->get_editor_id()));
         item->setData(stub->formID, Qt::UserRole);
         model->appendRow(item);
         return false;
      });
   }
   if (this->_allowUndefined) {
      QString text = this->_undefinedLabel;
      if (text.isEmpty())
         text = tr("UNDEFINED");
      auto* item = new QStandardItem(text);
      item->setData(0, Qt::UserRole);
      item->setData(1, undefined_role);
      model->appendRow(item);
   }
   if (this->_allowNone) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      auto* item = new QStandardItem(text);
      item->setData(0, Qt::UserRole);
      model->appendRow(item);
   }
   //
   auto* proxy = new _FormsOfTypeComboboxProxy;
   proxy->setSourceModel(model);
   proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setModel(proxy);
   proxy->sort(0);
   //
   int index = this->findData(previous_selection, Qt::UserRole);
   if (index >= 0)
      this->setCurrentIndex(index);
   else {
      index = -1;
      if (this->_allowNone)
         index = this->findData(0, Qt::UserRole);
      this->setCurrentIndex(index);
   }
   //
   emit populated();
}

void FormsOfTypeCombobox::setAllowNone(bool s) noexcept {
   if (this->_allowNone == s)
      return;
   this->_allowNone = s;
   if (s) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      this->addItem(text, 0);
   } else {
      int i = this->findData(0);
      if (i >= 0)
         this->removeItem(i);
   }
}
void FormsOfTypeCombobox::setNoneLabel(const QString& v) noexcept {
   this->_noneLabel = v;
   if (this->_allowNone) {
      int index = this->findData(0, Qt::UserRole);
      if (index < 0)
         return;
      this->setItemText(index, this->_noneLabel);
   }
}

void FormsOfTypeCombobox::setAllowUndefined(bool s) noexcept {
   if (this->_allowUndefined == s)
      return;
   this->_allowUndefined = s;
   if (s) {
      QString text = this->_undefinedLabel;
      if (text.isEmpty())
         text = tr("UNDEFINED");
      this->addItem(text, 0);
      this->setItemData(this->count() - 1, 1, undefined_role); // using the model here feels safer, but Qt crashes when we try
   } else {
      int i = this->findData(1, undefined_role);
      if (i >= 0)
         this->removeItem(i);
   }
}
void FormsOfTypeCombobox::setUndefinedLabel(const QString& v) noexcept {
   this->_undefinedLabel = v;
   if (this->_allowUndefined) {
      int index = this->findData(1, undefined_role);
      if (index < 0)
         return;
      this->setItemText(index, this->_undefinedLabel);
   }
}
bool FormsOfTypeCombobox::isUndefined() const noexcept {
   if (!this->_allowUndefined)
      return false;
   return this->currentData(undefined_role).toInt() != 0;
}
void FormsOfTypeCombobox::setToUndefined() noexcept {
   if (!this->_allowUndefined)
      return;
   int index = this->findData(1, undefined_role);
   if (index < 0)
      return;
   this->setCurrentIndex(index);
}

void FormsOfTypeCombobox::setFormByID(dovah::bare_form_id_t formID) noexcept {
   if (!formID && !this->_allowNone)
      formID = this->_defaultFormID;
   int i = this->findData(formID, Qt::UserRole);
   if (i >= 0)
      this->setCurrentIndex(i);
}
void FormsOfTypeCombobox::setDefaultFormID(dovah::bare_form_id_t formID) noexcept {
   this->_defaultFormID = formID;
}