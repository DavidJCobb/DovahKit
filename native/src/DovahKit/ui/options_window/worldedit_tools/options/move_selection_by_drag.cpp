#include "./move_selection_by_drag.h"
#include <array>
#include <type_traits>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   move_selection_by_drag::move_selection_by_drag(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);
      {
         auto* group = this->_typeButtonGroup = new QButtonGroup(this);
         group->addButton(this->ui.useAxis);
         group->addButton(this->ui.usePlane);

         this->ui.useAxis->setChecked(true);
         this->ui.axis->setEnabled(true);
         this->ui.plane->setEnabled(false);

         QObject::connect(this->ui.useAxis, &QRadioButton::toggled, this, [this](bool checked) {
            this->ui.axis->setEnabled(checked);
            this->ui.plane->setEnabled(!checked);

            this->_state.current_options.is_plane = !checked;
            auto& val_axis = this->_state.current_options.axis;

            if (checked) {
               val_axis = this->_widget_wrappers.drag_axis_axis.value();
            } else {
               val_axis = this->_widget_wrappers.drag_plane_normal.value();
            }
         });
      }
      {
         auto* widget  = this->ui.frame;
         auto& wrapper = this->_widget_wrappers.frame;
         wrapper = widget;

         using enum_type = dovahkit::subsystems::worldedit::reference_frame;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::camera,  "Camera" },
            item_type{ enum_type::current, "Edit gizmo" },
            item_type{ enum_type::local,   "Selection" },
            item_type{ enum_type::world,   "World" },
         };

         wrapper.addItems(items, "reference frame");
         wrapper.beginOneWaySync(this->_state.current_options.frame);
      }
      {
         auto* widget  = this->ui.axis;
         auto& wrapper = this->_widget_wrappers.drag_axis_axis;
         wrapper = widget;

         using enum_type = std::decay_t<decltype(wrapper)>::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::x, "X" },
            item_type{ enum_type::y, "Y" },
            item_type{ enum_type::z, "Z" },
         };

         wrapper.addItems(items, "3D axes");
         wrapper.beginOneWaySync(this->_state.current_options.axis);
      }
      {
         auto* widget  = this->ui.plane;
         auto& wrapper = this->_widget_wrappers.drag_plane_normal;
         wrapper = widget;

         using enum_type = std::decay_t<decltype(wrapper)>::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::z, "XY" },
            item_type{ enum_type::y, "XZ" },
            item_type{ enum_type::x, "YZ" },
         };

         wrapper.addItems(items, "3D plane axis pairs");
         wrapper.beginOneWaySync(this->_state.current_options.axis);
      }
   }

   void move_selection_by_drag::set_options(const options_type& v) {
      this->_state.current_options = v;

      this->_widget_wrappers.frame.setValueSilent(v.frame);
      
      if (v.is_plane) {
         this->_widget_wrappers.drag_plane_normal.setValueSilent(v.axis);
      } else {
         this->_widget_wrappers.drag_axis_axis.setValueSilent(v.axis);
      }
      this->ui.useAxis->setChecked(!v.is_plane);
      this->ui.usePlane->setChecked(v.is_plane);
      this->ui.axis->setEnabled(!v.is_plane);
      this->ui.plane->setEnabled(v.is_plane);
   }
}