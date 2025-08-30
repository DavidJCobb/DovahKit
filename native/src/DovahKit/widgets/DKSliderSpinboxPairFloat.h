#pragma once
#include <QDoubleSpinBox>
#include "./DKFloatSlider.h"

class DKSliderSpinboxPairFloat : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(float value      READ value      WRITE setValue      DESIGNABLE true USER true);
   Q_PROPERTY(float minimum    READ minimum    WRITE setMinimum    DESIGNABLE true);
   Q_PROPERTY(float maximum    READ maximum    WRITE setMinimum    DESIGNABLE true);
   Q_PROPERTY(float singleStep READ singleStep WRITE setSingleStep DESIGNABLE true);
   Q_PROPERTY(int   decimals READ decimals WRITE setDecimals DESIGNABLE true);
   Q_PROPERTY(QString prefix READ prefix WRITE setPrefix DESIGNABLE true);
   Q_PROPERTY(QString suffix READ suffix WRITE setSuffix DESIGNABLE true);
   public:
      using value_type = float;
      using StepType   = QAbstractSpinBox::StepType;

   public:
      DKSliderSpinboxPairFloat(QWidget* parent = nullptr);

      #pragma region Properties
         int decimals() const;
         void setDecimals(int);

         value_type minimum() const;
         value_type maximum() const;

         void setMinimum(value_type);
         void setMaximum(value_type);
         void setRange(value_type, value_type);

         value_type singleStep() const;
         void setSingleStep(value_type);

         StepType stepType() const;
         void setStepType(StepType);

         value_type value() const;
         void setValue(value_type);

         QString prefix() const;
         void setPrefix(QString);

         QString suffix() const;
         void setSuffix(QString);
      #pragma endregion

   signals:
      void valueChanged(value_type);

   protected:
      struct {
         DKFloatSlider*  slider  = nullptr;
         QDoubleSpinBox* spinbox = nullptr;
      } _subwidgets;
};