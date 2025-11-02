#include "idle_animations_model.h"
#include <QDialog>
#include <QGridLayout>
#include <QTreeView>
#include "dovah/form_stub.h"
#include "ui/form_windows/idle/IdleAnimationFormsModel.h"
#include "ui/form_windows/idle/IdleAnimationFormsModel_2.h"

namespace DovahKitDebug::features {
   /*static*/ void idle_animations_model::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new QTreeView(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      
      auto* model = new IdleAnimationFormsModel_2(widget);
      widget->setModel(model);
      widget->setHeaderHidden(true);

      layout->addWidget(widget);
      
      dialog->show();
   }
}
