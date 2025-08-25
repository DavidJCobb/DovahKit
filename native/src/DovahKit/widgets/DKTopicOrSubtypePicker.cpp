#include "./DKTopicOrSubtypePicker.h"
#include <cstdint>
#include <QBoxLayout>
#include <QComboBox>
#include <QEvent>
#include <QListView>
#include <QMap>
#include <QStandardItemModel>
#if !defined(QT_PLUGIN)
   #include "dovah/form_stub.h"
   #include "dovah/form_types.h"
   #include "editor/subsystems/papyrus/core.h"
   #include "editor/core.h"
   #include "editor/form_stub_meta_type.h"

   #include "./widget-models/DKTopicOrSubtypePicker/DKTopicOrSubtypePickerModel.h"
   #include "./widget-data/DKCustomFormFilter.h"
#endif
#include "./DKComboBox.h"

#if !defined(QT_PLUGIN)
   namespace {
      using model_type = ui::impl::DKTopicOrSubtypePicker::Model;
   }
#endif

DKTopicOrSubtypePicker::DKTopicOrSubtypePicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
   this->setLayout(layout);
   
   this->_subwidgets.form = new DKComboBox(this);
   this->_subwidgets.form->setDisabled(true);
   layout->addWidget(this->_subwidgets.form, 0);
   layout->setMargin(0);
   layout->setStretch(1, 1);
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.form);

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
   #if !defined(QT_PLUGIN)
      QObject::connect(this->_subwidgets.form, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         if (this->_rawModel()->isFilling()) // the QComboBox changes its value when the model fills, which can happen async -- and before we have a chance to make it select our `_value`.
            return;
         this->_topic   = this->_subwidgets.form->currentData(model_type::FormStubRole).value<dovah::form_stub*>();
         this->_subtype = this->_subwidgets.form->currentData(model_type::SubtypeRole).value<uint32_t>();
         emit valueChanged(this->_topic, this->_subtype);
      });
   #endif

   {  // Set up form-combobox models
      auto* widget = this->_subwidgets.form;
      #if !defined(QT_PLUGIN)
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

            const auto* const prior_topic   = this->_topic;
            const auto        prior_subtype = this->_subtype;

            int i = 0;
            if (this->_topic) {
               i = model->rowOf(this->_topic);
            } else if (this->_subtype) {
               i = model->rowOf(this->_subtype);
            }
            if (i >= 0)
               this->_subwidgets.form->setCurrentIndex(i);
            this->_topic   = this->_subwidgets.form->currentData(model_type::FormStubRole).value<dovah::form_stub*>();
            this->_subtype = this->_subwidgets.form->currentData(model_type::SubtypeRole).value<uint32_t>();
            this->_subwidgets.form->setEnabled(model->rowCount({}) != 0);
            if (this->_topic != prior_topic || this->_subtype != prior_subtype) {
               emit valueChanged(this->_topic, this->_subtype);
            }
         });
      #endif
   }

   #if !defined(QT_PLUGIN)
      auto& editor = DovahKitCore::get();
      QObject::connect(&editor, &DovahKitCore::dataAcquireComplete, this, &DKTopicOrSubtypePicker::_updateForms);
      QObject::connect(&editor, &DovahKitCore::dataAbandonImminent, this, [this]() {
         const auto blocker0 = QSignalBlocker(this->_subwidgets.form);
         this->_setSubwidgetEnableState(false);
         this->_topic   = nullptr;
         this->_subtype = 0;
      });
   #endif
}

void DKTopicOrSubtypePicker::setAllowNone(bool b) noexcept {
   if (b == this->allowNone())
      return;
   this->_properties.allow_none = b;
   this->_updateForms();
}

QString DKTopicOrSubtypePicker::overrideTextForNone() const {
   #if defined(QT_PLUGIN)
      return this->_properties.override_text_for_none;
   #else
      return this->_rawModel()->overrideTextForNone();
   #endif
}
void DKTopicOrSubtypePicker::setOverrideTextForNone(QString s) {
   #if defined(QT_PLUGIN)
      if (this->overrideTextForNone() == s)
         return;
      this->_properties.override_text_for_none = s;
      this->_updateForms();
   #else
      this->_rawModel()->setOverrideTextForNone(s);
   #endif
}

