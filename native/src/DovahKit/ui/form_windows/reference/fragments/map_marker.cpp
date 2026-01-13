#include "./map_marker.h"
#include <QCoreApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QGroupBox>
#include <QLineEdit>
#include "widgets/DKFormPicker.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/extra_data/types/m/map_marker.h"
#include "dovah/forms/components/extra_data/types/r/radius.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/core.h"
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/set_range.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void map_marker::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;

      ui::set_unsigned_range<float>(this->controls.radius);
      {
         auto* widget = this->controls.icon;
         widget->clear();
			
			auto all_icon_names = std::array{
				QCoreApplication::translate("ExtraMapMarker icon", "None"),
				QCoreApplication::translate("ExtraMapMarker icon", "City"),
				QCoreApplication::translate("ExtraMapMarker icon", "Town"),
				QCoreApplication::translate("ExtraMapMarker icon", "Settlement"),
				QCoreApplication::translate("ExtraMapMarker icon", "Cave"),
				QCoreApplication::translate("ExtraMapMarker icon", "Camp"),
				QCoreApplication::translate("ExtraMapMarker icon", "Fort"),
				QCoreApplication::translate("ExtraMapMarker icon", "Nordic Ruin"),
				QCoreApplication::translate("ExtraMapMarker icon", "Dwemer Ruin"),
				QCoreApplication::translate("ExtraMapMarker icon", "Shipwreck"),
				QCoreApplication::translate("ExtraMapMarker icon", "Grove"),
				QCoreApplication::translate("ExtraMapMarker icon", "Landmark"),
				QCoreApplication::translate("ExtraMapMarker icon", "Dragon Lair"),
				QCoreApplication::translate("ExtraMapMarker icon", "Farm"),
				QCoreApplication::translate("ExtraMapMarker icon", "Wood Mill"),
				QCoreApplication::translate("ExtraMapMarker icon", "Mine"),
				QCoreApplication::translate("ExtraMapMarker icon", "Imperial Camp"),
				QCoreApplication::translate("ExtraMapMarker icon", "Stormcloak Camp"),
				QCoreApplication::translate("ExtraMapMarker icon", "Doomstone"),
				QCoreApplication::translate("ExtraMapMarker icon", "Wheat Mill"),
				QCoreApplication::translate("ExtraMapMarker icon", "Smelter"),
				QCoreApplication::translate("ExtraMapMarker icon", "Stable"),
				QCoreApplication::translate("ExtraMapMarker icon", "Imperial Tower"),
				QCoreApplication::translate("ExtraMapMarker icon", "Clearing"),
				QCoreApplication::translate("ExtraMapMarker icon", "Pass"),
				QCoreApplication::translate("ExtraMapMarker icon", "Altar"),
				QCoreApplication::translate("ExtraMapMarker icon", "Rock"),
				QCoreApplication::translate("ExtraMapMarker icon", "Lighthouse"),
				QCoreApplication::translate("ExtraMapMarker icon", "Orc Stronghold"),
				QCoreApplication::translate("ExtraMapMarker icon", "Giant Camp"),
				QCoreApplication::translate("ExtraMapMarker icon", "Shack"),
				QCoreApplication::translate("ExtraMapMarker icon", "Nordic Tower"),
				QCoreApplication::translate("ExtraMapMarker icon", "Nordic Dwelling"),
				QCoreApplication::translate("ExtraMapMarker icon", "Docks"),
				QCoreApplication::translate("ExtraMapMarker icon", "Shrine"),
				QCoreApplication::translate("ExtraMapMarker icon", "Riften Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Riften Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Windhelm Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Windhelm Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Whiterun Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Whiterun Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Solitude Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Solitude Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Markarth Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Markarth Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Winterhold Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Winterhold Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Morthal Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Morthal Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Falkreath Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Falkreath Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "Dawnstar Castle"),
				QCoreApplication::translate("ExtraMapMarker icon", "Dawnstar Capitol"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- Temple of Miraak"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- Raven Rock"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- Standing Stones"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- Telvanni Tower"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- To Skyrim"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- To Solstheim"),
				QCoreApplication::translate("ExtraMapMarker icon", "DLC02 -- Castle Karstaag"),
			};
			for (size_t i = 0; i < all_icon_names.size(); ++i) {
				widget->addItem(all_icon_names[i], i);
			}
			widget->model()->sort(0);
      }
   }
   void map_marker::load(loaded_form_type& form) {
      if (auto* extra = form.extra_data.get<extra_data_type>()) {
			auto& gls = dovahkit::subsystems::game_localized_strings::core::get();
         this->controls.groupbox->setChecked(true);
         this->controls.name->setText(gls.convert_localized_string(extra->name));
         this->controls.icon->setCurrentIndex(this->controls.icon->findData((int)extra->type));
         this->controls.flags.is_visible->setChecked(extra->flags & extra_data_type::flag::visible);
         this->controls.flags.can_be_fast_traveled_to->setChecked(extra->flags & extra_data_type::flag::can_travel_to);
         this->controls.flags.is_unaffected_by_show_all->setChecked(extra->flags & extra_data_type::flag::show_all_hidden);
      } else {
         this->controls.groupbox->setChecked(false);
      }

      if (auto* extra = form.extra_data.get<extra_data_types::radius>())
         this->controls.radius->setValue(extra->value);
   }
   void map_marker::save(loaded_form_type& form) {
      if (!is_map_marker(form)) {
         this->controls.groupbox->setChecked(false);
         form.extra_data.remove<extra_data_type>(form);
         return;
      }
      if (this->controls.groupbox->isChecked()) {
         form.extra_data.remove<extra_data_type>(form);
      } else {
			auto& gls   = dovahkit::subsystems::game_localized_strings::core::get();
         auto* extra = form.extra_data.get_or_create<extra_data_type>();
         gls.assign_localized_string(extra->name, this->controls.name->text());
         extra->type  = this->controls.icon->currentData().toInt();
         cobb::edit_bit(extra->flags, extra_data_type::flag::visible, this->controls.flags.is_visible->isChecked());
         cobb::edit_bit(extra->flags, extra_data_type::flag::can_travel_to, this->controls.flags.can_be_fast_traveled_to->isChecked());
         cobb::edit_bit(extra->flags, extra_data_type::flag::show_all_hidden, this->controls.flags.is_unaffected_by_show_all->isChecked());
      }
      
      float radius = this->controls.radius->value();
      if (radius) {
         form.extra_data.get_or_create<extra_data_types::radius>()->value = radius;
      } else {
         form.extra_data.remove<extra_data_types::radius>(form);
      }
   }
	bool map_marker::is_map_marker(const loaded_form_type& form) {
		auto* base_form = form.base_form.get_form_stub();
		if (!base_form)
			return false;
		return base_form->formID == dovah::hardcoded_form_ids::MapMarker;
	}
}