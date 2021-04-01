#include "FormPicker.h"
#include "impl/_FormsOfTypeComboboxProxy.h"
#include <QBoxLayout>
#include <QEvent>
#include <QStandardItemModel>
#include "../../dovah/form_stub.h"
#include "../../editor/core.h"
#include "../../editor/form_stub_meta_type.h"
#include "../../helpers/qt/strings.h"

namespace {
   static constexpr Qt::ItemDataRole FormIDRole    = (Qt::ItemDataRole)Qt::UserRole;
   static constexpr Qt::ItemDataRole FormStubRole  = (Qt::ItemDataRole)(Qt::UserRole + 1);

   bool _should_exclude_form(const dovah::form_stub* stub) {
      if (stub->formType == dovah::form_type::cell)
         return stub->is_exterior_cell();
      return false;
   }
   QStandardItem* _make_form_item(dovah::form_stub* stub) {
      auto* item = new QStandardItem(QString::fromStdString(stub->get_editor_id()));
      item->setData(stub->formID, FormIDRole);
      item->setData(QVariant::fromValue(stub), FormStubRole);
      return item;
   }

   //
   // Weightings for the widget. When we allow a large number of form types, we will split 
   // the listing into a form-type combobox and a form combobox. We judge this based on the 
   // number of "common" forms that exist in Skyrim.esm, where a common form is any form 
   // that is not a reference, and is not INFO, LAND, NAVI, or NAVM.
   //
   // We use a weighted count -- so, some form types will push us to splitting the listing 
   // more than others. Here are all form types that represent more than 1.5% of all common 
   // forms in Skyrim.esm. All form types have a minimum weighting of 1, so these values 
   // are added to that, i.e. a 3% count should be listed as 2 here.
   //
   static const QMap<dovah::form_type_t, int> _form_type_split_weighting = {
      { dovah::form_type::activator,         1 }, //  1.84% of common forms
      { dovah::form_type::actor_base,        4 }, //  5.05% of common forms
      { dovah::form_type::armor,             2 }, //  2.72% of common forms
      { dovah::form_type::dialogue_branch,   2 }, //  3.02% of common forms
      { dovah::form_type::idle,              2 }, //  3.24% of common forms
      { dovah::form_type::leveled_item,      2 }, //  3.03% of common forms
      { dovah::form_type::package,           5 }, //  5.88% of common forms
      { dovah::form_type::quest,             1 }, //  1.78% of common forms
      { dovah::form_type::scene,             1 }, //  1.68% of common forms
      { dovah::form_type::sound_descriptor,  2 }, //  2.42% of common forms
      { dovah::form_type::statik,            9 }, //  9.59% of common forms
      { dovah::form_type::topic,            14 }, // 14.84% of common forms
      { dovah::form_type::weapon,            1 }, //  2.45% of common forms
   };
}

FormPicker::FormPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
   this->setLayout(layout);
   
   this->subwidgets.form = new QComboBox(this);
   this->subwidgets.type = new QComboBox(this);
   this->subwidgets.form->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
   this->subwidgets.type->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);

   QObject::connect(this->subwidgets.type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      if (auto* stub = this->formStub())
         this->_prior_selections[stub->formType] = stub->formID;
      //
      this->_updateForms();
   });
   {  // Set up form-combobox models
      auto* widget = this->subwidgets.form;
      auto* model  = new QStandardItemModel(widget);
      auto* proxy  = new _FormsOfTypeComboboxProxy(widget);
      proxy->setSortCaseSensitivity(Qt::CaseInsensitive);
      proxy->setSourceModel(model);
      widget->setModel(proxy);
      proxy->sort(0);
   }

   this->setDisabled(true);

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      if (!this->_activated)
         return;
      this->_updateForms();
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      const auto blocker0 = QSignalBlocker(this->subwidgets.form);
      const auto blocker1 = QSignalBlocker(this->subwidgets.type);
      this->setDisabled(true);
      this->subwidgets.type->clear();
      this->subwidgets.form->clear();
      //
      this->_prior_selections.clear();
      auto& prior = this->pre_activate_value;
      prior.id    = 0;
      prior.stub  = nullptr;
   });
   QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub) {
      if (!this->_activated) {
         auto& prior = this->pre_activate_value;
         if (prior.id == stub->formID)
            prior.id   = 0;
         if (prior.stub == stub)
            prior.stub = nullptr;
         return;
      }
      {
         auto& ps = this->_prior_selections;
         auto  it = ps.find(stub->formType);
         if (it != ps.end())
            this->_prior_selections.erase(it);
      }
      auto* c_form = this->subwidgets.form;
      int   index  = c_form->findData(stub->formID);
      if (index < 0)
         return;
      int si = c_form->currentIndex();
      c_form->removeItem(index);
      if (index == si && this->allowNone())
         c_form->setCurrentIndex(0);
   });
   QObject::connect(&editor, &DovahKitCore::formCreated, this, [this](dovah::form_stub* stub) {
      if (!this->_activated)
         return;
      const auto blocker = QSignalBlocker(this);
      if (_should_exclude_form(stub))
         return;
      if (!this->allowsFormType(stub->formType))
         return;
      auto* model = this->_rawModel();
      int   index = this->subwidgets.form->findData(stub->formID);
      if (index >= 0)
         return;
      auto* item = new QStandardItem(QString::fromStdString(stub->get_editor_id()));
      item->setData(stub->formID, FormIDRole);
      item->setData(QVariant::fromValue((void*)stub), FormStubRole);
      model->appendRow(item);
   });
   QObject::connect(&editor, &DovahKitCore::formModified, this, [this](dovah::form_stub* stub) {
      if (!this->_activated)
         return;
      if (!this->allowsFormType(stub->formType))
         return;
      const auto blocker = QSignalBlocker(this);
      auto* c_form = this->subwidgets.form;
      int   index  = c_form->findData(stub->formID);
      if (index < 0)
         return;
      c_form->setItemText(index, stub->get_editor_id());
   });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
      if (!this->_activated) {
         auto& prior = this->pre_activate_value;
         if (prior.id == oldID)
            prior.id = newID;
         return;
      }
      if (!this->allowsFormType(stub->formType))
         return;
      //
      {
         auto& ps = this->_prior_selections;
         auto  it = ps.find(stub->formType);
         if (it != ps.end()) {
            if (oldID == *it)
               *it = newID;
         }
      }
      //
      const auto blocker = QSignalBlocker(this);
      auto* c_form = this->subwidgets.form;
      int   index  = c_form->findData(QVariant::fromValue((void*)stub), FormStubRole);
      if (index < 0)
         return;
      c_form->setItemData(index, newID, FormIDRole);
   });
}

