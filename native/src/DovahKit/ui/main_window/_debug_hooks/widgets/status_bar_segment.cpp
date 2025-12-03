#include "./status_bar_segment.h"
#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include "widgets/DKStatusBar.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void status_bar_segment::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      layout->setContentsMargins(0, 0, 0, 0);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);

      auto* sublayout = new QHBoxLayout();
      layout->addLayout(sublayout, 1);
      sublayout->setContentsMargins(6, 6, 6, 6);

      auto* status = new DKStatusBar(dialog);
      auto* widget = new QLabel(dialog);
      auto* button = new QPushButton("Flash");
      layout->addWidget(status);
      {
         auto* label = new QLabel("foo");
         status->addPermanentWidget(label);
      }
      status->addPermanentWidget(widget);
      sublayout->addWidget(button);
      {
         auto* label = new QLabel("foo");
         status->addPermanentWidget(label);
      }

      widget->setText("Test");
      {
         auto* button = new QPushButton("SB F");
         sublayout->addWidget(button);
         QObject::connect(button, &QPushButton::clicked, widget, [status, widget]() {
            status->flash(widget);
         });
      }

      dialog->show();
   }
}
