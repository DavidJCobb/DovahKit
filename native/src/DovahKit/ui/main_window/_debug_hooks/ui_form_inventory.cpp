#include "ui_form_inventory.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKFormInventory.h"
#include "dovah/form_stub.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_form_inventory::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKFormInventory(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      widget->setOrientation(Qt::Orientation::Vertical);
      //widget->setOrientation(Qt::Orientation::Horizontal);

      layout->addWidget(widget);
      
      dialog->show();
   }
}
