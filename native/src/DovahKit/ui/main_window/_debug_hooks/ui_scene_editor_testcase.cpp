#include "./ui_scene_editor_testcase.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKQuestSceneEditor.h"
#include "dovah/form_stub.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_scene_editor_testcase::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKQuestSceneEditor(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      widget->spawnRenderTest();

      layout->addWidget(widget);
      
      dialog->show();
   }
}
