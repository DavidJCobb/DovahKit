#include "ui_yes_no_unset_widget.h"
#include <QDebug>
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include "widgets/DKYesNoUnsetWidget.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_yes_no_unset_widget::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QGridLayout(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      int row = 0;
      {
         auto* label  = new QLabel("widget:");
         auto* widget = new DKYesNoUnsetWidget();
         label->setBuddy(widget);
         layout->addWidget(label,  row, 0);
         layout->addWidget(widget, row, 1);
         QObject::connect(widget, &DKYesNoUnsetWidget::stateChanged, [](Qt::CheckState v) {
            qDebug() << "Value: " << (int)v;
         });
      }
      ++row;

      dialog->show();
   }
}
