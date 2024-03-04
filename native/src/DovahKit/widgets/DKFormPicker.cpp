#include "./DKFormPicker.h"
#include <QBoxLayout>
#include <QComboBox>
#include <QEvent>
#include <QListView>
#include <QStandardItemModel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/form_stub.h"
   #include "editor/core.h"
   #include "editor/form_stub_meta_type.h"
   #include "helpers/qt/strings.h"

   #include "./widget-models/DKFormPicker/DKFormPickerModel.h"
#endif

namespace {
   using model_type = ui::impl::DKFormPicker::Model;
}

namespace {
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

DKFormPicker::DKFormPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
   this->setLayout(layout);
   
   this->_subwidgets.type = new QComboBox(this);
   this->_subwidgets.form = new QComboBox(this);
   this->_subwidgets.form->setDisabled(true);
   layout->addWidget(this->_subwidgets.type, 0);
   layout->addWidget(this->_subwidgets.form, 1);
   layout->setMargin(0);
   layout->setSizeConstraint(QLayout::SizeConstraint::SetMinimumSize);
   this->setFocusProxy(this->_subwidgets.type);
   this->setTabOrder(this->_subwidgets.type, this->_subwidgets.form);

   //
   // Make preparations for performance with massive comboboxes:
   //
   {
      auto* combobox = this->_subwidgets.form;
      combobox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
      auto* view = qobject_cast<QListView*>(combobox->view());
      if (view) { // condition, just in case the library internals change later
         view->setUniformItemSizes(true);
         view->setLayoutMode(QListView::Batched);
         view->setBatchSize(50);
      }
   }

   // Handle our comboboxes changing.
   #if !defined(QT_DESIGNER_LIB)
      QObject::connect(this->_subwidgets.type, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         if (auto* stub = this->formStub())
            this->_prior_selections[stub->formType] = stub;
         //
         this->_updateForms();
      });
      QObject::connect(this->_subwidgets.form, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         if (this->_rawModel()->isFilling()) // the QComboBox changes its value when the model fills, which can happen async -- and before we have a chance to make it select our `_value`.
            return;
         this->_value = this->_subwidgets.form->currentData(model_type::FormStubRole).value<dovah::form_stub*>();
         emit formChanged(this->formStub());
      });
   #endif

   {  // Set up form-combobox models
      auto* widget = this->_subwidgets.form;
      auto* model  = new model_type(widget);
      widget->setModel(model);
      QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
         this->_subwidgets.form->setEnabled(this->_subwidgets.form->count() > 0);
      });
      QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
         this->_subwidgets.form->setEnabled(this->_subwidgets.form->count() > 0);
      });
      #if !defined(QT_DESIGNER_LIB)
         QObject::connect(model, &model_type::filled, this, [this]() {
            auto* model = this->_rawModel();
            auto* prior = this->_value;
            auto* stub  = this->_value;
            if (!stub && !this->allowNone())
               stub = this->defaultForm();

            int i = model->indexOf(stub);
            if (this->isSplittingTypes()) {
               auto data = this->_subwidgets.type->currentData();
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
               this->_subwidgets.form->setCurrentIndex(i);
            this->_value = this->_subwidgets.form->currentData(model_type::FormStubRole).value<dovah::form_stub*>();
            this->_setSubwidgetEnableState(model->rowCount({}) != 0);
            //
            if (this->_value != prior) {
               emit formChanged(this->_value);
            }
         });
      #endif
   }

   #if !defined(QT_DESIGNER_LIB)
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, [this]() {
         this->_updateTypePicker();
         this->_updateForms();
      });
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         const auto blocker0 = QSignalBlocker(this->_subwidgets.form);
         const auto blocker1 = QSignalBlocker(this->_subwidgets.type);
         this->_setSubwidgetEnableState(false);
         this->_subwidgets.type->clear();
         this->_prior_selections.clear();
         this->_value = nullptr;
      });
   #endif
}

QList<dovah::form_type_t> DKFormPicker::allowedFormTypesList() const {
   return QList(this->_properties.allowed_form_types.begin(), this->_properties.allowed_form_types.end());
}
void DKFormPicker::addAllowedFormType(dovah::form_type_t ft) {
   if (this->allowsFormType(ft))
      return;
   this->_properties.allowed_form_types.push_back(ft);
   if (this->_properties.allowed_form_types.empty()) {
      #if !defined(QT_DESIGNER_LIB)
         this->_prior_selections.clear();

         if (this->_value && this->_value->formType != ft)
            this->_value = nullptr;
      #endif
   }
   //
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateForms();
}
void DKFormPicker::setAllowedFormTypes(QList<dovah::form_type_t> t) noexcept {
   {
      this->_properties.allowed_form_types.clear();
      this->_properties.allowed_form_types.resize(t.size());
      for (size_t i = 0; i < t.size(); ++i)
         this->_properties.allowed_form_types[i] = t[i];
   }
   #if !defined(QT_DESIGNER_LIB)
      this->_prior_selections.clear();
   #endif
   //
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateForms();
}
void DKFormPicker::setSplitTypesWhenMany(bool b) noexcept {
   this->_properties.split_types_when_many = b;
   bool now    = this->isSplittingTypes();
   bool should = this->_shouldSplitTypes();
   if (now != should)
      this->_setIsSplittingTypes(should);
}

