#include "options_window.h"
#include <QButtonGroup>
#include "editor/subsystems/options/core.h"
#include "editor/ini/main.h"

#include "editor/subsystems/worldedit/gizmo_colors/edit_gizmo_color_scheme_manager.h"
#include "./edit_gizmo_colors/edit_gizmo_color_editor.h"

#include "ui/models/worldedit/EditGizmoColorSchemeModel.h"

/*static*/ OptionsWindow* OptionsWindow::instance = nullptr;

#include <QIcon>
#include <QImage>
#include <QPainter>
namespace {
   QIcon _gizmo_colors_to_icon(const dovahkit::subsystems::worldedit::gizmo_color_scheme& scheme) {
      QImage image(16, 16, QImage::Format::Format_ARGB32);

      QPainter painter(&image);
      painter.setPen(Qt::PenStyle::NoPen);

      QPointF points[4];

      painter.setBrush(QBrush(QColor::fromRgb(scheme.axis_x.r, scheme.axis_x.g, scheme.axis_x.b)));
      points[0] = { 0,  0 };
      points[1] = { 8,  0 };
      points[2] = { 8,  8 };
      points[3] = { 0, 16 };
      painter.drawPolygon(points, 4);

      painter.setBrush(QBrush(QColor::fromRgb(scheme.axis_y.r, scheme.axis_y.g, scheme.axis_y.b)));
      points[0] = { 16,  0 };
      points[1] = {  8,  0 };
      points[2] = {  8,  8 };
      points[3] = { 16, 16 };
      painter.drawPolygon(points, 4);

      painter.setBrush(QBrush(QColor::fromRgb(scheme.axis_z.r, scheme.axis_z.g, scheme.axis_z.b)));
      points[0] = {  0, 16 };
      points[1] = {  8,  8 };
      points[2] = { 16, 16 };
      painter.drawPolygon(points, 3);

      painter.setPen(QColor::fromRgb(0, 0, 0));
      painter.setBrush(QBrush(QColor::fromRgb(scheme.highlight.r, scheme.highlight.g, scheme.highlight.b)));
      painter.drawEllipse(QPoint{ 8, 8 }, 3, 3);

      painter.setBrush(Qt::BrushStyle::NoBrush);
      painter.drawRect(0, 0, 15, 15);

      return QIcon(QPixmap::fromImage(image));
   }
}

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
      auto* widget = this->ui.editGizmoColorList;
      auto  id     = dovahkit::subsystems::worldedit::gizmo_color_scheme_manager::get().get_current_color_scheme_id();

      bool found = false;
      for (size_t i = 0; i < widget->count(); ++i) {
         auto hc = widget->itemData(i).toBool();
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

   auto* model = (EditGizmoColorSchemeModel*) this->ui.editGizmoColorList->model();
   if (model)
      model->force_replace_options(model->index(this->ui.editGizmoColorList->currentIndex(), 0, {}));

   dovahkit::subsystems::options::core::get().save();
}