#include "ui_texture_asset_pane.h"
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKGameFilePicker.h"
#include "widgets/DKTextureAssetPane.h"
#include "../../../dovah/form_stub.h"
#include "../../../editor/form_stub_meta_type.h"
#include "widgets/DKFormPicker.h"

namespace DovahKitDebug::features {
   /*static*/ void ui_texture_asset_pane::execute(QWidget* from) {
      auto* dialog = new QDialog(from);
      auto* layout = new QGridLayout(dialog);
      auto* widget = new DKTextureAssetPane(dialog);
      auto* picker = new DKGameFilePicker(dialog);
      auto* f_list = new DKFormPicker(dialog);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      //
      f_list->setAllowNone(true);
      f_list->setAllowedFormType(dovah::form_type::texture_set);
      //
      layout->addWidget(widget);
      layout->addWidget(picker, 1, 0);
      layout->addWidget(f_list, 2, 0);
      layout->setRowStretch(0, 1);
      layout->setRowStretch(1, 0);
      layout->setRowStretch(2, 0);
      picker->setPresetConfiguration(DKGameFilePicker::PresetConfiguration::Textures);
      QObject::connect(picker, &DKGameFilePicker::valueChanged, widget, [widget, f_list](const ui::types::game_file_path& path) {
         const auto blocker = QSignalBlocker(f_list);
         widget->setAsset(path.to_string());
         f_list->setFormStub(nullptr);
      });
      QObject::connect(f_list, &DKFormPicker::formChanged, widget, [widget, picker, f_list](dovah::form_stub* stub) {
         const auto blocker = QSignalBlocker(picker);
         picker->clear();
         widget->setAsset(stub);
      });
      //
      widget->setThrottleEnabled(true);
      widget->setThrottleTime(75);
      dialog->show();
   }
}
