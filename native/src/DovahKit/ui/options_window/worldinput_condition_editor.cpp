#include "./worldinput_condition_editor.h"
#include <array>
#include "helpers/qt/basic_bindings.h"
#include "helpers/qt/combobox.h"

#include "editor/subsystems/worldinput/control_scheme/condition.h"
#include "editor/subsystems/worldinput/control_scheme.h"
#include "editor/subsystems/worldinput/condition_set.h"

namespace {
   namespace worldinput {
      using namespace ::dovahkit::subsystems::worldinput;
   }
}

WorldinputConditionEditDialog::WorldinputConditionEditDialog(QWidget* parent) : QDialog(parent) {
   this->ui.setupUi(this);

   this->ui.name->setMaxLength(node_type::max_name_length);

   QObject::connect(this->ui.buttonCancel, &QPushButton::clicked, this, &QDialog::reject);
   QObject::connect(this->ui.buttonSave,   &QPushButton::clicked, this, &QDialog::accept);

   #pragma region Editor modes
   {
      using editor_mode = dovahkit::subsystems::worldedit::editor_mode;

      std::array<QCheckBox*, 3> all_checkboxes = {
         this->ui.editModeNavmesh,
         this->ui.editModeObject,
         this->ui.editModeTerrain,
      };

      auto handler = [this, all_checkboxes]<editor_mode Mode>(bool checked) {
         bool all_checked = false;
         if (checked) {
            all_checked = true;
            for (const auto* widget : all_checkboxes) {
               if (!widget->isChecked()) {
                  all_checked = false;
                  break;
               }
            }
         }
         if (all_checked) {
            this->_data.editor_modes = {};
            return;
         }
         if (!this->_data.editor_modes.has_value()) {
            this->_data.editor_modes.emplace();
         }
         auto& v = this->_data.editor_modes.value();
         if (checked) {
            v.set(Mode);
         } else {
            v.reset(Mode);
         }
      };

      QObject::connect(this->ui.editModeNavmesh, &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<editor_mode::navmesh>(checked); });
      QObject::connect(this->ui.editModeObject,  &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<editor_mode::objects>(checked); });
      QObject::connect(this->ui.editModeTerrain, &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<editor_mode::terrain>(checked); });
   }
   #pragma endregion
   #pragma region Gizmo modes
   {
      using gizmo_mode = dovahkit::subsystems::worldedit::gizmo_mode;

      std::array<QCheckBox*, 4> all_checkboxes = {
         this->ui.gizmoModeNone,
         this->ui.gizmoModeTranslate,
         this->ui.gizmoModeRotate,
         this->ui.gizmoModeScale,
      };

      auto handler = [this, all_checkboxes]<gizmo_mode Mode>(bool checked) {
         bool all_checked = false;
         if (checked) {
            all_checked = true;
            for (const auto* widget : all_checkboxes) {
               if (!widget->isChecked()) {
                  all_checked = false;
                  break;
               }
            }
         }
         if (all_checked) {
            this->_data.gizmo_modes = {};
            return;
         }
         if (!this->_data.gizmo_modes.has_value()) {
            this->_data.gizmo_modes.emplace();
         }
         auto& v = this->_data.gizmo_modes.value();
         if (checked) {
            v.set(Mode);
         } else {
            v.reset(Mode);
         }
      };

      QObject::connect(this->ui.gizmoModeNone,      &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<gizmo_mode::none>(checked); });
      QObject::connect(this->ui.gizmoModeTranslate, &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<gizmo_mode::translate>(checked); });
      QObject::connect(this->ui.gizmoModeRotate,    &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<gizmo_mode::rotate>(checked); });
      QObject::connect(this->ui.gizmoModeScale,     &QCheckBox::toggled, this, [handler](bool checked) { handler.operator()<gizmo_mode::scale>(checked); });
   }
   #pragma endregion
   #pragma region Selection count
   {
      this->ui.numSelectionsComparison->clear();
      this->ui.numSelectionsComparison->addItem(QString("="), (int)worldinput::comparison_operator::equal);
      this->ui.numSelectionsComparison->addItem(QString::fromUtf8((const char*)u8"≠"), (int)worldinput::comparison_operator::not_equal);
      this->ui.numSelectionsComparison->addItem(QString(">"), (int)worldinput::comparison_operator::greater);
      this->ui.numSelectionsComparison->addItem(QString::fromUtf8((const char*)u8"≥"), (int)worldinput::comparison_operator::greater_or_equal);
      this->ui.numSelectionsComparison->addItem(QString("<"), (int)worldinput::comparison_operator::less);
      this->ui.numSelectionsComparison->addItem(QString::fromUtf8((const char*)u8"≤"), (int)worldinput::comparison_operator::less_or_equal);

      QObject::connect(this->ui.numSelectionsEnable, &QCheckBox::toggled, this, [this](bool checked) {
         this->_update_selection_count_constraint();
         this->ui.numSelectionsComparison->setEnabled(checked);
         this->ui.numSelectionsComparand->setEnabled(checked);
      });
      QObject::connect(this->ui.numSelectionsComparison, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() { this->_update_selection_count_constraint(); });
      QObject::connect(this->ui.numSelectionsComparand,  QOverload<int>::of(&QSpinBox::valueChanged),         this, [this]() { this->_update_selection_count_constraint(); });
   }
   #pragma endregion
}