void DKFormPicker::setAllowNone(bool b) noexcept {
   if (b == this->allowNone())
      return;
   this->_properties.allow_none = b;
   this->_updateForms();
}

#if !defined(QT_DESIGNER_LIB)
void DKFormPicker::setFormStub(dovah::form_stub* stub) noexcept {
   if (!stub) {
      if (!this->allowNone()) {
         if (this->_default)
            this->setFormStub(this->_default);
         return;
      }
   } else {
      if (!this->_properties.allowed_form_types.contains(stub->formType))
         return;
   }
   this->_value = stub;
   this->_prior_selections.clear();
   //
   auto* subwidget = this->_subwidgets.form;
   int   index     = subwidget->findData(QVariant::fromValue(stub), model_type::FormStubRole);
   if (index >= 0) {
      const auto blocker = QSignalBlocker(subwidget);
      subwidget->setCurrentIndex(index);
   }
}
void DKFormPicker::setDefaultForm(dovah::form_stub* stub) noexcept {
   this->_default = stub;
}

model_type* DKFormPicker::_rawModel() const noexcept {
   auto* proxy = (model_type*) this->_subwidgets.form->model();
   assert(proxy);
   return proxy;
}
#endif

void DKFormPicker::_setIsSplittingTypes(bool s) noexcept {
   this->_state.is_splitting_types = s;
   this->_subwidgets.type->setVisible(s);
   this->setFocusProxy(s ? this->_subwidgets.type : this->_subwidgets.form); // needed to prevent tabbing from breaking when the "type" subwidget is hidden
   if (s)
      this->_updateTypePicker();
   else
      this->_updateForms();
}
void DKFormPicker::_setSubwidgetEnableState(bool s) {
   this->_subwidgets.type->setEnabled(s);
   this->_subwidgets.form->setEnabled(s);
}
bool DKFormPicker::_shouldSplitTypes() const noexcept {
   if (!this->splitTypesWhenMany())
      return false;
   if (this->_properties.allowed_form_types.isEmpty())
      return true;
   int total = this->_properties.allowed_form_types.size();
   for (auto ft : this->_properties.allowed_form_types) {
      auto it = _form_type_split_weighting.find(ft);
      if (it != _form_type_split_weighting.end())
         total += *it;
   }
   if (total > 15) // TODO: fine-tune this
      return true;
   return false;
}
void DKFormPicker::_updateForms() {
   auto* c_form = this->_subwidgets.form;
   auto* stub   = this->formStub();
   auto* model  = this->_rawModel();
   
   const auto blocker = QSignalBlocker(c_form);

   #if !defined(QT_DESIGNER_LIB)
      model_type::filter_parameters params;
      params.allow_none = this->allowNone();
      if (this->isSplittingTypes()) {
         auto ftd = this->_subwidgets.type->currentData();
         if (!ftd.isValid()) {
            auto* c_type = this->_subwidgets.type;
            const auto blocker = QSignalBlocker(c_type);
            c_type->setCurrentIndex(0);
            ftd = c_type->currentData();
         }
         params.form_types = { (dovah::form_type_t)ftd.toInt() };
      } else {
         params.form_types = QList(this->_properties.allowed_form_types.begin(), this->_properties.allowed_form_types.end());
      }
      this->_rawModel()->updateParameters(params);
   #endif
}
void DKFormPicker::_updateTypePicker() {
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   //
   auto* c_type = this->_subwidgets.type;
   auto  prior  = c_type->currentData().toInt();
   auto* stub   = this->formStub();
   //
   c_type->clear();
   if (this->_properties.allowed_form_types.isEmpty()) {
      for (const auto& type : dovah::form_types) {
         c_type->addItem(cobb::qt::four_cc_to_string(type.signature), type.form_type);
      }
   } else {
      for (auto ft : this->_properties.allowed_form_types) {
         auto& type = dovah::form_type_info::lookup(ft);
         c_type->addItem(cobb::qt::four_cc_to_string(type.signature), type.form_type);
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

void DKFormPicker::clear() {
   #if !defined(QT_DESIGNER_LIB)
      dovah::form_stub* stub = nullptr;
      if (!this->allowNone()) {
         stub = this->defaultForm();
         if (!stub)
            return;
      }
      this->setFormStub(stub);
   #endif
}