dovah::bare_form_id_t FormPicker::formID() const noexcept {
   auto* stub = this->formStub();
   if (stub)
      return stub->formID;
   return 0;
}
dovah::form_stub* FormPicker::formStub() const noexcept {
   auto data = this->subwidgets.form->currentData(FormStubRole);
   if (data.isValid())
      return (dovah::form_stub*)this->subwidgets.form->currentData(FormStubRole).value<dovah::form_stub*>();
   return nullptr;
}

void FormPicker::addFormType(dovah::form_type_t ft) {
   if (this->_formTypes.contains(ft))
      return;
   this->_formTypes.push_back(ft);
   //
   if (!this->_activated)
      return;
   //
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   bool split = this->_shouldSplitTypes();
   if (split) {
      if (!this->isSplittingTypes()) {
         this->_startSplittingTypes();
         return;
      }
      this->_updateTypes();
   } else {
      this->_addFormsOfType(ft);
   }
}
void FormPicker::setAllowedFormTypes(QVector<dovah::form_type_t> t) noexcept {
   this->_formTypes = t;
   this->_prior_selections.clear();
   //
   if (!this->_activated)
      return;
   //
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   bool split = this->_shouldSplitTypes();
   if (split) {
      if (!this->isSplittingTypes()) {
         this->_startSplittingTypes();
         return;
      }
      this->_updateTypes();
   }
   this->_updateForms();
}
void FormPicker::setSplitTypesWhenMany(bool b) noexcept {
   this->_splitTypesWhenMany = b;
   bool now    = this->isSplittingTypes();
   bool should = this->_shouldSplitTypes();
   if (now != should) {
      if (should)
         this->_startSplittingTypes();
      else
         ; // TODO
   }
}

void FormPicker::setAllowNone(bool b) noexcept {
   if (b == this->allowNone())
      return;
   this->_allowNone = b;
   if (b) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      this->subwidgets.form->addItem(text, 0);
   } else {
      auto* stub = this->formStub();
      int   i    = this->subwidgets.form->findData(0, FormIDRole);
      if (i >= 0)
         this->subwidgets.form->removeItem(i);
      if (!stub) {
         //
         // TODO: select default stub
         //
      }
   }
}
void FormPicker::setNoneLabel(const QString& n) noexcept {
   this->_noneLabel = n;
   if (!this->allowNone())
      return;
   int index = this->subwidgets.form->findData(0, FormIDRole);
   if (index >= 0)
      this->subwidgets.form->setItemText(index, this->_noneLabel);
}

void FormPicker::setFormByID(dovah::bare_form_id_t id) noexcept {
   if (id == 0) {
      if (!this->allowNone())
         return;
   }
   int index = this->subwidgets.form->findData(id, FormIDRole);
   if (index >= 0) {
      this->_prior_selections.clear();
      this->subwidgets.form->setCurrentIndex(index);
   }
}
void FormPicker::setFormStub(dovah::form_stub* stub) noexcept {
   if (!stub) {
      if (!this->allowNone())
         return;
      this->_prior_selections.clear();
      int index = this->subwidgets.form->findData(0);
      if (index >= 0)
         this->subwidgets.form->setCurrentIndex(index);
      return;
   }
   if (!this->_formTypes.contains(stub->formType))
      return;
   this->_prior_selections.clear();
   int index = this->subwidgets.form->findData(QVariant::fromValue(stub), FormStubRole);
   if (index >= 0)
      this->subwidgets.form->setCurrentIndex(index);
}

