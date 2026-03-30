#include "./audio_simple.h"
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include "widgets/DKAudioWidgetSimple.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void audio_simple::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKAudioWidgetSimple(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      layout->addWidget(widget);

      widget->makeAudioCore();
      {
         auto* button = new QPushButton(("MaleBrute"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("sound/voice/skyrim.esm/malebrute/darkbrotherhood__00100565_1.fuz");
         });
      }
      {
         auto* button = new QPushButton(("FemaleCommander"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("sound/voice/skyrim.esm/femalecommander/dialoguetu_tutorialcombat_000dba32_1.fuz");
         });
      }
      
      dialog->show();
   }
}
