#pragma once
#include <QSlider>
#include <QSpinBox>

class DKSliderSpinboxPairInt : public QWidget {
   Q_OBJECT;
   Q_PROPERTY(int value      READ value      WRITE setValue      DESIGNABLE true USER true);
   Q_PROPERTY(int minimum    READ minimum    WRITE setMinimum    DESIGNABLE true);
   Q_PROPERTY(int maximum    READ maximum    WRITE setMinimum    DESIGNABLE true);
   Q_PROPERTY(int singleStep READ singleStep WRITE setSingleStep DESIGNABLE true);
   Q_PROPERTY(int pageStep   READ pageStep   WRITE setPageStep   DESIGNABLE true);
   Q_PROPERTY(int displayIntegerBase READ displayIntegerBase WRITE setDisplayIntegerBase DESIGNABLE true);
   Q_PROPERTY(QString prefix READ prefix WRITE setPrefix DESIGNABLE true);
   Q_PROPERTY(QString suffix READ suffix WRITE setSuffix DESIGNABLE true);
   public:
      using value_type = int;
      using StepType   = QAbstractSpinBox::StepType;

   public:
      DKSliderSpinboxPairInt(QWidget* parent = nullptr);

      #pragma region Properties
         int displayIntegerBase() const;
         void setDisplayIntegerBase(int);

         value_type minimum() const;
         value_type maximum() const;

         void setMinimum(value_type);
         void setMaximum(value_type);
         void setRange(value_type, value_type);

         value_type singleStep() const;
         void setSingleStep(value_type);
         //
         value_type pageStep() const;
         void setPageStep(value_type);

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
         QSlider*  slider  = nullptr;
         QSpinBox* spinbox = nullptr;
      } _subwidgets;
};