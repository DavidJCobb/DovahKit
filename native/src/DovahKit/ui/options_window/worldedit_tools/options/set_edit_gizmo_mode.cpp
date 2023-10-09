#include "./set_edit_gizmo_mode.h"
#include <array>
#include <type_traits>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   set_edit_gizmo_mode::set_edit_gizmo_mode(QWidget* parent) : base(parent) {
      this->ui.setupUi(this);

      this->_widget_wrappers.frames.a = this->ui.frameA;
      this->_widget_wrappers.frames.b = this->ui.frameB;
      this->_widget_wrappers.modes.a  = this->ui.modeA;
      this->_widget_wrappers.modes.b  = this->ui.modeB;
      {
         auto& widgets = this->_widget_wrappers.frames;

         using enum_type = decltype(widgets.a)::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::camera, "Camera" },
            item_type{ enum_type::local,  "Selection" },
            item_type{ enum_type::world,  "World" },
         };

         widgets.a.addItems(items);
         widgets.b.addItems(items);

         auto& fields = this->_state.current_options.frame;
         widgets.a.beginOneWaySync(fields.a);
         widgets.b.beginOneWaySync(fields.b);

         QObject::connect(this->ui.frameGroupbox, &QGroupBox::toggled, this, [this, &widgets](bool checked) {
            auto& data = this->_state.current_options;
            if (checked) {
               data.frame.a = widgets.a.value();
               data.frame.b = widgets.b.value();
            } else {
               data.frame.a = data.frame.b = enum_type::current;
            }
         });
         cobb::qt::bind(this->ui.frameToggle, this->_state.current_options.toggle_frame);
      }
      {
         auto& widgets = this->_widget_wrappers.modes;

         using enum_type = decltype(widgets.a)::value_type;
         using item_type = std::pair<enum_type, const char*>;

         constexpr const auto items = std::array{
            item_type{ enum_type::translate, "Translate" },
            item_type{ enum_type::rotate,    "Rotate" },
            item_type{ enum_type::scale,     "Scale" },
            item_type{ enum_type::none,      "Hidden" },
         };

         widgets.a.addItems(items);
         widgets.b.addItems(items);

         auto& fields = this->_state.current_options.gizmo;
         widgets.a.beginOneWaySync(fields.a);
         widgets.b.beginOneWaySync(fields.b);

         QObject::connect(this->ui.modeGroupbox, &QGroupBox::toggled, this, [this, &widgets](bool checked) {
            this->_state.current_options.modify_gizmo = checked;
         });
         cobb::qt::bind(this->ui.modeToggle, this->_state.current_options.toggle_gizmo);
      }
   }

   void set_edit_gizmo_mode::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto& fields  = this->_state.current_options.frame;
         auto& widgets = this->_widget_wrappers.frames;

         constexpr auto fallback = decltype(fields.a)::local;

         bool no_modify = fields.a == decltype(fields.a)::current;

         if (!widgets.a.setValueSilent(fields.a, fallback)) {
            if (!no_modify)
               fields.a = fallback;
         }
         if (!widgets.b.setValueSilent(fields.b, fallback))
            fields.b = fallback;

         const auto blocker_g = QSignalBlocker(this->ui.frameGroupbox);
         const auto blocker_t = QSignalBlocker(this->ui.frameToggle);

         this->ui.frameGroupbox->setChecked(!no_modify);
         this->ui.frameToggle->setChecked(this->_state.current_options.toggle_frame);
      }
      {
         auto& fields  = this->_state.current_options.gizmo;
         auto& widgets = this->_widget_wrappers.modes;

         constexpr auto fallback = decltype(fields.a)::translate;
         
         if (!widgets.a.setValueSilent(fields.a, fallback))
            fields.a = fallback;
         if (!widgets.b.setValueSilent(fields.b, fallback))
            fields.b = fallback;

         const auto blocker_g = QSignalBlocker(this->ui.modeGroupbox);
         const auto blocker_t = QSignalBlocker(this->ui.modeToggle);

         this->ui.modeGroupbox->setChecked(this->_state.current_options.modify_gizmo);
         this->ui.modeToggle->setChecked(this->_state.current_options.toggle_gizmo);
      }
   }
}