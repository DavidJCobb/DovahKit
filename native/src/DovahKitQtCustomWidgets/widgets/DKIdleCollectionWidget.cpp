#include "./DKIdleCollectionWidget.h"
#include <limits>
#include <QBoxLayout>
#include <QLabel>
#if !defined(QT_PLUGIN)
   #include "dovah/forms/components/idle_collection.h"
#endif

DKIdleCollectionWidget::DKIdleCollectionWidget(QWidget* parent) : QWidget(parent) {
   auto& ui = this->_subwidgets;

   //
   // Create widgets:
   //
   ui.idles = new DKFormListPane(this);
   ui.idles->setShowFormTypes(false);
   #if !defined(QT_PLUGIN)
      ui.idles->setAllowedFormTypes({ dovah::form_type::idle });
   #endif

   ui.order = new QComboBox(this);
   ui.order->addItem(tr("Random"), false);
   ui.order->addItem(tr("Run in Sequence"), true);

   ui.do_once = new QCheckBox(tr("Do Once"), this);

   ui.idle_timer = new QDoubleSpinBox(this);
   ui.idle_timer->setRange(0, std::numeric_limits<float>::max());
   ui.idle_timer->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
   //
   // Layout:
   //
   {
      auto* layout = new QVBoxLayout(this);
      layout->addWidget(ui.idles, 1);
      layout->setContentsMargins(0, 0, 0, 0);
      this->setLayout(layout);

      auto* sublayout = new QHBoxLayout();
      sublayout->setContentsMargins(0, 0, 0, 0);
      layout->addLayout(sublayout);
      {
         auto* label = new QLabel(tr("Order:"), this);
         label->setBuddy(ui.order);
         sublayout->addWidget(label);
      }
      sublayout->addWidget(ui.order);
      sublayout->addWidget(ui.do_once);
      sublayout->addSpacerItem(new QSpacerItem(8, 0));
      {
         auto* label = new QLabel(tr("Idle Timer:"), this);
         label->setBuddy(ui.idle_timer);
         sublayout->addWidget(label);
      }
      sublayout->addWidget(ui.idle_timer);
   }
   //
   // Focus and tab order:
   //
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(ui.idles);
   QWidget::setTabOrder(ui.idles, ui.order);
   QWidget::setTabOrder(ui.order, ui.do_once);
   QWidget::setTabOrder(ui.do_once, ui.idle_timer);

   #pragma region "What's this?" text
      //: "What's this?" text for the "Do Once" flag.
      ui.do_once->setWhatsThis(tr("If this is checked, then idles in this list will each only be played once.", "what's this?"));

      //: "What's this?" text for the Idle TImer.
      ui.idle_timer->setWhatsThis(tr("If this list plays animations in a random order, then this is the duration of each animation. If this list plays all of its animations in sequence, then this is the delay between each time the sequence loops.", "what's this?"));
   #pragma endregion
}

#if !defined(QT_PLUGIN)
void DKIdleCollectionWidget::importData(const dovah::loaded_forms::components::idle_collection& component) {
   using component_type = dovah::loaded_forms::components::idle_collection;

   auto& ui = this->_subwidgets;
   ui.idles->pullStubs(component.idles);
   ui.order->setCurrentIndex((component.flags & component_type::flag::run_in_sequence) ? 1 : 0);
   ui.do_once->setChecked(component.flags & component_type::flag::do_once);
   ui.idle_timer->setValue(component.timer);
}
void DKIdleCollectionWidget::exportData(dovah::loaded_forms::components::idle_collection& component, dovah::loaded_forms::Form& component_containing_form) {
   using component_type = dovah::loaded_forms::components::idle_collection;

   auto& ui = this->_subwidgets;
   ui.idles->commitStubs(component.idles, component_containing_form);
   if (ui.order->currentData().toBool()) {
      component.flags |= component_type::flag::run_in_sequence;
   } else {
      component.flags &= ~component_type::flag::run_in_sequence;
   }
   if (ui.do_once->isChecked()) {
      component.flags |= component_type::flag::do_once;
   } else {
      component.flags &= ~component_type::flag::do_once;
   }
   component.timer = ui.idle_timer->value();
}
#endif
