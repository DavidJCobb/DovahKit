#include "./audio_simple.h"
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include "widgets/DKAudioWidget.h"
#include "editor/core.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void audio_simple::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKAudioWidget(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      layout->addWidget(widget);

      {
         auto* button = new QPushButton(("FUZ: MaleBrute"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("sound/voice/skyrim.esm/malebrute/darkbrotherhood__00100565_1.fuz");
         });
      }
      {
         auto* button = new QPushButton(("FUZ: FemaleCommander"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("sound/voice/skyrim.esm/femalecommander/dialoguetu_tutorialcombat_000dba32_1.fuz");
         });
      }
      {
         auto* button = new QPushButton(("WAV: ITMNirnrootLPSD"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("Sound/FX/ITM/Ingredient/ITM_NirnRoot_LP.wav");
         });
      }
      {
         auto* button = new QPushButton(("WAV: NPCDogBarkSD (02)"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            widget->setPath("Sound/FX/NPC/Dog/Bark/NPC_Dog_Bark_02.wav");
         });
      }
      {
         auto* button = new QPushButton(("WAV: MUSCombat01"), dialog);
         layout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [widget]() {
            auto& editor = DovahKitCore::get();
            if (editor.get_current_game() == dovah::game::skyrim_special) {
               widget->setPath("Music/Combat/MUS_Combat_01.wav");
            } else {
               widget->setPath("Music/Combat/MUS_Combat_01.xwm");
            }
         });
      }
      
      dialog->show();
   }
}
