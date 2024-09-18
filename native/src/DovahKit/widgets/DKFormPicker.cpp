#include "./DKFormPicker.h"
#include <cstdint>
#include <QBoxLayout>
#include <QComboBox>
#include <QEvent>
#include <QListView>
#include <QMap>
#include <QStandardItemModel>
#if !defined(QT_DESIGNER_LIB)
   #include "dovah/form_stub.h"
   #include "editor/subsystems/papyrus/core.h"
   #include "editor/core.h"
   #include "editor/form_stub_meta_type.h"

   #include "./widget-models/DKFormPicker/DKFormPickerModel.h"
   #include "./widget-data/DKFormPickerCustomFilter.h"
#endif
#include "./DKComboBox.h"

#if !defined(QT_DESIGNER_LIB)
   namespace {
      using model_type = ui::impl::DKFormPicker::Model;
   }
#endif

namespace {
   QString _four_cc_to_string(uint32_t signature) {
      return QString("%1%2%3%4")
         .arg(QChar(signature >> 0x18))
         .arg(QChar((signature >> 0x10) & 0xFF))
         .arg(QChar((signature >> 0x08) & 0xFF))
         .arg(QChar(signature & 0xFF));
   }

   bool _allow_form_type(const dovah::form_type_info& info) {
      using flag = dovah::form_type_info::flag;
      if (info.flags & (flag::no_editor_id | flag::no_connections))
         return false;
      if (info.form_type == dovah::form_type::none)
         return false;
      return true;
   }
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
   static const QMap<dovah::form_type, int> _form_type_split_weighting = {
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
   this->_subwidgets.form = new DKComboBox(this);
   this->_subwidgets.form->setDisabled(true);
   layout->addWidget(this->_subwidgets.type, 0);
   layout->addWidget(this->_subwidgets.form, 1);
   layout->setMargin(0);
   layout->setStretch(1, 1);
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.type);
   this->setTabOrder(this->_subwidgets.type, this->_subwidgets.form);

