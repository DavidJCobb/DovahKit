#include "./attack_data.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKAttackDataWidget.h"
#include "dovah/form_stub.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void attack_data::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKAttackDataWidget(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      layout->addWidget(widget);
      
      dialog->show();
   }
}
