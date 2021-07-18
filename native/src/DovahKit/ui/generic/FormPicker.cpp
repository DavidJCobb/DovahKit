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
   static constexpr Qt::ItemDataRole FormIDRole    = FormPickerImpl::FormPickerSharedUnderlyingModel::FormIDRole;
   static constexpr Qt::ItemDataRole FormStubRole  = FormPickerImpl::FormPickerSharedUnderlyingModel::FormStubRole;

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
   
   this->subwidgets.type = new QComboBox(this);
   this->subwidgets.form = new QComboBox(this);
   this->subwidgets.form->setDisabled(true);
   layout->addWidget(this->subwidgets.type, 0);
   layout->addWidget(this->subwidgets.form, 1);
   layout->setMargin(0);
   this->setFocusProxy(this->subwidgets.type);
   this->setTabOrder(this->subwidgets.type, this->subwidgets.form);

   //
   // Make preparations for performance with massive comboboxes:
   //
   {
      auto* combobox = this->subwidgets.form;
      combobox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
      auto* view = qobject_cast<QListView*>(combobox->view());
      if (view) { // condition, just in case the library internals change later
         view->setUniformItemSizes(true);
         view->setLayoutMode(QListView::Batched);
         view->setBatchSize(50);
      }
   }

   QObject::connect(this->subwidgets.type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      if (auto* stub = this->formStub())
         this->_prior_selections[stub->formType] = stub;
      //
      this->_updateForms();
   });
   QObject::connect(this->subwidgets.form, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
      if (this->_rawModel()->isFilling()) // the QComboBox changes its value when the model fills, which can happen async -- and before we have a chance to make it select our (_value).
         return;
      this->_value = this->subwidgets.form->currentData(FormStubRole).value<dovah::form_stub*>();
      emit formChanged(this->formStub());
   });
   {  // Set up form-combobox models
      auto* widget = this->subwidgets.form;
      auto* model  = new FormPickerImpl::FormPickerIterativeModel(widget);
      widget->setModel(model);
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
         this->subwidgets.form->setEnabled(this->subwidgets.form->count() > 0);
      });
      QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
         this->subwidgets.form->setEnabled(this->subwidgets.form->count() > 0);
      });
      QObject::connect(model, &FormPickerImpl::FormPickerIterativeModel::filled, this, [this]() {
         auto* model = this->_rawModel();
         auto* prior = this->_value;
         auto* stub  = this->_value;
         if (!stub && !this->allowNone())
            stub = this->defaultForm();
         int   i     = model->indexOf(stub);
         if (this->isSplittingTypes()) {
            auto data = this->subwidgets.type->currentData();
            if (data.isValid()) {
               auto ft = (dovah::form_type_t) data.toInt();
               if (!stub || stub->formType != ft) {
                  auto it = this->_prior_selections.find(ft);
                  if (it != this->_prior_selections.end()) {
                     stub = it->second;
                     i    = model->indexOf(stub);
                  }
               }
            }
         }
         if (i >= 0)
            this->subwidgets.form->setCurrentIndex(i);
         this->_value = this->subwidgets.form->currentData(FormStubRole).value<dovah::form_stub*>();
         this->_setSubwidgetEnableState(model->rowCount(QModelIndex()) != 0);
         //
         if (this->_value != prior) {
            emit formChanged(this->_value);
         }
      });
   }

   auto& editor = DovahKitCore::get();
   QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
      this->_updateTypePicker();
      this->_updateForms();
   });
   QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
      const auto blocker0 = QSignalBlocker(this->subwidgets.form);
      const auto blocker1 = QSignalBlocker(this->subwidgets.type);
      this->_setSubwidgetEnableState(false);
      this->subwidgets.type->clear();
      this->_prior_selections.clear();
   });
}

dovah::bare_form_id_t FormPicker::formID() const noexcept {
   auto* stub = this->formStub();
   if (stub)
      return stub->formID;
   return 0;
}

void FormPicker::addFormType(dovah::form_type_t ft) {
   if (this->_formTypes.contains(ft))
      return;
   this->_formTypes.push_back(ft);
   if (this->_formTypes.empty()) {
      this->_prior_selections.clear();
      if (this->_value && this->_value->formType != ft)
         this->_value = nullptr;
   }
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
      if (!this->allowNone()) {
         if (this->_default)
            this->setFormStub(this->_default);
         return;
      }
   } else {
      if (!this->_formTypes.contains(stub->formType))
         return;
   }
   this->_value = stub;
   this->_prior_selections.clear();
   //
   auto* subwidget = this->subwidgets.form;
   int   index     = subwidget->findData(QVariant::fromValue(stub), FormStubRole);
   if (index >= 0) {
      const auto blocker = QSignalBlocker(subwidget);
      subwidget->setCurrentIndex(index);
   }
}
void FormPicker::setDefaultForm(dovah::form_stub* stub) noexcept {
   this->_default = stub;
}
void FormPicker::setDefaultFormID(dovah::bare_form_id_t id) noexcept {
   this->setDefaultForm(DovahKitCore::get().get_form(id));
}

FormPickerImpl::FormPickerIterativeModel* FormPicker::_rawModel() const noexcept {
   auto* proxy = (FormPickerImpl::FormPickerIterativeModel*) this->subwidgets.form->model();
   assert(proxy);
   return proxy;
}

void FormPicker::_setIsSplittingTypes(bool s) noexcept {
   this->_isSplittingTypes = s;
   this->subwidgets.type->setVisible(s);
   this->setFocusProxy(s ? this->subwidgets.type : this->subwidgets.form); // needed to prevent tabbing from breaking when the "type" subwidget is hidden
   if (s)
      this->_updateTypePicker();
   else
      this->_updateForms();
}
void FormPicker::_setSubwidgetEnableState(bool s) {
   this->subwidgets.type->setEnabled(s);
   this->subwidgets.form->setEnabled(s);
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
   auto* c_form = this->subwidgets.form;
   auto* stub   = this->formStub();
   auto* model  = this->_rawModel();
   //
   const auto blocker = QSignalBlocker(c_form);
   if (this->isSplittingTypes()) {
      auto ftd = this->subwidgets.type->currentData();
      if (!ftd.isValid()) {
         auto* c_type = this->subwidgets.type;
         const auto blocker = QSignalBlocker(c_type);
         c_type->setCurrentIndex(0);
         ftd = c_type->currentData();
      }
      this->_rawModel()->updateParameters(this->_allowNone, { (dovah::form_type_t)ftd.toInt() });
   } else {
      this->_rawModel()->updateParameters(this->_allowNone, this->_formTypes);
   }
}
void FormPicker::_updateTypePicker() {
   const auto blocker0 = QSignalBlocker(this->subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->subwidgets.form);
   //
   auto* c_type = this->subwidgets.type;
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
   c_type->model()->sort(0);
   //
   int index;
   if (stub)
      index = c_type->findData(stub->formType);
   else
      index = c_type->findData(prior);
   if (index < 0)
      index = 0;
   c_type->setCurrentIndex(index);
}

void FormPicker::clear() {
   dovah::form_stub* stub = nullptr;
   if (!this->allowNone()) {
      stub = this->defaultForm();
      if (!stub)
         return;
   }
   this->setFormStub(stub);
}