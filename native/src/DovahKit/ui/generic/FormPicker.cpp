#include "FormPicker.h"
#include "impl/FormPickerImpl.h"
#include <QBoxLayout>
#include <QEvent>
#include <QListView>
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
   this->subwidgets.form->setDisabled(true);
   layout->addWidget(this->subwidgets.type, 0);
   layout->addWidget(this->subwidgets.form, 1);
   layout->setMargin(0);

   //
   // Make preparations for performance with massive comboboxes:
   //
   {
      auto* combobox = this->subwidgets.form;
      combobox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
      //
      // When dealing with especially massive comboxes (thousands of elements, as in the case of STAT 
      // forms), even the above is not enough...
      //
      auto* view = qobject_cast<QListView*>(combobox->view());
      if (view) { // just in case the library internals change later
         view->setUniformItemSizes(true);
         view->setLayoutMode(QListView::Batched);
         view->setBatchSize(50);
      }
   }

   QObject::connect(this->subwidgets.type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      if (auto* stub = this->formStub())
         this->_prior_selections[stub->formType] = stub->formID;
      //
      this->_updateForms();
   });
   {  // Set up form-combobox models
      auto* widget = this->subwidgets.form;
      auto* model  = new FormPickerImpl::FormPickerProxyModel(widget);
      widget->setModel(model);
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
         this->subwidgets.form->setEnabled(this->subwidgets.form->count() > 0);
      });
      QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
         this->subwidgets.form->setEnabled(this->subwidgets.form->count() > 0);
      });
   }

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
      this->_prior_selections.clear();
   });
   QObject::connect(&editor, &DovahKitCore::formRenumbered, this, [this](dovah::form_stub* stub, dovah::bare_form_id_t oldID, dovah::bare_form_id_t newID) {
      auto& ps = this->_prior_selections;
      auto  it = ps.find(stub->formType);
      if (it != ps.end())
         if (oldID == it->second)
            it->second = newID;
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
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateForms();
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
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateForms();
}
void FormPicker::setSplitTypesWhenMany(bool b) noexcept {
   this->_splitTypesWhenMany = b;
   bool now    = this->isSplittingTypes();
   bool should = this->_shouldSplitTypes();
   if (now != should)
      this->_setIsSplittingTypes(should);
}

void FormPicker::setAllowNone(bool b) noexcept {
   if (b == this->allowNone())
      return;
   this->_allowNone = b;
   this->_updateForms();
}

void FormPicker::setFormByID(dovah::bare_form_id_t id) noexcept {
   this->setFormStub(DovahKitCore::get().get_form(id));
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
void FormPicker::showEvent(QShowEvent* event) {
   if (this->_activated)
      return;
   this->_activate();
}

FormPickerImpl::FormPickerProxyModel* FormPicker::_rawModel() const noexcept {
   auto* proxy = (FormPickerImpl::FormPickerProxyModel*) this->subwidgets.form->model();
   assert(proxy);
   return proxy;
}

void FormPicker::_activate() {
   if (this->_activated)
      return;
   this->_activated = true;
   //
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   //
   this->_updateTypePicker();
   this->_updateForms();
}
void FormPicker::_setIsSplittingTypes(bool s) noexcept {
   this->subwidgets.type->setVisible(s);
   if (s)
      this->_updateTypePicker();
   else
      this->_updateForms();
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
void FormPicker::_updateForms() {
   if (this->isSplittingTypes()) {
      auto ft = (dovah::form_type_t) this->subwidgets.type->currentData().toInt();
      this->_rawModel()->updateParameters(this->_allowNone, { ft });
   } else {
      this->_rawModel()->updateParameters(this->_allowNone, this->_formTypes);
   }
}
void FormPicker::_updateTypePicker() {
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   //
   auto* c_type = this->subwidgets.type;
   auto* model  = c_type->model();
   auto  prior  = c_type->currentData().toInt();
   auto* stub   = this->formStub();
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
   int index;
   if (stub)
      index = c_type->findData(stub->formType);
   else
      index = c_type->findData(prior);
   if (index < 0)
      index = 0;
   c_type->setCurrentIndex(index);
}