#include "ui_texture_asset_pane.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKGameFilePicker.h"
#include "widgets/DKTextureAssetPane.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_texture_asset_pane::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QGridLayout(dialog);
      auto* widget = new DKTextureAssetPane;
      auto* picker = new DKGameFilePicker(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      //
      layout->addWidget(widget);
      layout->addWidget(picker, 1, 0);
      layout->setRowStretch(0, 1);
      layout->setRowStretch(1, 0);
      picker->setStandardConfiguration(DKGameFilePicker::StandardConfiguration::Textures);
      QObject::connect(picker, &DKGameFilePicker::pathChanged, widget, [widget](const QString& path) {
         widget->setAsset(path);
      });
      //
      dialog->show();
   }
}
