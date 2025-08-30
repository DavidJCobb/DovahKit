#include "./DKSliderSpinboxPairFloat.h"
#include <array>
#include <QHBoxLayout>

DKSliderSpinboxPairFloat::DKSliderSpinboxPairFloat(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);

   auto* slider  = this->_subwidgets.slider  = new DKFloatSlider(this);
   auto* spinbox = this->_subwidgets.spinbox = new QDoubleSpinBox(this);

   slider->setOrientation(Qt::Orientation::Horizontal);
   slider->setDecimals(spinbox->decimals());
   slider->setValue(spinbox->value());

   layout->addWidget(slider, 1);
   layout->addWidget(spinbox);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(slider);
   QWidget::setTabOrder(slider, spinbox);

   QObject::connect(slider, qOverload<value_type>(&DKFloatSlider::valueChanged), this, [this](value_type v) {
      auto*      paired  = this->_subwidgets.spinbox;
      const auto blocker = QSignalBlocker(paired);
      paired->setValue(v);
      emit this->valueChanged(v);
   });
   QObject::connect(spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
      auto*      paired  = this->_subwidgets.slider;
      const auto blocker = QSignalBlocker(paired);
      paired->setValue(v);
      emit this->valueChanged(v);
   });
}

#pragma region Properties
   #pragma push_macro("SPINBOX_BASED_GETTER")
   #pragma push_macro("SPINBOX_ONLY_SETTER")
   #pragma push_macro("PAIRED_SETTER_INFLUENCING_VALUE")
   //
   #define SPINBOX_BASED_GETTER(name, type) \
      type DKSliderSpinboxPairFloat:: name () const { return this->_subwidgets.spinbox-> name (); }
   #define SPINBOX_ONLY_SETTER(name, type) \
      void DKSliderSpinboxPairFloat:: name (type v) { this->_subwidgets.spinbox-> name (v); }
   #define PAIRED_SETTER_INFLUENCING_VALUE(name, type) \
      void DKSliderSpinboxPairFloat:: name (type v) { \
         const auto prior = this->value(); \
         { \
            const auto blockers = std::array{ \
               QSignalBlocker(this->_subwidgets.spinbox), \
               QSignalBlocker(this->_subwidgets.slider) \
            }; \
            this->_subwidgets.spinbox-> name (v); \
            this->_subwidgets.slider-> name (v); \
         } \
         const auto after = this->value(); \
         if (after != prior) \
            emit valueChanged(after); \
      }

   SPINBOX_BASED_GETTER(decimals, int);
   PAIRED_SETTER_INFLUENCING_VALUE(setDecimals, int);

   SPINBOX_BASED_GETTER(minimum, DKSliderSpinboxPairFloat::value_type);
   SPINBOX_BASED_GETTER(maximum, DKSliderSpinboxPairFloat::value_type);

   PAIRED_SETTER_INFLUENCING_VALUE(setMinimum, value_type);
   PAIRED_SETTER_INFLUENCING_VALUE(setMaximum, value_type);
   void DKSliderSpinboxPairFloat::setRange(value_type min, value_type max) {
      const auto prior = this->value();
      {
         const auto blockers = std::array{
            QSignalBlocker(this->_subwidgets.spinbox),
            QSignalBlocker(this->_subwidgets.slider)
         };
         this->_subwidgets.spinbox->setRange(min, max);
         this->_subwidgets.slider->setRange(min, max);
      }
      const auto after = this->value();
      if (after != prior)
         emit valueChanged(after);
   }

   SPINBOX_BASED_GETTER(singleStep, DKSliderSpinboxPairFloat::value_type);
   SPINBOX_ONLY_SETTER(setSingleStep, value_type);

   SPINBOX_BASED_GETTER(stepType, DKSliderSpinboxPairFloat::StepType);
   SPINBOX_ONLY_SETTER(setStepType, DKSliderSpinboxPairFloat::StepType);

   SPINBOX_BASED_GETTER(value, DKSliderSpinboxPairFloat::value_type);
   void DKSliderSpinboxPairFloat::setValue(value_type v) {
      const auto prior = this->value();
      if (prior == v)
         return;
      {
         const auto blockers = std::array{
            QSignalBlocker(this->_subwidgets.spinbox),
            QSignalBlocker(this->_subwidgets.slider)
         };
         this->_subwidgets.spinbox->setValue(v);
         this->_subwidgets.slider->setValue(v);
      }
      const auto after = this->value();
      if (prior != after)
         emit valueChanged(this->value());
   }

   SPINBOX_BASED_GETTER(prefix, QString);
   SPINBOX_ONLY_SETTER(setPrefix, QString);

   SPINBOX_BASED_GETTER(suffix, QString);
   SPINBOX_ONLY_SETTER(setSuffix, QString);

   #undef SPINBOX_BASED_GETTER
   #undef SPINBOX_ONLY_SETTER
   #undef PAIRED_SETTER_INFLUENCING_VALUE
   #pragma pop_macro("SPINBOX_BASED_GETTER")
   #pragma pop_macro("SPINBOX_ONLY_SETTER")
   #pragma pop_macro("PAIRED_SETTER_INFLUENCING_VALUE")
#pragma endregion