#include "./DKSliderSpinboxPairInt.h"
#include <array>
#include <QHBoxLayout>

DKSliderSpinboxPairInt::DKSliderSpinboxPairInt(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);

   auto* slider  = this->_subwidgets.slider  = new QSlider(this);
   auto* spinbox = this->_subwidgets.spinbox = new QSpinBox(this);

   slider->setOrientation(Qt::Orientation::Horizontal);

   layout->addWidget(slider, 1);
   layout->addWidget(spinbox);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(slider);
   QWidget::setTabOrder(slider, spinbox);

   QObject::connect(slider, qOverload<value_type>(&QSlider::valueChanged), this, [this](value_type v) {
      auto*      paired  = this->_subwidgets.spinbox;
      const auto blocker = QSignalBlocker(paired);
      paired->setValue(v);
      emit this->valueChanged(v);
   });
   QObject::connect(spinbox, qOverload<value_type>(&QSpinBox::valueChanged), this, [this](value_type v) {
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
      type DKSliderSpinboxPairInt:: name () const { return this->_subwidgets.spinbox-> name (); }
   #define SPINBOX_ONLY_SETTER(name, type) \
      void DKSliderSpinboxPairInt:: name (type v) { this->_subwidgets.spinbox-> name (v); }
   #define PAIRED_SETTER_INFLUENCING_VALUE(name, type) \
      void DKSliderSpinboxPairInt:: name (type v) { \
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

   SPINBOX_BASED_GETTER(displayIntegerBase, int);
   SPINBOX_ONLY_SETTER(setDisplayIntegerBase, int);

   SPINBOX_BASED_GETTER(minimum, DKSliderSpinboxPairInt::value_type);
   SPINBOX_BASED_GETTER(maximum, DKSliderSpinboxPairInt::value_type);

   PAIRED_SETTER_INFLUENCING_VALUE(setMinimum, value_type);
   PAIRED_SETTER_INFLUENCING_VALUE(setMaximum, value_type);
   void DKSliderSpinboxPairInt::setRange(value_type min, value_type max) {
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

   SPINBOX_BASED_GETTER(singleStep, DKSliderSpinboxPairInt::value_type);
   PAIRED_SETTER_INFLUENCING_VALUE(setSingleStep, value_type);
   //
   DKSliderSpinboxPairInt::value_type DKSliderSpinboxPairInt::pageStep() const {
      return this->_subwidgets.slider->pageStep();
   }
   void DKSliderSpinboxPairInt::setPageStep(value_type v) {
      this->_subwidgets.slider->setPageStep(v);
   }

   SPINBOX_BASED_GETTER(stepType, DKSliderSpinboxPairInt::StepType);
   SPINBOX_ONLY_SETTER(setStepType, DKSliderSpinboxPairInt::StepType);

   SPINBOX_BASED_GETTER(value, DKSliderSpinboxPairInt::value_type);
   void DKSliderSpinboxPairInt::setValue(value_type v) {
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