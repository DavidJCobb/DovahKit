#include "qt_ini_tests.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QPushButton>
#include "../../../helpers/qt/ini.h"

namespace {
   using File    = cobb::qt::ini::File;
   using Setting = cobb::qt::ini::Setting;
}

namespace {
   cobb::qt::ini::File& get_ini() {
      static File* instance = nullptr;
      if (instance)
         return *instance;
      instance = new File({
         .path       = "test_ini.ini",
         .categories = {
            {  .name     = "General",
               .settings = {
                  { "bTestSetting", true },
                  { "iTestSetting", 5 },
               }
            },
         }
      });
      instance->load();
      return *instance;
   }
}

namespace DovahKitDebug {
   extern void debug_qt_ini_helpers(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QGridLayout(dialog);
      dialog->setLayout(layout);
      //
      auto& ini = get_ini();
      int   row = 0;
      {
         auto* checkbox_set = new QCheckBox("bTestSetting");
         auto* checkbox_get = new QCheckBox("bTestSetting echo");
         auto* setting      = ini.setting("General", "bTestSetting");
         {
            auto value = setting->currentValue().toBool();
            checkbox_set->setChecked(value);
            checkbox_get->setChecked(value);
            checkbox_get->setEnabled(false);
         }
         QObject::connect(checkbox_set, &QCheckBox::toggled, dialog, [setting](bool checked) {
            setting->setPendingValue(checked);
         });
         QObject::connect(setting, &Setting::valueChanged, dialog, [checkbox_get](QVariant old, const QVariant& now) {
            checkbox_get->setChecked(now.toBool());
         }, Qt::ConnectionType::QueuedConnection);
         QObject::connect(setting, &Setting::pendingValueDiscarded, dialog, [setting, checkbox_set]() {
            checkbox_set->setChecked(setting->currentValue().toBool());
         }, Qt::ConnectionType::QueuedConnection);
         layout->addWidget(checkbox_get, row, 0);
         layout->addWidget(checkbox_set, row, 1);
         ++row;
      }
      //
      {
         auto* button = new QPushButton("Reload");
         QObject::connect(button, &QPushButton::clicked, dialog, []() {
            auto& ini = get_ini();
            ini.discardPendingChanges();
            ini.load();
         });
         layout->addWidget(button, row, 0);
         //++row;
      }
      {
         auto* button = new QPushButton("Save");
         QObject::connect(button, &QPushButton::clicked, dialog, []() {
            auto& ini = get_ini();
            ini.commitPendingChanges();
            ini.save(true);
         });
         layout->addWidget(button, row, 1);
         ++row;
      }
      //
      dialog->exec();
      delete dialog;
   }
}