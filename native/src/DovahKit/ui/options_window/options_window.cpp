#include "options_window.h"
#include <QButtonGroup>
#include "editor/subsystems/options/core.h"
#include "editor/ini/main.h"

/*static*/ OptionsWindow* OptionsWindow::instance = nullptr;

OptionsWindow::OptionsWindow(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   if (OptionsWindow::instance != nullptr) {
      this->close();
      return;
   }
   //
   #pragma region Render Window
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::fCameraSpeedNormal,           this->ui.iniPref_fCameraSpeedNormal);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::fCameraSpeedMultBoost,        this->ui.iniPref_fCameraSpeedMultBoost);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::fCameraSpeedMultPrecision,    this->ui.iniPref_fCameraSpeedMultPrecision);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::fTurnSpeedDegreesPerSecondX, this->ui.iniPref_fTurnSpeedDegreesPerSecondX);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::fTurnSpeedDegreesPerSecondY, this->ui.iniPref_fTurnSpeedDegreesPerSecondY);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::bInvertLookX, this->ui.iniPref_bInvertLookX);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::bInvertLookY, this->ui.iniPref_bInvertLookY);
      //
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldedit::uLoadedGridSize, this->ui.iniPref_uLoadedGridSize);
      this->_mappings.radio_bool.emplace_back(
         &dovahkit::ini::main::worldedit::bLoadedGridSizeOverrideFromSkyrimINI,
         this->ui.iniPref_bLoadedGridSizeOverrideFromSkyrimINI_true,
         this->ui.iniPref_bLoadedGridSizeOverrideFromSkyrimINI_false
      );
   #pragma endregion

   // After all widgets are mapped to their INI settings, ensure that radio buttons are properly set up.
   for (auto& item : this->_mappings.radio_bool) {
      auto* group = new QButtonGroup(this);
      group->addButton(item.widget_false);
      group->addButton(item.widget_true);
   }

   // Enforce constraints.
   for (auto& item : this->_mappings.basic) {
      auto& setting = *item.setting;
      auto* widget  =  item.widget;
      if (auto* casted = dynamic_cast<QSpinBox*>(widget)) {
         int min = 0;
         std::optional<int> max;
         if (setting.is_of_type<int>()) {
            auto& info = setting.get_constraints<int>();
            min = info.min;
            if (info.max != std::numeric_limits<int>::max())
               max = info.max;
         } else if (setting.is_of_type<unsigned int>()) {
            auto& info = setting.get_constraints<unsigned int>();
            min = info.min;
            if (info.max != std::numeric_limits<unsigned int>::max())
               max = info.max;
         }
         casted->setMinimum(min);
         if (max.has_value())
            casted->setMaximum(max.value());
      } else if (auto* casted = dynamic_cast<QDoubleSpinBox*>(widget)) {
         auto& constraints = setting.get_constraints<double>();
         if (constraints.min != std::numeric_limits<double>::lowest())
            casted->setMinimum(constraints.min);
         if (constraints.max != std::numeric_limits<double>::max())
            casted->setMaximum(constraints.max);
      }
   }

   // After all widgets are mapped to their INI settings, pull the current values;
   this->revertChanges();

   #pragma region Special-cases: interdependent widgets
   QObject::connect(this->ui.iniPref_bLoadedGridSizeOverrideFromSkyrimINI_false, &QRadioButton::toggled, this, [this](bool checked) {
      this->ui.iniPref_uLoadedGridSize->setEnabled(checked);
   });
   #pragma endregion

   QObject::connect(this->ui.buttonSave, &QPushButton::clicked, this, [this]() {
      this->save();
      this->accept();
   });
   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, [this]() {
      this->reject();
   });

   auto& options_core = dovahkit::subsystems::options::core::get();
   QObject::connect(&options_core, &dovahkit::subsystems::options::core::mainIniSettingChanged, this,
      [this](cobb::ini::setting& setting, cobb::ini::value_variant prior, cobb::ini::value_variant after) {
         for (auto& item : this->_mappings.basic) {
            if (item.setting == &setting) {
               auto* widget = item.widget;
               if (auto* casted = dynamic_cast<QCheckBox*>(widget)) {
                  casted->setChecked(setting.get_current_value<bool>());
               } else if (auto* casted = dynamic_cast<QSpinBox*>(widget)) {
                  if (std::holds_alternative<int>(after)) {
                     casted->setValue(std::get<int>(after));
                  } else if (std::holds_alternative<unsigned int>(after)) {
                     casted->setValue(std::get<unsigned int>(after));
                  }
               } else if (auto* casted = dynamic_cast<QDoubleSpinBox*>(widget)) {
                  casted->setValue(setting.get_current_value<double>());
               }
               return;
            }
         }
         for (auto& item : this->_mappings.radio_bool) {
            if (item.setting == &setting) {
               auto* target = (setting.get_current_value<bool>()) ? item.widget_true : item.widget_false;
               target->setChecked(true);
               return;
            }
         }
      }
   );
}

/*static*/ OptionsWindow* OptionsWindow::open(QWidget* parent) {
   if (!instance) {
      instance = new OptionsWindow(parent);
      instance->show();
   } else {
      instance->raise();
      instance->activateWindow();
   }
   return instance;
}
OptionsWindow::~OptionsWindow() {
   if (this == OptionsWindow::instance)
      OptionsWindow::instance = nullptr;
}

void OptionsWindow::revertChanges() {
   for (auto& item : this->_mappings.basic) {
      auto& setting = *item.setting;
      auto* widget  =  item.widget;
      if (auto* casted = dynamic_cast<QCheckBox*>(widget)) {
         casted->setChecked(setting.get_current_value<bool>());
      } else if (auto* casted = dynamic_cast<QSpinBox*>(widget)) {
         if (setting.is_of_type<int>()) {
            casted->setValue(setting.get_current_value<int>());
         } else if (setting.is_of_type<unsigned int>()) {
            casted->setValue(setting.get_current_value<unsigned int>());
         }
      } else if (auto* casted = dynamic_cast<QDoubleSpinBox*>(widget)) {
         casted->setValue(setting.get_current_value<double>());
      }
   }
   for (auto& item : this->_mappings.radio_bool) {
      auto& setting = *item.setting;
      item.widget_true->setChecked(setting.get_current_value<bool>());
   }
}

void OptionsWindow::save() {
   const auto blocker = QSignalBlocker(this);

   for (auto& item : this->_mappings.basic) {
      auto* setting = item.setting;
      auto* widget  = item.widget;
      if (auto* casted = dynamic_cast<QCheckBox*>(widget)) {
         setting->set_current_value<bool>(casted->isChecked());
      } else if (auto* casted = dynamic_cast<QSpinBox*>(widget)) {
         auto value = casted->value();
         if (setting->is_of_type<int>()) {
            setting->set_current_value<int>(value);
         } else if (setting->is_of_type<unsigned int>()) {
            setting->set_current_value<unsigned int>(value);
         }
      } else if (auto* casted = dynamic_cast<QDoubleSpinBox*>(widget)) {
         setting->set_current_value<double>(casted->value());
      }
   }
   for (auto& item : this->_mappings.radio_bool) {
      auto* setting = item.setting;

      setting->set_current_value<bool>(item.widget_true->isChecked());
   }

   dovahkit::subsystems::options::core::get().save();
}