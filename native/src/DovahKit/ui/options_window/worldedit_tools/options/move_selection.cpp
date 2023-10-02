#include "./move_selection.h"
#include <array>
#include <type_traits>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   move_selection::move_selection(QWidget* parent) : QWidget(parent) {
      this->ui.setupUi(this);
      {
         auto* group = this->_typeButtonGroup = new QButtonGroup(this);
         group->addButton(this->ui.movementTypeCursor);
         group->addButton(this->ui.movementTypeCustom);

         QObject::connect(this->ui.movementTypeCursor, &QRadioButton::toggled, this, [this](bool checked) {
            this->ui.customMovementGroupbox->setEnabled(!checked);
         });
      }
      {
         auto* widget_movement  = this->ui.transformFrame;
         auto* widget_constrain = this->ui.constraintFrame;

         using enum_type = dovahkit::subsystems::worldedit::reference_frame;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::camera,  "Camera" },
            item_type{ enum_type::current, "Edit gizmo" },
            item_type{ enum_type::local,   "Selection" },
            item_type{ enum_type::world,   "World" },
         };

         widget_movement->addItem(tr("Camera",     "reference frame"), (int)enum_type::camera);
         widget_movement->addItem(tr("Edit gizmo", "reference frame"), (int)enum_type::current);
         widget_movement->addItem(tr("Selection",  "reference frame"), (int)enum_type::local);
         widget_movement->addItem(tr("World",      "reference frame"), (int)enum_type::world);

         widget_constrain->addItem(tr("Camera",     "reference frame"), (int)enum_type::camera);
         widget_constrain->addItem(tr("Edit gizmo", "reference frame"), (int)enum_type::current);
         widget_constrain->addItem(tr("Selection",  "reference frame"), (int)enum_type::local);
         widget_constrain->addItem(tr("World",      "reference frame"), (int)enum_type::world);

         auto& cf = this->_widget_wrappers.constraintFrame;
         auto& tf = this->_widget_wrappers.transformFrame;
         cf = widget_constrain;
         tf = widget_movement;
         tf.beginOneWaySync(this->_state.current_options.frame);
         cf.beginOneWaySync(this->_state.current_options.locked_axes.frame);
      }
      QObject::connect(this->ui.constraintX, &QCheckBox::toggled, this, [this](bool checked) {
         this->_state.current_options.locked_axes.x = !checked;
      });
      QObject::connect(this->ui.constraintY, &QCheckBox::toggled, this, [this](bool checked) {
         this->_state.current_options.locked_axes.y = !checked;
      });
      QObject::connect(this->ui.constraintZ, &QCheckBox::toggled, this, [this](bool checked) {
         this->_state.current_options.locked_axes.z = !checked;
      });
      {
         this->ui.customMagnitudeX->setRange(-4096, 4096);
         this->ui.customMagnitudeY->setRange(-4096, 4096);
         this->ui.customMagnitudeZ->setRange(-4096, 4096);
         
         auto& fields = this->_state.current_options.magnitudes;
         cobb::qt::bind(this->ui.customMagnitudeX, fields.x);
         cobb::qt::bind(this->ui.customMagnitudeY, fields.y);
         cobb::qt::bind(this->ui.customMagnitudeZ, fields.z);

         cobb::qt::bind(this->ui.alsoMoveCamera, this->_state.current_options.also_move_camera);
      }
      this->ui.rangeInputScales->setSyncTarget(&this->_state.current_options.range);
   }

   void move_selection::set_options(const options_type& v) {
      this->_state.current_options = v;

      this->_widget_wrappers.transformFrame.setValueSilent(v.frame);
      this->_widget_wrappers.constraintFrame.setValueSilent(v.locked_axes.frame);
      {
         const auto blocker_x = QSignalBlocker(this->ui.constraintX);
         const auto blocker_y = QSignalBlocker(this->ui.constraintY);
         const auto blocker_z = QSignalBlocker(this->ui.constraintZ);

         this->ui.constraintX->setChecked(!v.locked_axes.x);
         this->ui.constraintY->setChecked(!v.locked_axes.y);
         this->ui.constraintZ->setChecked(!v.locked_axes.z);
      }
      {
         const auto blocker_1 = QSignalBlocker(this->_typeButtonGroup);
         const auto blocker_2 = QSignalBlocker(this->ui.movementTypeCursor);
         const auto blocker_3 = QSignalBlocker(this->ui.movementTypeCustom);

         this->ui.movementTypeCursor->setChecked(v.follow_pointer);
         this->ui.movementTypeCustom->setChecked(!v.follow_pointer);
         this->ui.customMovementGroupbox->setEnabled(!v.follow_pointer);
      }
      {
         const auto blocker_x = QSignalBlocker(this->ui.customMagnitudeX);
         const auto blocker_y = QSignalBlocker(this->ui.customMagnitudeY);
         const auto blocker_z = QSignalBlocker(this->ui.customMagnitudeZ);

         this->ui.customMagnitudeX->setValue(v.magnitudes.x);
         this->ui.customMagnitudeY->setValue(v.magnitudes.y);
         this->ui.customMagnitudeZ->setValue(v.magnitudes.z);
      }
      {
         const auto blocker = QSignalBlocker(this->ui.alsoMoveCamera);
         this->ui.alsoMoveCamera->setChecked(v.also_move_camera);
      }
      this->ui.rangeInputScales->reloadFromSyncTarget();
   }
}