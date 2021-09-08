#include "qt_ini_tests.h"
#include <QApplication>
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include "../../../helpers/qt/ini.h"
#include "../../../helpers/qt/ini/binding.h"

namespace {
   using File    = cobb::qt::ini::File;
   using Setting = cobb::qt::ini::Setting;
}

namespace {
   cobb::qt::ini::File& get_ini() {
      static File* instance = nullptr;
      if (instance)
         return *instance;
      QMetaType::registerConverter(&DovahKitDebug::INIValueTestStruct::toString);
      QMetaType::registerConverter<QString, DovahKitDebug::INIValueTestStruct>(&DovahKitDebug::INIValueTestStruct::fromString);
      instance = new File({
         .path       = "test_ini.ini",
         .categories = {
            {  .name     = "General",
               .settings = {
                  { "bTestSetting", true },
                  { "iTestSetting", 5 },
                  { "xTestSetting", QVariant::fromValue<DovahKitDebug::INIValueTestStruct>({ 1, "Initial" }) },
               }
            },
         }
      });
      instance->load();
      return *instance;
   }
}

namespace DovahKitDebug {
   /*static*/ INIValueTestStruct INIValueTestStruct::fromString(const QString& text) {
      auto view = QStringRef(&text).trimmed();
      auto i    = view.indexOf(':');
      if (i <= 0)
         return INIValueTestStruct{};
      bool ok;
      int  value = view.left(i).toInt(&ok);
      if (!ok)
         return INIValueTestStruct{};
      //
      INIValueTestStruct out;
      out.number = value;
      out.text   = view.mid(i + 1).toString();
      return out;
   }
   QString INIValueTestStruct::toString() const noexcept {
      return QString("%1:%2").arg(this->number).arg(this->text);
   }


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
            checkbox_get->setChecked(value);
            checkbox_get->setEnabled(false);
         }
         QObject::connect(setting, &Setting::valueChanged, dialog, [checkbox_get](QVariant old, const QVariant& now) { // "echo" widget handler, to show the current value
            checkbox_get->setChecked(now.toBool());
         }, Qt::ConnectionType::QueuedConnection);
         //
         cobb::qt::ini::bindSettingControl(*setting, checkbox_set, &QCheckBox::setChecked, &QCheckBox::toggled); // setter widget handler, to work with the pending value
         //
         layout->addWidget(checkbox_get, row, 0);
         layout->addWidget(checkbox_set, row, 1);
         ++row;
      }
      {
         auto* spinbox_get = new QSpinBox;
         auto* spinbox_set = new QSpinBox;
         auto* textbox_get = new QLineEdit;
         auto* textbox_set = new QLineEdit;
         auto* setting     = ini.setting("General", "xTestSetting");
         textbox_get->setPlaceholderText("xTestSetting echo");
         textbox_set->setPlaceholderText("xTestSetting");
         {
            auto value = setting->currentValue().value<INIValueTestStruct>();
            spinbox_get->setEnabled(false);
            spinbox_get->setValue(value.number);
            spinbox_set->setValue(value.number);
            textbox_get->setEnabled(false);
            textbox_get->setText(value.text);
            textbox_set->setText(value.text);
         }
         QObject::connect(setting, &Setting::valueChanged, dialog, [spinbox_get, textbox_get](QVariant old, const QVariant& now) { // "echo" widget handler, to show the current value
            auto value = now.value<INIValueTestStruct>();
            spinbox_get->setValue(value.number);
            textbox_get->setText(value.text);
         }, Qt::ConnectionType::QueuedConnection);
         //
         QObject::connect(setting, &Setting::pendingValueDiscarded, dialog, [spinbox_set, textbox_set, setting]() {
            const auto blocker0 = QSignalBlocker(spinbox_set);
            const auto blocker1 = QSignalBlocker(textbox_set);
            //
            auto value = setting->currentValue().value<INIValueTestStruct>();
            spinbox_set->setValue(value.number);
            textbox_set->setText(value.text);
         }, Qt::ConnectionType::QueuedConnection);
         //
         QObject::connect(spinbox_set, QOverload<int>::of(&QSpinBox::valueChanged), setting, [setting](int v) {
            setting->modifyPendingValue<INIValueTestStruct>([v](INIValueTestStruct& s) {
               s.number = v;
            });
         });
         QObject::connect(textbox_set, &QLineEdit::textEdited, setting, [setting](const QString& v) {
            setting->modifyPendingValue<INIValueTestStruct>([&v](INIValueTestStruct& s) {
               s.text = v;
            });
         });
         //
         layout->addWidget(spinbox_get, row, 0);
         layout->addWidget(spinbox_set, row, 1);
         ++row;
         layout->addWidget(textbox_get, row, 0);
         layout->addWidget(textbox_set, row, 1);
         ++row;
      }
      //
      {
         auto* button = new QPushButton("Reload");
         QObject::connect(button, &QPushButton::clicked, dialog, []() {
            auto& ini = get_ini();
            ini.load();
            ini.discardPendingChanges();
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
      ini.discardPendingChanges(); // discard any pending changes when the dialog is closed
   }
}