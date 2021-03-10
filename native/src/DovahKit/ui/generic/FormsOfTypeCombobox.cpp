#include "FormsOfTypeCombobox.h"
#include <QStandardItemModel>
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include "impl/_FormsOfTypeComboboxProxy.h"

//
// These can incur a performance hit in Debug, but they're mostly fine in Release.
//

//
// The "undefined" option allows you to have a different "no form" state from "none," 
// but is not currently used. It arose from a misunderstanding on my part regarding 
// cell music types: the Creation Kit gives you a "DEFAULT" option and a "NONE" 
// option. What I didn't realize is that "NONE" is actually a music type. Bethesda 
// literally created a music type form and named it "NONE." See, I had thought that 
// selecting "DEFAULT" would not encode any music type and selecting "NONE" would 
// encode form ID 0 as the music type... but of course, given the Rule of One, both 
// of those would have the same effect anyway.
//

namespace {
   bool _should_exclude_form(const dovah::form_stub* stub) {
      if (stub->formType == dovah::form_type::cell)
         return stub->is_exterior_cell();
      return false;
   }
}

FormsOfTypeCombobox::FormsOfTypeCombobox(QWidget* parent) : QComboBox(parent) {
   this->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon); // needed for performance with large data sets
   //
   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      const auto blocker = QSignalBlocker(this);
      this->setDisabled(true);
      this->clear();
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      int index = this->findData(stub->formID);
      if (index < 0)
         return;
      int si = this->currentIndex();
      this->removeItem(index);
      if (index == si && this->_allowNone)
         this->setCurrentIndex(0);
   });
   QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
      const auto blocker = QSignalBlocker(this);
      if (!this->count()) // only auto-update for new forms after we've been populated at our owner's discretion
         return;
      if (!this->allowsFormType(stub->formType))
         return;
      if (_should_exclude_form(stub))
         return;
      auto* proxy = (_FormsOfTypeComboboxProxy*)this->model();
      if (!proxy)
         return;
      auto* model = (QStandardItemModel*)proxy->sourceModel();
      if (!model)
         return;
      int index = this->findData(stub->formID);
      if (index >= 0)
         return;
      auto* item = new QStandardItem(QString::fromStdString(stub->get_editor_id()));
      item->setData(stub->formID, FormIDRole);
      item->setData(QVariant::fromValue((void*)stub), FormStubRole);
      model->appendRow(item);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      const auto blocker = QSignalBlocker(this);
      int index = this->findData(stub->formID);
      if (index < 0)
         return;
      this->setItemText(index, stub->get_editor_id());
   });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
      const auto blocker = QSignalBlocker(this);
      int index = this->findData(QVariant::fromValue((void*)stub), FormStubRole);
      if (index < 0)
         return;
      this->setItemData(index, newID, FormIDRole);
   });
};

void FormsOfTypeCombobox::addFormType(dovah::form_type_t ft) {
   this->_formTypes.push_back(ft);
}
void FormsOfTypeCombobox::allowAllFormTypes() {
   this->_formTypes.clear();
}
bool FormsOfTypeCombobox::allowsFormType(dovah::form_type_t ft) const noexcept {
   if (this->_formTypes.isEmpty())
      return true;
   return this->_formTypes.indexOf(ft) >= 0;
}
dovah::bare_form_id_t FormsOfTypeCombobox::formID() const noexcept {
   int i = this->currentIndex();
   if (i < 0)
      return 0;
   return this->currentData(FormIDRole).value<quint32>();
}
dovah::form_stub* FormsOfTypeCombobox::formStub() const noexcept {
   int i = this->currentIndex();
   if (i < 0)
      return nullptr;
   return (dovah::form_stub*)this->currentData(FormStubRole).value<void*>();
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
   {
      auto insert_lambda = [model](dovah::form_stub* stub) {
         if (_should_exclude_form(stub))
            return false;
         auto* item = new QStandardItem(QString::fromStdString(stub->get_editor_id()));
         item->setData(stub->formID, FormIDRole);
         item->setData(QVariant::fromValue((void*)stub), FormStubRole);
         model->appendRow(item);
         return false;
      };
      //
      for (auto ft : this->_formTypes)
         editor.for_each_form_of_type(ft, insert_lambda);
      if (this->_formTypes.isEmpty()) // allow all forms
         editor.for_each_form(insert_lambda);
   }
   if (this->_allowUndefined) {
      QString text = this->_undefinedLabel;
      if (text.isEmpty())
         text = tr("UNDEFINED");
      auto* item = new QStandardItem(text);
      item->setData(0, FormIDRole);
      item->setData(1, UndefinedRole);
      item->setData(QVariant::fromValue(nullptr), FormStubRole);
      model->appendRow(item);
   }
   if (this->_allowNone) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      auto* item = new QStandardItem(text);
      item->setData(0, FormIDRole);
      item->setData(QVariant::fromValue(nullptr), FormStubRole);
      model->appendRow(item);
   }
   //
   auto* proxy = new _FormsOfTypeComboboxProxy;
   proxy->setSourceModel(model);
   proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
   this->setModel(proxy);
   proxy->sort(0);
   //
   int index = this->findData(previous_selection, FormIDRole);
   if (index >= 0)
      this->setCurrentIndex(index);
   else {
      index = -1;
      if (this->_allowNone)
         index = this->findData(0, FormIDRole);
      this->setCurrentIndex(index);
   }
   //
   emit populated();
}
void FormsOfTypeCombobox::setAllowedFormType(dovah::form_type_t ft) {
   this->_formTypes.clear();
   this->addFormType(ft);
}
void FormsOfTypeCombobox::setAllowedFormTypes(QVector<dovah::form_type_t> ft) {
   this->_formTypes = ft;
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
      this->setItemData(this->count() - 1, 1, UndefinedRole); // using the model here feels safer, but Qt crashes when we try
   } else {
      int i = this->findData(1, UndefinedRole);
      if (i >= 0)
         this->removeItem(i);
   }
}
void FormsOfTypeCombobox::setUndefinedLabel(const QString& v) noexcept {
   this->_undefinedLabel = v;
   if (this->_allowUndefined) {
      int index = this->findData(1, UndefinedRole);
      if (index < 0)
         return;
      this->setItemText(index, this->_undefinedLabel);
   }
}
bool FormsOfTypeCombobox::isUndefined() const noexcept {
   if (!this->_allowUndefined)
      return false;
   return this->currentData(UndefinedRole).toInt() != 0;
}
void FormsOfTypeCombobox::setToUndefined() noexcept {
   if (!this->_allowUndefined)
      return;
   int index = this->findData(1, UndefinedRole);
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
void FormsOfTypeCombobox::setFormStub(dovah::form_stub* stub) noexcept {
   dovah::bare_form_id_t id = 0;
   if (stub)
      id = stub->formID;
   this->setFormByID(id);
}
void FormsOfTypeCombobox::setDefaultFormID(dovah::bare_form_id_t formID) noexcept {
   this->_defaultFormID = formID;
}