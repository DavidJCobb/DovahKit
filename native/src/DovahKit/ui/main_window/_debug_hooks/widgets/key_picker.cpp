#include "./key_picker.h"
#include <QDialog>
#include <QGridLayout>
#include <QKeySequence>
#include "widgets/DKKeyPickerWidget.h"

namespace DovahKitDebug::features::widgets {
   /*static*/ void key_picker::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::Down, dialog);
      auto* widget = new DKKeyPickerWidget(dialog);
      auto* single = new DKKeyPickerWidget(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      //
      layout->addWidget(widget);
      layout->addWidget(single);
      single->setAllowKeyCombinations(false);
      QObject::connect(widget, &DKKeyPickerWidget::valueChanged, widget, [widget]() {
         auto list = widget->keys();
         if (list.empty()) {
            qDebug("No keys.");
            return;
         }
         qDebug("Keys:");
         for (auto& key : list) {
            qDebug(
               QString("Key %1 (%2): vk %3; scan %4; glyph %5")
                  .arg(key.code)
                  .arg(QKeySequence(key.code).toString())
                  .arg(key.native.vk)
                  .arg(key.native.scan)
                  .arg(key.glyph)
               .toUtf8()
            );
         }
         qDebug("All keys listed.");
      });
      QObject::connect(single, &DKKeyPickerWidget::valueChanged, single, [single]() {
         auto list = single->keys();
         if (list.empty()) {
            qDebug("No key.");
            return;
         }
         qDebug("Key:");
         for (auto& key : list) {
            qDebug(
               QString("Key %1 (%2): vk %3; scan %4; glyph %5")
                  .arg(key.code)
                  .arg(QKeySequence(key.code).toString())
                  .arg(key.native.vk)
                  .arg(key.native.scan)
                  .arg(key.glyph)
               .toUtf8()
            );
         }
         qDebug("Key listed.");
      });
      //
      dialog->show();
   }
}
