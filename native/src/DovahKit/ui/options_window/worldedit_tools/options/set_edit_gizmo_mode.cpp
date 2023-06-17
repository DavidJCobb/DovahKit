#include "./set_edit_gizmo_mode.h"
#include <array>
#include <type_traits>
#include <QComboBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include "helpers/qt/basic_bindings.h"

namespace dovahkit::ui::worldedit::tools {
   set_edit_gizmo_mode::set_edit_gizmo_mode(QWidget* parent) : QWidget(parent) {
      auto* layout = new QGridLayout(this);
      this->setLayout(layout);

      int row = 0;

      {
         auto& widgets  = this->_subwidgets.frames;
         auto* groupbox = widgets.groupbox = new QGroupBox(tr("Reference frame"));
         groupbox->setCheckable(true);
         layout->addWidget(groupbox, row, 0);

         int   gb_row = 0;
         auto* gb_layout = new QGridLayout(groupbox);
         groupbox->setLayout(gb_layout);

         widgets.a = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Change to:")), gb_row, 0);
         gb_layout->addWidget(widgets.a, gb_row, 1);
         ++gb_row;

         widgets.toggle = new QCheckBox(tr("Toggle between:"));
         widgets.b = new QComboBox;
         gb_layout->addWidget(widgets.toggle, gb_row, 0);
         gb_layout->addWidget(widgets.b, gb_row, 1);
         ++gb_row;
      }
      ++row;
      
      {
         auto& widgets  = this->_subwidgets.modes;
         auto* groupbox = widgets.groupbox = new QGroupBox(tr("Mode"));
         groupbox->setCheckable(true);
         layout->addWidget(groupbox, row, 0);

         int   gb_row = 0;
         auto* gb_layout = new QGridLayout(groupbox);
         groupbox->setLayout(gb_layout);

         widgets.a = new QComboBox;
         gb_layout->addWidget(new QLabel(tr("Change to:")), gb_row, 0);
         gb_layout->addWidget(widgets.a, gb_row, 1);
         ++gb_row;

         widgets.toggle = new QCheckBox(tr("Toggle between:"));
         widgets.b = new QComboBox;
         gb_layout->addWidget(widgets.toggle, gb_row, 0);
         gb_layout->addWidget(widgets.b, gb_row, 1);
         ++gb_row;
      }
      ++row;

      //
      // Layout done; set up the fields:
      //

      {
         auto& widgets = this->_subwidgets.frames;

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

         QObject::connect(widgets.groupbox, &QGroupBox::toggled, this, [this, &widgets](bool checked) {
            auto& data = this->_state.current_options;
            if (checked) {
               data.frame.a = widgets.a.value();
               data.frame.b = widgets.b.value();
            } else {
               data.frame.a = data.frame.b = enum_type::current;
            }
         });
         cobb::qt::bind(widgets.toggle, this->_state.current_options.toggle_frame);
      }
      {
         auto& widgets = this->_subwidgets.modes;

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

         QObject::connect(widgets.groupbox, &QGroupBox::toggled, this, [this, &widgets](bool checked) {
            this->_state.current_options.modify_gizmo = checked;
         });
         cobb::qt::bind(widgets.toggle, this->_state.current_options.toggle_gizmo);
      }
   }

   void set_edit_gizmo_mode::set_options(const options_type& v) {
      this->_state.current_options = v;
      {
         auto& fields = this->_state.current_options.frame;
         auto& widgets = this->_subwidgets.frames;

         constexpr auto fallback = decltype(fields.a)::local;

         bool no_modify = fields.a == decltype(fields.a)::current;

         if (!widgets.a.setValueSilent(fields.a, fallback)) {
            if (!no_modify)
               fields.a = fallback;
         }
         if (!widgets.b.setValueSilent(fields.b, fallback))
            fields.b = fallback;

         const auto blocker_g = QSignalBlocker(widgets.groupbox);
         const auto blocker_t = QSignalBlocker(widgets.toggle);

         widgets.groupbox->setChecked(!no_modify);
         widgets.toggle->setChecked(this->_state.current_options.toggle_frame);
      }
      {
         auto& fields = this->_state.current_options.gizmo;
         auto& widgets = this->_subwidgets.modes;

         constexpr auto fallback = decltype(fields.a)::translate;
         
         if (!widgets.a.setValueSilent(fields.a, fallback))
            fields.a = fallback;
         if (!widgets.b.setValueSilent(fields.b, fallback))
            fields.b = fallback;

         const auto blocker_g = QSignalBlocker(widgets.groupbox);
         const auto blocker_t = QSignalBlocker(widgets.toggle);

         widgets.groupbox->setChecked(this->_state.current_options.modify_gizmo);
         widgets.toggle->setChecked(this->_state.current_options.toggle_gizmo);
      }
   }
}