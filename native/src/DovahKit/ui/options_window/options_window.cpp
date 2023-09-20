#include "options_window.h"
#include <cassert>
#include <QButtonGroup>
#include "editor/subsystems/options/core.h"
#include "editor/ini/main.h"

#include "editor/subsystems/worldedit/gizmo_colors/edit_gizmo_color_scheme_manager.h"
#include "./edit_gizmo_colors/edit_gizmo_color_editor.h"
//
#include "ui/models/worldedit/EditGizmoColorSchemeModel.h"

#include "editor/subsystems/worldinput/worldinput_control_scheme_manager.h"
#include "./worldinput_scheme_editor.h"
//
#include "ui/models/worldinput/DKWorldinputDeviceSchemesModel.h"

/*static*/ OptionsWindow* OptionsWindow::instance = nullptr;

namespace worldinput {
   using namespace dovahkit::subsystems::worldinput;
}

OptionsWindow::OptionsWindow(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);
   if (OptionsWindow::instance != nullptr) {
      this->close();
      return;
   }

   #pragma region Set up page switcher
   {
      constexpr int WidgetColumn = 0;
      constexpr Qt::ItemDataRole WidgetRole = Qt::UserRole;

      auto* switcher = this->ui.navbar;
      auto* stacker  = this->ui.stackedWidget;

      QTreeWidgetItem* current = nullptr;

      switcher->clear();
      {
         auto* item = new QTreeWidgetItem;
         item->setText(WidgetColumn, tr("Loading and Saving", "navbar page name"));
         item->setData(WidgetColumn, WidgetRole, QVariant::fromValue<QWidget*>(this->ui.pageTESData));
         switcher->addTopLevelItem(item);

         current = item;
      }
      {
         auto* item = new QTreeWidgetItem;
         item->setText(WidgetColumn, tr("Render Window", "navbar page name"));
         item->setData(WidgetColumn, WidgetRole, QVariant::fromValue<QWidget*>(this->ui.pageRenderWindow));
         switcher->addTopLevelItem(item);
      }

      assert(current);
      switcher->setCurrentItem(current);
      stacker->setCurrentWidget(current->data(WidgetColumn, WidgetRole).value<QWidget*>());

      QObject::connect(switcher, &QTreeWidget::currentItemChanged, this, [this](QTreeWidgetItem* item) {
         this->ui.stackedWidget->setCurrentWidget(item->data(WidgetColumn, WidgetRole).value<QWidget*>());
      });
   }
   #pragma endregion
   
   //
   // First, the basic mappings. We'll loop over these lists and automatically connect INI settings to 
   // their widgets, for basic things like checkboxes, radio buttons, and spinboxes.
   //
   #pragma region Loading and Saving
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::saving::bApplyRefPersistenceAsNeeded, this->ui.iniPref_bApplyRefPersistenceAsNeeded);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::saving::bClearRefPersistenceWhenAble, this->ui.iniPref_bClearRefPersistenceWhenAble);
   #pragma endregion
   #pragma region Render Window
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldinput::fKeySequenceExpireTime,     this->ui.iniPref_fKeySequenceExpireTime);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldinput::fPressToHoldThreshold,      this->ui.iniPref_fPressToHoldThreshold);
      this->_mappings.basic.emplace_back(&dovahkit::ini::main::worldinput::fPressToLongPressThreshold, this->ui.iniPref_fPressToLongPressThreshold);
      //
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

   #pragma region Worldinput control schemes
   {
      {
         auto* widget = this->ui.worldinputKBList;
         widget->clear();

         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
            auto is_hardcoded = widget->currentData(EditGizmoColorSchemeModel::IsHardcodedRole).toBool();
            this->ui.worldinputKBDelete->setDisabled(is_hardcoded);
         });
         //
         auto* model = new DKWorldinputDeviceSchemesModel(widget);
         widget->setModel(model);
         model->reload(worldinput::input_device_type::keyboard_mouse);

         std::array<QPushButton*, 3> buttons = {
            this->ui.worldinputKBNew,
            this->ui.worldinputKBEdit,
            this->ui.worldinputKBDelete,
         };
         for (auto* button : buttons) {
            button->setProperty("picker", QVariant::fromValue(widget));
         }
         widget->setProperty("device-type", (int)worldinput::input_device_type::keyboard_mouse);
      }
      {
         auto* widget = this->ui.worldinputGPList;
         widget->clear();

         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
            auto is_hardcoded = widget->currentData(EditGizmoColorSchemeModel::IsHardcodedRole).toBool();
            this->ui.worldinputGPDelete->setDisabled(is_hardcoded);
         });
         //
         auto* model = new DKWorldinputDeviceSchemesModel(widget);
         widget->setModel(model);
         model->reload(worldinput::input_device_type::xinput);

         std::array<QPushButton*, 3> buttons = {
            this->ui.worldinputGPNew,
            this->ui.worldinputGPEdit,
            this->ui.worldinputGPDelete,
         };
         for (auto* button : buttons) {
            button->setProperty("picker", QVariant::fromValue(widget));
         }
         widget->setProperty("device-type", (int)worldinput::input_device_type::xinput);
      }

      // For these handlers, `QObject::sender` is whatever QObject sent the signal we're currently reacting to, if any.
      auto handler_new = [this]() {
         auto* sender = (QWidget*)this->sender();

         auto* picker = sender->property("picker").value<QComboBox*>();
         auto  device = (worldinput::input_device_type)picker->property("device-type").toInt();
         auto* model  = (DKWorldinputDeviceSchemesModel*)picker->model();

         auto qmi  = model->index(picker->currentIndex(), 0, {});
         auto data = model->dataFor(qmi);
         if (!data.has_value()) // should never happen
            return;

         auto* dialog = new WorldinputSchemeEditDialog(device, sender);
         dialog->initializeFrom(data.value());
         dialog->exec();
         if (dialog->result() == QDialog::DialogCode::Accepted) {
            auto qmi = model->insert(dialog->retrieve());
            if (qmi.isValid())
               picker->setCurrentIndex(qmi.row()); // select new scheme
         }
         dialog->deleteLater();
      };
      auto handler_edit = [this]() {
         auto* sender = (QWidget*)this->sender();

         auto* picker = sender->property("picker").value<QComboBox*>();
         auto  device = (worldinput::input_device_type)picker->property("device-type").toInt();
         auto* model  = (DKWorldinputDeviceSchemesModel*)picker->model();

         bool hardcoded = picker->currentData(DKWorldinputDeviceSchemesModel::IsHardcodedRole).toBool();

         auto qmi  = model->index(picker->currentIndex(), 0, {});
         auto data = model->dataFor(qmi);
         if (!data.has_value())
            return;

         auto* dialog = new WorldinputSchemeEditDialog(device, sender);
         dialog->initializeFrom(data.value());
         dialog->exec();
         if (dialog->result() == QDialog::DialogCode::Accepted) {
            if (hardcoded) {
               //
               // Don't let users actually save changes to hardcoded schemes, but for ergonomic 
               // reasons, let them view schemes, and take any changes they make and save those 
               // as a new scheme.
               //
               auto data_after = dialog->retrieve();
               if (data != data_after) { // only if any changes were actually made
                  auto qmi = model->insert(data_after);
                  if (qmi.isValid())
                     picker->setCurrentIndex(qmi.row()); // select new scheme
               }
            } else {
               dialog->overwrite(data.value());
               model->replaceDataFor(qmi, data.value());
            }
         }
         dialog->deleteLater();
      };
      auto handler_delete = [this]() {
         auto* sender = (QWidget*)this->sender();

         auto* picker = sender->property("picker").value<QComboBox*>();
         auto* model  = (DKWorldinputDeviceSchemesModel*)picker->model();
         if (picker->currentData(DKWorldinputDeviceSchemesModel::IsHardcodedRole).toBool()) {
            return;
         }
         model->removeRow(picker->currentIndex());
      };

      QObject::connect(this->ui.worldinputGPNew, &QPushButton::clicked, this, handler_new);
      QObject::connect(this->ui.worldinputKBNew, &QPushButton::clicked, this, handler_new);
      QObject::connect(this->ui.worldinputGPEdit, &QPushButton::clicked, this, handler_edit);
      QObject::connect(this->ui.worldinputKBEdit, &QPushButton::clicked, this, handler_edit);
      QObject::connect(this->ui.worldinputGPDelete, &QPushButton::clicked, this, handler_delete);
      QObject::connect(this->ui.worldinputKBDelete, &QPushButton::clicked, this, handler_delete);
   }
   #pragma endregion

   #pragma region Edit gizmo color scheme
   {
      {
         auto& manager = dovahkit::subsystems::worldedit::gizmo_color_scheme_manager::get();

         auto* widget = this->ui.editGizmoColorList;
         widget->clear();

         auto* model = new EditGizmoColorSchemeModel(widget);
         widget->setModel(model);
         model->reset_from_options();

         QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, widget](int index) {
            auto is_hardcoded = widget->currentData(EditGizmoColorSchemeModel::IsHardcodedRole).toBool();
            this->ui.editGizmoColorDelete->setDisabled(is_hardcoded);
            this->ui.editGizmoColorEdit->setDisabled(is_hardcoded);
         });
      }

      QObject::connect(this->ui.editGizmoColorNew, &QPushButton::clicked, this, [this]() {
         auto* picker = this->ui.editGizmoColorList;
         auto* model  = (EditGizmoColorSchemeModel*)picker->model();

         auto qmi  = model->index(picker->currentIndex(), 0, {});
         auto data = model->dataFor(qmi);
         if (!data.has_value()) // should never happen
            return;

         auto* dialog = new EditGizmoColorSchemeEditDialog(this);
         dialog->initializeFrom(data.value());
         dialog->exec();
         if (dialog->result() == QDialog::DialogCode::Accepted) {
            EditGizmoColorSchemeModel::data_type created;
            dialog->overwrite(created);

            auto qmi = model->insert(created);
            if (qmi.isValid())
               picker->setCurrentIndex(qmi.row()); // select new scheme
         }
         dialog->deleteLater();
      });
      QObject::connect(this->ui.editGizmoColorEdit, &QPushButton::clicked, this, [this]() {
         auto* picker = this->ui.editGizmoColorList;
         auto* model  = (EditGizmoColorSchemeModel*)picker->model();
         if (picker->currentData(EditGizmoColorSchemeModel::IsHardcodedRole).toBool()) {
            return;
         }
         auto qmi  = model->index(picker->currentIndex(), 0, {});
         auto data = model->dataFor(qmi);
         if (!data.has_value())
            return;

         auto* dialog = new EditGizmoColorSchemeEditDialog(this);
         dialog->initializeFrom(data.value());
         dialog->exec();
         if (dialog->result() == QDialog::DialogCode::Accepted) {
            dialog->overwrite(data.value());
            model->replaceDataFor(qmi, data.value());
         }
         dialog->deleteLater();
      });
      QObject::connect(this->ui.editGizmoColorDelete, &QPushButton::clicked, this, [this]() {
         auto* picker = this->ui.editGizmoColorList;
         auto* model = (EditGizmoColorSchemeModel*)picker->model();
         if (picker->currentData(EditGizmoColorSchemeModel::IsHardcodedRole).toBool()) {
            return;
         }
         model->removeRow(picker->currentIndex());
      });
   }
   #pragma endregion

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
   QObject::connect(this->ui.buttonApply, &QPushButton::clicked, this, [this]() {
      this->save();
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
      auto  value   = setting.get_current_value<bool>();
      item.widget_true->setChecked(value);
      if (!value)
         //
         // Setting the "true" radio button to unchecked does not inherently 
         // set the "false" radio button to checked. Gotta do that manually.
         //
         item.widget_false->setChecked(true);
   }

   {
      auto& mgr = worldinput::control_scheme_manager::get_or_create();
      {
         auto& current = mgr.get_current_scheme(worldinput::input_device_type::keyboard_mouse);
         auto* widget  = this->ui.worldinputKBList;
         auto* model   = (DKWorldinputDeviceSchemesModel*)widget->model();

         auto i = model->rowFor(current);
         if (i >= 0)
            widget->setCurrentIndex(i);
      }
      {
         auto& current = mgr.get_current_scheme(worldinput::input_device_type::xinput);
         auto* widget  = this->ui.worldinputGPList;
         auto* model   = (DKWorldinputDeviceSchemesModel*)widget->model();

         auto i = model->rowFor(current);
         if (i >= 0)
            widget->setCurrentIndex(i);
      }
   }

   {
      auto* widget = this->ui.editGizmoColorList;
      auto  id     = dovahkit::subsystems::worldedit::gizmo_color_scheme_manager::get().get_current_color_scheme_id();

      bool found = false;
      for (size_t i = 0; i < widget->count(); ++i) {
         auto hc = widget->itemData(i, EditGizmoColorSchemeModel::IsHardcodedRole).toBool();
         if (hc != id.is_hardcoded)
            continue;
         if (widget->itemText(i).toUtf8().toStdString() != id.name)
            continue;
         found = true;
         widget->setCurrentIndex(i);
         break;
      }
      if (!found) {
         // TODO: reload scheme list
         // TODO: try selecting current scheme again; if absent, select "standard"
      }
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

   {
      auto& mgr = worldinput::control_scheme_manager::get_or_create();
      {
         auto* widget = this->ui.worldinputKBList;
         auto* model  = (DKWorldinputDeviceSchemesModel*)widget->model();

         auto  i    = widget->currentIndex();
         auto* data = model->savedSchemeAtRow(i);
         if (data) {
            mgr.set_current_scheme(data);
         }
      }
      {
         auto* widget = this->ui.worldinputGPList;
         auto* model  = (DKWorldinputDeviceSchemesModel*)widget->model();

         auto  i    = widget->currentIndex();
         auto* data = model->savedSchemeAtRow(i);
         if (data) {
            mgr.set_current_scheme(data);
         }
      }
   }

   auto* model = (EditGizmoColorSchemeModel*) this->ui.editGizmoColorList->model();
   if (model)
      model->force_replace_options(model->index(this->ui.editGizmoColorList->currentIndex(), 0, {}));

   dovahkit::subsystems::options::core::get().save();
}