#if !defined(QT_PLUGIN)
void DKTopicOrSubtypePicker::setTopic(dovah::form_stub* stub) noexcept {
   if (this->_topic == stub) {
      this->_updateForceIncludedForm(stub);
      return;
   }
   if (!stub) {
      if (!this->allowNone())
         return;
   } else {
      if (!this->_wouldAllowFormStub(*stub))
         return;
   }

   this->_topic   = stub;
   this->_subtype = 0;

   auto*      subwidget = this->_subwidgets.form;
   const auto blocker   = QSignalBlocker(subwidget);

   this->_updateForceIncludedForm(stub);
   
   int index = subwidget->findData(QVariant::fromValue(stub), model_type::FormStubRole);
   if (index >= 0) {
      subwidget->setCurrentIndex(index);
      emit this->valueChanged(stub, this->_subtype);
   } else {
      if (auto* model = this->_rawModel())
         if (model->isFilling())
            emit this->valueChanged(stub, this->_subtype); // Assume we're going to succeed. If we fail, we'll emit formChanged when correcting ourselves.
   }
}
void DKTopicOrSubtypePicker::setSubtype(uint32_t s) {
   if (this->_subtype == s) {
      return;
   }
   if (!s && !this->allowNone())
      return;

   this->_topic   = nullptr;
   this->_subtype = s;

   auto*      subwidget = this->_subwidgets.form;
   const auto blocker   = QSignalBlocker(subwidget);

   this->_updateForceIncludedForm(nullptr);
   
   int index = subwidget->findData(QVariant::fromValue(s), model_type::SubtypeRole);
   if (index >= 0) {
      subwidget->setCurrentIndex(index);
      emit this->valueChanged(nullptr, s);
   } else {
      if (auto* model = this->_rawModel())
         if (model->isFilling())
            emit this->valueChanged(nullptr, s); // Assume we're going to succeed. If we fail, we'll emit formChanged when correcting ourselves.
   }
}

model_type* DKTopicOrSubtypePicker::_rawModel() const noexcept {
   auto* proxy = (model_type*) this->_subwidgets.form->model();
   assert(proxy);
   return proxy;
}

bool DKTopicOrSubtypePicker::_wouldAllowFormStub(const dovah::form_stub& stub) const {
   return (stub.form_type == dovah::form_type::topic);
}

/*virtual*/ void DKTopicOrSubtypePicker::changeEvent(QEvent* event) /*override*/ {
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
   this->_updateForms();
}
#endif

void DKTopicOrSubtypePicker::_setSubwidgetEnableState(bool s) {
   this->_subwidgets.form->setEnabled(s);
}
void DKTopicOrSubtypePicker::_updateForceIncludedForm(dovah::form_stub* stub) {
   auto* prior = this->_state.last_force_included_form;
   if (prior == stub)
      return;
   
   #if !defined(QT_PLUGIN)
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
void DKTopicOrSubtypePicker::_updateForms() {
   this->_state.needs_initial_fill = false;

   #if !defined(QT_PLUGIN)
      auto* c_form = this->_subwidgets.form;
      auto* stub   = this->topic();
      auto* model  = this->_rawModel();
   
      const auto blocker = QSignalBlocker(c_form);

      if (stub) {
         bool retain = this->_wouldAllowFormStub(*stub);
         this->_updateForceIncludedForm(retain ? stub : nullptr);
      } else {
         this->_updateForceIncludedForm(nullptr);
      }

      model_type::filter_parameters params;
      params.allow_none = this->allowNone();
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
      c_form->addItem("(ExampleSubtype)");
      c_form->addItem("ExampleTopic01");
   #endif
}

void DKTopicOrSubtypePicker::clear() {
   #if !defined(QT_PLUGIN)
      this->setTopic(nullptr);
      this->setSubtype(0);
   #endif
}

#if !defined(QT_PLUGIN)
DKCustomFormFilter* DKTopicOrSubtypePicker::customFilter() const {
   return this->_rawModel()->get_custom_filter();
}
void DKTopicOrSubtypePicker::setCustomFilter(DKCustomFormFilter* v) {
   this->_rawModel()->set_custom_filter(v);
}
#endif