void WorldinputConditionEditDialog::_update_selection_count_constraint() {
   auto& constraint_opt = this->_data.selection_count;

   bool present = this->ui.numSelectionsEnable->isChecked();
   if (!present) {
      constraint_opt = {};
      return;
   }
   if (!constraint_opt.has_value())
      constraint_opt.emplace();

   auto& constraint = constraint_opt.value();
   if (constraint.comparisons.empty()) {
      constraint.comparisons.emplace_back();
   }
   auto& comparison = constraint.comparisons.back();

   comparison.comparand = this->ui.numSelectionsComparand->value();
   comparison.op = (worldinput::comparison_operator)this->ui.numSelectionsComparison->currentData().toInt();
}

void WorldinputConditionEditDialog::initializeFrom(const node_type& src) {
   this->_data = src.data;

   this->ui.name->setText(src.name);

   #pragma region Editor modes
   {
      using editor_mode = dovahkit::subsystems::worldedit::editor_mode;

      const auto& constraint_opt = this->_data.editor_modes;
      if (constraint_opt.has_value()) {
         const auto& constraint = constraint_opt.value();

         auto handler = [this, &constraint]<editor_mode Mode>(QCheckBox* widget) {
            const QSignalBlocker blocker(widget);
            widget->setChecked(constraint.test(Mode));
         };
         handler.operator()<editor_mode::navmesh>(this->ui.editModeNavmesh);
         handler.operator()<editor_mode::objects>(this->ui.editModeObject);
         handler.operator()<editor_mode::terrain>(this->ui.editModeTerrain);
      } else {
         std::array<QCheckBox*, 3> all_checkboxes = {
            this->ui.editModeNavmesh,
            this->ui.editModeObject,
            this->ui.editModeTerrain,
         };
         for (auto* widget : all_checkboxes) {
            const QSignalBlocker blocker(widget);
            widget->setChecked(true);
         }
      }
   }
   #pragma endregion
   #pragma region Gizmo modes
   {
      using gizmo_mode = dovahkit::subsystems::worldedit::gizmo_mode;

      const auto& constraint_opt = this->_data.gizmo_modes;
      if (constraint_opt.has_value()) {
         const auto& constraint = constraint_opt.value();

         auto handler = [this, &constraint]<gizmo_mode Mode>(QCheckBox* widget) {
            const QSignalBlocker blocker(widget);
            widget->setChecked(constraint.test(Mode));
         };
         handler.operator()<gizmo_mode::none>(this->ui.gizmoModeNone);
         handler.operator()<gizmo_mode::translate>(this->ui.gizmoModeTranslate);
         handler.operator()<gizmo_mode::rotate>(this->ui.gizmoModeRotate);
         handler.operator()<gizmo_mode::scale>(this->ui.gizmoModeScale);
      } else {
         std::array<QCheckBox*, 4> all_checkboxes = {
            this->ui.gizmoModeNone,
            this->ui.gizmoModeTranslate,
            this->ui.gizmoModeRotate,
            this->ui.gizmoModeScale,
         };
         for (auto* widget : all_checkboxes) {
            const QSignalBlocker blocker(widget);
            widget->setChecked(true);
         }
      }
   }
   #pragma endregion
   #pragma region Selection count
   {
      std::array<const QSignalBlocker, 3> blockers = {
         QSignalBlocker(this->ui.numSelectionsEnable),
         QSignalBlocker(this->ui.numSelectionsComparison),
         QSignalBlocker(this->ui.numSelectionsComparand)
      };

      this->ui.numSelectionsEnable->setChecked(false);
      this->ui.numSelectionsComparison->setEnabled(false);
      this->ui.numSelectionsComparand->setEnabled(false);

      const auto& constraint_opt = this->_data.selection_count;
      if (constraint_opt.has_value()) {
         const auto& constraint = constraint_opt.value();
         if (!constraint.comparisons.empty()) {
            const auto& comparison = constraint.comparisons[0];

            this->ui.numSelectionsEnable->setChecked(true);
            this->ui.numSelectionsComparison->setEnabled(true);
            this->ui.numSelectionsComparand->setEnabled(true);

            cobb::qt::set_combobox_value(this->ui.numSelectionsComparison, comparison.op);
            this->ui.numSelectionsComparand->setValue(comparison.comparand);
         }
      }
   }
   #pragma endregion
}
void WorldinputConditionEditDialog::overwrite(node_type& dst) const {
   dst.data = this->_data;
   dst.name = this->ui.name->text();
}