void FormPicker::changeEvent(QEvent* event) {
   if (event->type() != QEvent::ParentChange) // ParentAboutToChange can't be received here
      return;
   if (this->_activated)
      return;
   this->_activate();
}

QStandardItemModel* FormPicker::_rawModel() const noexcept {
   auto* proxy = (_FormsOfTypeComboboxProxy*) this->subwidgets.form->model();
   assert(proxy);
   auto* model = (QStandardItemModel*) proxy->sourceModel();
   assert(model);
   return model;
}

void FormPicker::_activate() {
   if (this->_activated)
      return;
   this->_activated = true;
   //
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   //
   this->_updateTypes();
   this->_updateForms();
   //
   auto& prior = this->pre_activate_value;
   if (prior.id || prior.stub) {
      if (prior.id)
         this->setFormByID(prior.id);
      else if (prior.stub)
         this->setFormStub(prior.stub);
      prior.id = 0;
      prior.stub = nullptr;
      //
      auto* c_type = this->subwidgets.type;
      if (c_type->isVisible()) {
         if (auto* stub = this->formStub()) {
            auto index = c_type->findData(stub->formType);
            if (index >= 0)
               c_type->setCurrentIndex(index);
         }
      }
   }
   //
   this->setDisabled(this->_rawModel()->rowCount() == 0);
}
void FormPicker::_addFormsOfType(dovah::form_type_t ft) noexcept {
   auto* model = this->_rawModel();
   DovahKitCore::get().for_each_form_of_type(ft, [model](dovah::form_stub* stub) {
      if (_should_exclude_form(stub))
         return false; // continue
      model->appendRow(_make_form_item(stub));
      return false; // continue
   });
}
bool FormPicker::_shouldSplitTypes() const noexcept {
   if (!this->_splitTypesWhenMany)
      return false;
   if (this->_formTypes.isEmpty())
      return true;
   int total = this->_formTypes.size();
   for (auto ft : this->_formTypes) {
      auto it = _form_type_split_weighting.find(ft);
      if (it != _form_type_split_weighting.end())
         total += *it;
   }
   if (total > 15) // TODO: fine-tune this
      return true;
   return false;
}
void FormPicker::_startSplittingTypes() noexcept {
   if (this->isSplittingTypes())
      return;
   auto* stub = this->formStub();
   this->_updateTypes();
   this->subwidgets.type->setVisible(true);
   this->_updateForms();
}
void FormPicker::_updateForms() {
   auto& editor = DovahKitCore::get();
   auto* c_form = this->subwidgets.form;
   auto* c_type = this->subwidgets.type;
   auto* model  = (QStandardItemModel*) c_form->model();
   //
   auto* prior  = this->formStub();
   //
   const auto blocker = QSignalBlocker(this->subwidgets.form);
   if (this->isSplittingTypes()) {
      auto ft = (dovah::form_type_t)c_type->currentData().toInt();
      //
      c_form->clear();
      this->_addFormsOfType(ft);
      //
      if (!prior || prior->formType != ft) {
         dovah::bare_form_id_t id = 0;
         auto it = this->_prior_selections.find(ft);
         if (it != this->_prior_selections.end())
            prior = DovahKitCore::get().get_form(*it);
      }
   } else {
      c_form->clear();
      for (auto ft : this->_formTypes)
         this->_addFormsOfType(ft);
   }
   if (this->_allowNone) {
      QString text = this->_noneLabel;
      if (text.isEmpty())
         text = tr("NONE");
      auto* item = new QStandardItem(text);
      item->setData(0, FormIDRole);
      item->setData(QVariant::fromValue<dovah::form_stub*>(nullptr), FormStubRole);
      model->appendRow(item);
   }
   //
   // Re-select prior:
   //
   int index = c_form->findData(QVariant::fromValue(prior), FormStubRole);
   if (index >= 0) {
      c_form->setCurrentIndex(index);
   } else {
      c_form->setCurrentIndex(0);
   }
}
void FormPicker::_updateTypes() {
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   //
   auto* c_type = this->subwidgets.type;
   auto* model  = c_type->model();
   auto  prior  = c_type->currentData().toInt();
   //
   c_type->clear();
   if (this->_formTypes.isEmpty()) {
      for (const auto& type : dovah::form_types) {
         c_type->addItem(cobb::qt::four_cc_to_string(type.signature), type.formType);
      }
   } else {
      for (auto ft : this->_formTypes) {
         auto& type = dovah::form_type_info::lookup(ft);
         c_type->addItem(cobb::qt::four_cc_to_string(type.signature), type.formType);
      }
   }
   int index = c_type->findData(prior);
   if (index >= 0) {
      c_type->setCurrentIndex(index);
   }
}