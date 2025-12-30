#include "./primitive.h"
#include <cassert>
#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDoubleSpinBox>
#include <QLabel>
#include "helpers/bound_mem_fn.h"
#include "widgets/DKColorPickerButton.h"
#include "dovah/data/collision_layers.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/extra_data/types/c/collision_data.h"
#include "dovah/forms/components/extra_data/types/m/multibound_bounds.h"
#include "dovah/forms/components/extra_data/types/p/primitive.h"
#include "dovah/forms/components/extra_data/types/r/room_ref_data.h"
#include "dovah/forms/ObjectReference.h"
#include "dovah/utils/default_primitive_color_for_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/localize/collision_layer.h"
#include "editor/subsystems/message_log/core.h"
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void primitive::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      
      {
         using shape_type = extra_data_type::shape_enum;
         QComboBox* widget = this->controls.shape;
         widget->clear();
         widget->addItem(owner.tr("None"),   (int)shape_type::none);
         widget->addItem(owner.tr("Box"),    (int)shape_type::box);
         widget->addItem(owner.tr("Plane"),  (int)shape_type::portal_box);
         widget->addItem(owner.tr("Sphere"), (int)shape_type::sphere);
         widget->addItem(owner.tr("Line"),   (int)shape_type::line);
      }
      {
         QComboBox* widget = this->controls.layer;
         widget->clear();
         for (auto cl : dovah::all_collision_layers) {
            widget->addItem(
               editor::localize::collision_layer(cl),
               (int)cl
            );
         }
      }
      QObject::connect(this->controls.layer,             qOverload<int>(&QComboBox::currentIndexChanged), &owner, cobb__bound_this_fn(_on_collision_layer_changed));
      QObject::connect(this->controls.player_activation, &QCheckBox::toggled, &owner, cobb__bound_this_fn(_set_player_activation));

      //
      // Have the Primitive Origin and World Coordinates Position spinboxes mirror each other.
      //
      for (size_t i = 0; i < 3; ++i) {
         auto* ref_pos = this->controls.ref_position.all[i];
         auto* origin  = this->controls.origin.all[i];
         QObject::connect(ref_pos, qOverload<double>(&QDoubleSpinBox::valueChanged), origin, [origin](double v) {
            const auto blocker = QSignalBlocker(origin);
            origin->setValue(v);
         });
         QObject::connect(origin, qOverload<double>(&QDoubleSpinBox::valueChanged), ref_pos, &QDoubleSpinBox::setValue);
      }
   }
   void primitive::issue_initial_warnings(loaded_form_type& form) {
      //
      // If the ref is a primitive and a multibound, warn if those two sets of 
      // sizes don't match.
      //
      auto* extra_prim = form.extra_data.get<extra_data_types::primitive>();
      auto* extra_mbnd = form.extra_data.get<extra_data_types::multibound_bounds>();
      if (!extra_prim || !extra_mbnd)
         return;
      if (extra_prim->bounds != extra_mbnd->halfwidths * 2) {
         auto& logger = dovahkit::subsystems::message_log::core::get();
         logger.addLogItem(ui::types::log_item(
            QCoreApplication::translate(
               "frontend REFR load warnings",
               //
               "Multibound %1 has internally inconsistent size values. These will be "
               "corrected when you click \"OK.\""
            ).arg(editor_helpers::form_identifiers_to_string(&form.stub)),
            ui::types::log_item_type::warning,
            ui::types::log_item_context::form_load
         ));
      }
   }
   void primitive::load(loaded_form_type& form) {
      this->stub = &form.stub;

      auto* base_form = form.base_form.get_form_stub();
      auto  base_type = base_form ? base_form->form_type : dovah::form_type::none;
      if (!is_primitive(form))
         return;
      const auto blockers = std::array{
         QSignalBlocker(this->controls.color),
         QSignalBlocker(this->controls.layer),
         QSignalBlocker(this->controls.player_activation),
         QSignalBlocker(this->controls.shape),
      };

      this->controls.function->setText(_primitive_function_text(base_form));

      bool is_typically_flat = false;
      if (base_form) {
         switch (base_form->formID) {
            case dovah::hardcoded_form_ids::PlaneMarker:
            case dovah::hardcoded_form_ids::PortalMarker:
               is_typically_flat = true;
               break;
         }
      }

      extra_data_type* extra = nullptr;
      if (extra = form.extra_data.get<extra_data_type>()) {
         if (auto* extra_bound = form.extra_data.get<extra_data_types::multibound_bounds>()) {
            if (extra->bounds != extra_bound->halfwidths) {
               //
               // We've already warned the user about this, so just correct it now.
               //
               extra_bound->halfwidths = extra->bounds / 2;
            }
         }
      } else {
         extra = form.extra_data.get_or_create<extra_data_type>();
         if (is_typically_flat) {
            extra->shape = extra_data_type::shape::portal_box;
         } else {
            extra->shape = extra_data_type::shape::box;
         }
         if (base_form) {
            extra->color = dovah::utils::default_primitive_color_for_base_form(*base_form);
         }
         if (auto* extra_bound = form.extra_data.get<extra_data_types::multibound_bounds>()) {
            extra->bounds = extra_bound->halfwidths * 2;
         }
      }
      this->controls.color->setColor(QColor::fromRgbF(extra->color.r, extra->color.g, extra->color.b));
      this->controls.shape->setCurrentIndex(this->controls.shape->findData((int)extra->shape));
      this->controls.shape->setEnabled(can_change_shape(form));
      for (size_t i = 0; i < this->controls.extents.all.size(); ++i) {
         auto* widget  = this->controls.extents.all[i];
         auto  blocker = QSignalBlocker(widget);
         widget->setValue(extra->bounds[i]);
      }
      //
      // The "Player Activation" checkbox just changes the layer type to L_NONCOLLIDABLE. 
      // If the base form is an Activator, then this enables an activation prompt on the 
      // primitive.
      //
      {
         QComboBox* widget = this->controls.layer;
         if (auto* extra = form.extra_data.get<extra_data_types::collision_data>()) {
            auto i = widget->findData((int)extra->value);
            if (i < 0)
               i = widget->findData((int)dovah::collision_layer::unidentified);
            widget->setCurrentIndex(i);
         } else {
            widget->setCurrentIndex(widget->findData((int)dovah::collision_layer::null));
         }
         _on_collision_layer_changed();
      }
      this->controls.player_activation->setEnabled(base_type == dovah::form_type::activator);
   }
   void primitive::save(loaded_form_type& form) {
      if (!is_primitive(form))
         return;
      auto* extra_prim = form.extra_data.get_or_create<extra_data_type>();
      for (size_t i = 0; i < this->controls.extents.all.size(); ++i)
         extra_prim->bounds[i] = this->controls.extents.all[i]->value();
      {
         auto color = this->controls.color->color();
         extra_prim->color = {
            .r = (float)color.redF(),
            .g = (float)color.greenF(),
            .b = (float)color.blueF(),
            .a = extra_prim->color.a,
         };
      }
      if (can_change_shape(form)) {
         extra_prim->shape = (enum extra_data_type::shape) this->controls.shape->currentData().toInt();
      }

      auto layer = (dovah::collision_layer) this->controls.layer->currentData().toInt();
      if (layer == dovah::collision_layer::unidentified) {
         form.extra_data.remove<extra_data_types::collision_data>(form);
      } else {
         form.extra_data.get_or_create<extra_data_types::collision_data>()->set_layer_id(layer);
      }

      if (auto* extra_bound = form.extra_data.get<extra_data_types::multibound_bounds>()) {
         extra_bound->halfwidths = extra_prim->bounds / 2;
      }
   }

   bool primitive::is_primitive(const loaded_form_type& form) {
      dovah::form_stub* base_form = form.base_form.get_form_stub();
      if (!base_form)
         return false;

      switch (base_form->form_type) {
         case dovah::form_type::acoustic_space:
         case dovah::form_type::sound:
            return true;
         case dovah::form_type::activator:
            return form.extra_data.get<extra_data_type>() != nullptr;
      }
      switch (base_form->formID) {
         case dovah::hardcoded_form_ids::CollisionMarker:
         case dovah::hardcoded_form_ids::MultiBoundMarker:
         case dovah::hardcoded_form_ids::PlaneMarker:
         case dovah::hardcoded_form_ids::PortalMarker:
         case dovah::hardcoded_form_ids::RoomMarker:
            return true;
      }
      return false;
   }
   bool primitive::can_change_shape(const loaded_form_type& form) {
      dovah::form_stub* base_form = form.base_form.get_form_stub();
      if (!base_form)
         return false;
      return base_form->form_type == dovah::form_type::activator;
   }

   QString primitive::_primitive_function_text(dovah::form_stub* base_form) {
      if (!base_form)
         return QCoreApplication::translate("BGSPrimitive purpose", "???");
      const auto base_type = base_form->form_type;
      switch (base_type) {
         case dovah::form_type::acoustic_space:
            return QCoreApplication::translate("BGSPrimitive purpose", "Acoustic Space");
         case dovah::form_type::activator:
            return QCoreApplication::translate("BGSPrimitive purpose", "Trigger");
         case dovah::form_type::sound:
            return QCoreApplication::translate("BGSPrimitive purpose", "Sound Emitter");
         default:
            switch (base_form->formID) {
               case dovah::hardcoded_form_ids::CollisionMarker:
                  return QCoreApplication::translate("BGSPrimitive purpose", "Collision Object");
               case dovah::hardcoded_form_ids::MultiBoundMarker:
                  return QCoreApplication::translate("BGSPrimitive purpose", "Multibound");
               case dovah::hardcoded_form_ids::PlaneMarker:
                  return QCoreApplication::translate("BGSPrimitive purpose", "Occlusion Plane");
               case dovah::hardcoded_form_ids::PortalMarker:
                  return QCoreApplication::translate("BGSPrimitive purpose", "Portal");
               case dovah::hardcoded_form_ids::RoomMarker:
                  {
                     auto* extra = loaded().extra_data.get<extra_data_types::room_ref_data>();
                     if (extra && extra->is_master) {
                        return QCoreApplication::translate("BGSPrimitive purpose", "Room (master)");
                     }
                  }
                  // This is Bethesda's terminology. Can we figure out a better word? 
                  // When does the CK even decide to make a roombound a "master?"
                  return QCoreApplication::translate("BGSPrimitive purpose", "Room (slave)");
            }
            break;
      }
      return QCoreApplication::translate("BGSPrimitive purpose", "Static");
   }
   void primitive::_on_collision_layer_changed() {
      auto* widget = this->controls.layer;
      auto  layer = (dovah::collision_layer)widget->currentData().toInt();
      if (layer == layer_for_player_activate_primitives) {
         auto* base_form = loaded().base_form.get_form_stub();
         if (base_form && base_form->form_type == dovah::form_type::activator) {
            this->controls.player_activation->setChecked(true);
         }
      } else {
         this->state.prior_layer = layer;
      }
   }
   void primitive::_set_player_activation(bool checked) {
      QComboBox* widget  = this->controls.layer;
      const auto blocker = QSignalBlocker(widget);

      dovah::collision_layer layer;
      if (checked) {
         layer = layer_for_player_activate_primitives;
      } else {
         layer = this->state.prior_layer;
      }
      widget->setCurrentIndex(widget->findData((int)layer));
   }

   primitive::loaded_form_type& primitive::loaded() {
      assert(this->stub != nullptr);
      return *this->stub->get_working_copy<loaded_form_type>();
   }
}