   //
   // Make preparations for performance with massive comboboxes:
   //
   {
      auto* combobox = this->_subwidgets.form;
      combobox->setSizeAdjustPolicy(QComboBox::AdjustToContentsOnFirstShow);
      combobox->setAutoResizeEnabled(false); // DKComboBox: force the combobox to let its contents be truncated
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
         if (!this->isSplittingTypes())
            return;

         if (auto* stub = this->formStub())
            this->_prior_selections[stub->form_type] = stub;
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
      #if !defined(QT_DESIGNER_LIB)
         auto* model  = new model_type(widget);
         widget->setModel(model);
         QObject::connect(model, &QAbstractItemModel::rowsInserted, this, [this]() {
            this->_subwidgets.form->setEnabled(this->_subwidgets.form->count() > 0);
         });
         QObject::connect(model, &QAbstractItemModel::rowsRemoved, this, [this]() {
            this->_subwidgets.form->setEnabled(this->_subwidgets.form->count() > 0);
         });
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
                  auto ft = (dovah::form_type) data.toInt();
                  if (!stub || stub->form_type != ft) {
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
            this->_subwidgets.form->setEnabled(model->rowCount({}) != 0);
            this->_subwidgets.type->setEnabled(true);
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

void DKFormPicker::addAllowedFormType(dovah::form_type ft) {
   if (this->allowsFormType(ft))
      return;
   this->_properties.allowed_form_types.push_back(ft);
   if (this->_properties.allowed_form_types.empty()) {
      #if !defined(QT_DESIGNER_LIB)
         this->_prior_selections.clear();

         if (this->_value && this->_value->form_type != ft)
            this->_value = nullptr;
      #endif
   }
   //
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateTypePicker();
   this->_updateForms();
}
void DKFormPicker::setAllowedFormTypes(QList<dovah::form_type> t) noexcept {
   this->_properties.allowed_form_types = t;
   #if !defined(QT_DESIGNER_LIB)
      this->_prior_selections.clear();
   #endif
   //
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   this->_setIsSplittingTypes(this->_shouldSplitTypes());
   this->_updateTypePicker();
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

QString DKFormPicker::overrideTextForNone() const {
   #if defined(QT_DESIGNER_LIB)
      return this->_properties.override_text_for_none;
   #else
      return this->_rawModel()->overrideTextForNone();
   #endif
}
void DKFormPicker::setOverrideTextForNone(QString s) {
   #if defined(QT_DESIGNER_LIB)
      if (this->overrideTextForNone() == s)
         return;
      this->_properties.override_text_for_none = s;
      this->_updateForms();
   #else
      this->_rawModel()->setOverrideTextForNone(s);
   #endif
}

QString DKFormPicker::requiredScriptname() const {
   return this->_properties.scriptname_on_form;
}
void DKFormPicker::setRequiredScriptname(QString s) {
   if (s == this->requiredScriptname())
      return;
   this->_properties.scriptname_on_form = s;
   this->_updateForms();
}
void DKFormPicker::setRequiredScriptname(std::string_view s) {
   this->setRequiredScriptname(QString::fromUtf8(s.data(), s.size()));
}

QString DKFormPicker::requiredAliasScriptname() const {
   return this->_properties.scriptname_on_alias;
}
void DKFormPicker::setRequiredAliasScriptname(QString s) {
   if (s == this->requiredAliasScriptname())
      return;
   this->_properties.scriptname_on_alias = s;
   this->_updateForms();
}
void DKFormPicker::setRequiredAliasScriptname(std::string_view s) {
   this->setRequiredAliasScriptname(QString::fromUtf8(s.data(), s.size()));
}

#if !defined(QT_DESIGNER_LIB)
void DKFormPicker::setFormStub(dovah::form_stub* stub) noexcept {
   if (this->_value == stub) {
      this->_updateForceIncludedForm(stub);
      return;
   }
   if (!stub) {
      if (!this->allowNone()) {
         if (this->_default)
            this->setFormStub(this->_default);
         return;
      }
   } else {
      if (!this->_wouldAllowFormStub(*stub))
         return;
   }

   this->_value = stub;
   this->_prior_selections.clear();

   auto*      subwidget = this->_subwidgets.form;
   const auto blocker   = QSignalBlocker(subwidget);

   this->_updateForceIncludedForm(stub);
   if (stub) {
      auto* c_type = this->_subwidgets.type;
      int   index  = c_type->findData((int)stub->form_type);
      if (index >= 0) {
         c_type->setCurrentIndex(index);
      }
   }
   
   int index = subwidget->findData(QVariant::fromValue(stub), model_type::FormStubRole);
   if (index >= 0) {
      subwidget->setCurrentIndex(index);
      emit this->formChanged(stub);
   } else {
      if (auto* model = this->_rawModel())
         if (model->isFilling())
            emit this->formChanged(stub); // Assume we're going to succeed. If we fail, we'll emit formChanged when correcting ourselves.
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

bool DKFormPicker::_wouldAllowFormStub(const dovah::form_stub& stub) const {
   if (!this->_properties.allowed_form_types.isEmpty() && !this->_properties.allowed_form_types.contains(stub.form_type))
      return false;

   bool needs_alias_script = stub.form_type == dovah::form_type::quest && !this->_properties.scriptname_on_alias.isEmpty();
   bool needs_form_script  = !this->_properties.scriptname_on_form.isEmpty();
   if (needs_alias_script || needs_form_script) {
      auto& papyrus = dovahkit::subsystems::papyrus::core::get();
      if (needs_form_script  && !papyrus.form_has_script_attached(stub, this->_properties.scriptname_on_form.toStdString()))
         return false;
      if (needs_alias_script && !papyrus.quest_has_script_attached_to_any_alias(stub, this->_properties.scriptname_on_alias.toStdString()))
         return false;
   }

   return true;
}

/*virtual*/ void DKFormPicker::changeEvent(QEvent* event) /*override*/ {
   //
   // If the widget is attached to the UI without ever having parameters configured 
   // on it (i.e. the stock defaults), then we need to populate the widget at that 
   // time.
   //
   if (!this->_state.needs_initial_fill)
      return;
   if (event->type() != QEvent::ParentChange)
      return;
   this->_state.needs_initial_fill = false;
   this->_updateTypePicker();
   this->_updateForms();
}
#endif

void DKFormPicker::_setIsSplittingTypes(bool s) noexcept {
   if (this->_state.is_splitting_types == s)
      return;
   this->_state.is_splitting_types = s;
   this->_subwidgets.type->setVisible(s);
   this->setFocusProxy(s ? this->_subwidgets.type : this->_subwidgets.form); // needed to prevent tabbing from breaking when the "type" subwidget is hidden
   if (s)
      this->_updateTypePicker();
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
void DKFormPicker::_updateForceIncludedForm(dovah::form_stub* stub) {
   auto* prior = this->_state.last_force_included_form;
   if (prior == stub)
      return;
   
   #if !defined(QT_DESIGNER_LIB)
      auto* model = this->_rawModel();
      if (prior) {
         model->setFormNeverDefaultExcluded(*prior, false);
      }
      if (stub) {
         model->setFormNeverDefaultExcluded(*stub, true);
      }
   #endif
   this->_state.last_force_included_form = stub;
}
void DKFormPicker::_updateForms() {
   this->_state.needs_initial_fill = false;

   #if !defined(QT_DESIGNER_LIB)
      auto* c_form = this->_subwidgets.form;
      auto* stub   = this->formStub();
      auto* model  = this->_rawModel();
   
      const auto blocker = QSignalBlocker(c_form);

      if (stub) {
         bool retain = this->_wouldAllowFormStub(*stub);
         this->_updateForceIncludedForm(retain ? stub : nullptr);
      } else {
         this->_updateForceIncludedForm(nullptr);
      }

      model_type::filter_parameters params;
      params.allow_none            = this->allowNone();
      params.scriptname            = this->requiredScriptname().toStdString();
      params.scriptname_on_aliases = this->requiredAliasScriptname().toStdString();
      if (this->isSplittingTypes()) {
         auto ftd = this->_subwidgets.type->currentData();
         if (!ftd.isValid()) {
            auto* c_type = this->_subwidgets.type;
            const auto blocker = QSignalBlocker(c_type);
            c_type->setCurrentIndex(0);
            ftd = c_type->currentData();
         }
         params.form_types = { (dovah::form_type)ftd.toInt() };
      } else {
         params.form_types = QList(this->_properties.allowed_form_types.begin(), this->_properties.allowed_form_types.end());
      }
      this->_rawModel()->updateParameters(params);
   #else
      auto* c_form = this->_subwidgets.form;

      const auto blocker = QSignalBlocker(c_form);

      c_form->clear();

      if (this->allowNone()) {
         QString none = this->_properties.override_text_for_none;
         if (none.isEmpty())
            none = tr("NONE");
         c_form->addItem(none);
      }
      c_form->addItem("ExampleForm01");
   #endif
}
void DKFormPicker::_updateTypePicker() {
   const auto blocker0 = QSignalBlocker(this->_subwidgets.type);
   const auto blocker1 = QSignalBlocker(this->_subwidgets.form);
   //
   auto* c_type = this->_subwidgets.type;
   auto  prior  = c_type->currentData().toInt();
   #if !defined(QT_DESIGNER_LIB)
      auto* stub = this->formStub();
   #endif
   //
   c_type->clear();
   if (this->_properties.allowed_form_types.isEmpty()) {
      for (const auto& type : dovah::form_types) {
         if (!_allow_form_type(type))
            continue;
         c_type->addItem(_four_cc_to_string(type.signature), (int)type.form_type);
      }
   } else {
      for (auto ft : this->_properties.allowed_form_types) {
         auto& type = dovah::form_type_info::lookup(ft);
         if (!_allow_form_type(type))
            continue;
         c_type->addItem(_four_cc_to_string(type.signature), (int)type.form_type);
      }
   }
   c_type->model()->sort(0);
   //
   #if !defined(QT_DESIGNER_LIB)
      int index;
      if (stub)
         index = c_type->findData((int)stub->form_type);
      else
         index = c_type->findData(prior);
      if (index < 0)
         index = 0;
      c_type->setCurrentIndex(index);
   #endif
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

#if !defined(QT_DESIGNER_LIB)
DKCustomFormFilter* DKFormPicker::customFilter() const {
   return this->_rawModel()->get_custom_filter();
}
void DKFormPicker::setCustomFilter(DKCustomFormFilter* v) {
   this->_rawModel()->set_custom_filter(v);
}
#endif