#include "./scene_editor.h"
#include <QDialog>
#include <QGridLayout>
//#include "widgets/DKQuestSceneEditor.h"
#include "dovah/form_stub.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void scene_editor::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      //auto* widget = new DKQuestSceneEditor(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      //widget->spawnRenderTest();

      //layout->addWidget(widget);
      
      dialog->show();
   }
}
