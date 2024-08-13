#include "ui_float_slider.h"
#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include "widgets/DKFloatSlider.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_float_slider::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QGridLayout(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      int row = 0;
      {
         auto* label  = new QLabel("[0, 1] 3 decimals:");
         auto* widget = new DKFloatSlider(Qt::Orientation::Horizontal);
         label->setBuddy(widget);
         widget->setRange(0, 1);
         widget->setDecimals(3);
         widget->setTickInterval(0.10);
         widget->setTickPosition(DKFloatSlider::TickPosition::TicksBothSides);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKFloatSlider::valueChanged, [](float v) {
            qDebug() << "Value: " << v;
         });
      }
      ++row;
      {
         auto* label = new QLabel("[-1, 1] 3 decimals:");
         auto* widget = new DKFloatSlider(Qt::Orientation::Horizontal);
         label->setBuddy(widget);
         widget->setRange(-1, 1);
         widget->setDecimals(3);
         widget->setTickInterval(0.10);
         widget->setTickPosition(DKFloatSlider::TickPosition::TicksBothSides);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKFloatSlider::valueChanged, [](float v) {
            qDebug() << "Value: " << v;
         });
      }
      ++row;
      {
         auto* label = new QLabel("[0, 2] 1 decimals:");
         auto* widget = new DKFloatSlider(Qt::Orientation::Horizontal);
         label->setBuddy(widget);
         widget->setRange(0, 2);
         widget->setDecimals(1);
         widget->setTickInterval(0.10);
         widget->setTickPosition(DKFloatSlider::TickPosition::TicksBothSides);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKFloatSlider::valueChanged, [](float v) {
            qDebug() << "Value: " << v;
         });
      }
      ++row;
      {
         auto* label = new QLabel("[0, 2] 0 decimals:");
         auto* widget = new DKFloatSlider(Qt::Orientation::Horizontal);
         label->setBuddy(widget);
         widget->setRange(0, 2);
         widget->setDecimals(0);
         widget->setTickInterval(1);
         widget->setTickPosition(DKFloatSlider::TickPosition::TicksBothSides);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKFloatSlider::valueChanged, [](float v) {
            qDebug() << "Value: " << v;
         });
      }
      ++row;
      {
         auto* label = new QLabel("[-2, 2] 0 decimals:");
         auto* widget = new DKFloatSlider(Qt::Orientation::Horizontal);
         label->setBuddy(widget);
         widget->setRange(-2, 2);
         widget->setDecimals(0);
         widget->setTickInterval(1);
         widget->setTickPosition(DKFloatSlider::TickPosition::TicksBothSides);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKFloatSlider::valueChanged, [](float v) {
            qDebug() << "Value: " << v;
         });
      }
      ++row;

      dialog->show();
   }
}
