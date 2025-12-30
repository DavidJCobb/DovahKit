#include "./teleport.h"
#include <QCoreApplication>
#include <QGroupBox>
#include <QMessageBox>
#include <QPushButton>
#include <QVariant>
#include "widgets/DKFormPicker.h"
#include "widgets/DKObjectReferencePicker.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "dovah/forms/components/extra_data/types/r/random_teleport_marker.h"
#include "dovah/forms/components/extra_data/types/t/teleport.h"
#include "dovah/forms/components/extra_data/types/t/teleport_name.h"
#include "dovah/forms/Door.h"
#include "dovah/forms/ObjectReference.h"
#include "editor/subsystems/worldedit/core.h"
namespace {
   constexpr const bool render_window_displays_teleport_markers = false;
}
namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }
}

namespace ui::reference::fragments {
   void teleport::setup(QWidget& owner, const control_collection& controls) {
      this->controls = controls;
      this->owner    = &owner;
      
      controls.name->setAllowedFormType(dovah::form_type::message);

      QObject::connect(this->controls.buttons.view_marker, &QPushButton::clicked, &owner, std::mem_fn(&_view_linked_marker));
      QObject::connect(this->controls.buttons.view_ref,    &QPushButton::clicked, &owner, std::mem_fn(&_view_linked_door));

      QObject::connect(this->controls.ref, &DKObjectReferencePicker::refChanged, &owner, std::mem_fn(&_on_ref_picked));
      this->controls.ref->setValidationFunction([this](dovah::form_stub* ref) { return this->_validate_picked_ref(ref); });
   }
   void teleport::load(loaded_form_type& form) {
      this->stub = &form.stub;

      this->state.is_random_door = _is_random_teleport_door(form);
      if (this->state.is_random_door) {
         this->controls.groupbox->setChecked(true);
         this->controls.groupbox->setCheckable(false);
         this->controls.ref->setEnabled(true);
         this->controls.buttons.view_marker->setEnabled(false);
         this->controls.buttons.view_ref->setEnabled(false);
         if (auto* extra = form.extra_data.get<extra_data_types::random_teleport_marker>()) {
            auto* ref = extra->form.get_form_stub();
            this->controls.ref->setRef(ref);
            this->controls.buttons.view_marker->setEnabled(ref != nullptr);
         }
      } else {
         this->controls.groupbox->setCheckable(true);
         if (auto* extra = form.extra_data.get<extra_data_type>()) {
            this->controls.groupbox->setChecked(true);

            auto* ref = extra->target_door.get_form_stub();
            if (ref && !dovah::form_type_is_reference(ref->form_type))
               ref = nullptr;

            this->controls.ref->setRef(ref);
            this->controls.buttons.view_ref->setEnabled(ref != nullptr);
            if constexpr (render_window_displays_teleport_markers) {
               this->controls.buttons.view_marker->setEnabled(ref != nullptr);
            }
         } else {
            this->controls.groupbox->setChecked(false);
            this->controls.buttons.view_ref->setEnabled(false);
            this->controls.buttons.view_marker->setEnabled(false);
         }
      }

      if (auto* extra = form.extra_data.get<extra_data_types::teleport_name>()) {
         this->controls.name->setFormStub(extra->form.get_form_stub());
      }
   }
   void teleport::save(loaded_form_type& form) {
      auto* prior_destination = _destination_of(form);
      bool  disconnect_prior_destination = false;

      bool is_door = false;
      if (auto* base_form = form.base_form.get_form_stub())
         is_door = base_form->form_type == dovah::form_type::door;

      if (this->controls.groupbox->isChecked()) {
         form.extra_data.remove<extra_data_type>(form);
         form.extra_data.remove<extra_data_types::random_teleport_marker>(form);
         disconnect_prior_destination = true;
      } else if (is_door) {
         auto* destination = this->controls.ref->ref();
         if (destination && !_is_legal_teleport_destination(*destination)) {
            QMessageBox::critical(
               this->owner,
               QCoreApplication::translate("ui::reference::fragments::teleport", "Error"),
               QCoreApplication::translate("ui::reference::fragments::teleport",
                  "The destination that you selected for this load door was edited at some "
                  "point between you selecting it and you clicking OK. Those edits have "
                  "caused it to no longer be a valid destination (e.g. because it's not a "
                  "door anymore, or because it's been connected to another load door. This "
                  "ref will not be connected to that destination."
               )
            );
            destination = nullptr;
         }
         auto* extra = form.extra_data.get_or_create<extra_data_type>();
         extra->target_door.set(form, destination);

         if (destination) {
            _auto_place_teleport_marker(form, *extra, *destination);
         }
         disconnect_prior_destination = destination != prior_destination;
      }
      if (disconnect_prior_destination && prior_destination) {
         //
         // This door was previously connected to some other door. We've broken that 
         // connection, so delete the old destination's teleport data.
         //
         _disconnect_old_destination(*prior_destination);
      }

      if (auto* name = this->controls.name->formStub()) {
         form.extra_data.get_or_create<extra_data_types::teleport_name>()->form.set(form, name);
      } else {
         form.extra_data.remove<extra_data_types::teleport_name>(form);
      }
   }

   bool teleport::_is_random_teleport_door(loaded_form_type& form) {
      auto* base_stub = form.base_form.get_form_stub();
      if (!base_stub || base_stub->form_type != dovah::form_type::door)
         return false;
      auto  base_form = base_stub->load().ptr_cast<dovah::loaded_forms::Door>();
      if (!base_form)
         return false;
      return !base_form->random_destinations.empty();
   }
   void teleport::_view_linked_door() {
      if (this->state.is_random_door)
         return;
      auto* ref = this->controls.ref->ref();
      if (!ref)
         return;
      dovahkit::subsystems::worldedit::core::get().center_on_refr(*ref);
   }
   void teleport::_view_linked_marker() {
      dovah::form_stub* ref = nullptr;
      if (this->state.is_random_door) {
         ref = this->controls.ref->ref();
      } else {
         static_assert(
            !render_window_displays_teleport_markers,
            "TODO: If the Render Window can display teleport markers, then "
            "the View Teleport Marker button should be implemented!"
         );
      }
      if (ref) {
         dovahkit::subsystems::worldedit::core::get().center_on_refr(*ref);
      }
   }
   void teleport::_on_ref_picked(dovah::form_stub* ref) {
      this->state.ever_changed = true;
      if (!ref) {
         this->controls.buttons.view_marker->setEnabled(false);
         this->controls.buttons.view_ref->setEnabled(false);
         return;
      }
      if (this->state.is_random_door) {
         this->controls.buttons.view_marker->setEnabled(true);
         this->controls.buttons.view_ref->setEnabled(false);
      } else {
         this->controls.buttons.view_ref->setEnabled(true);
         if constexpr (render_window_displays_teleport_markers) {
            this->controls.buttons.view_marker->setEnabled(true);
         } else {
            this->controls.buttons.view_marker->setEnabled(false);
         }
      }
   }
   bool teleport::_validate_picked_ref(dovah::form_stub* ref) const {
      if (this->state.is_random_door) {
         return true;
      } else {
         if (!ref)
            return true;
         return _is_legal_teleport_destination(*ref);
      }
   }

   bool teleport::_is_legal_teleport_destination(dovah::form_stub& ref) const {
      auto* base = dovah::form_stub_helpers::get_base_form(&ref);
      if (!base || base->form_type != dovah::form_type::door) {
         return false;
      }

      auto loaded = ref.load().ptr_cast<loaded_form_type>();
      if (loaded) {
         auto* extra = loaded->extra_data.get<extra_data_types::teleport>();
         if (extra) {
            if (extra->target_door.get_form_stub() != this->stub) {
               //
               // The desired door is already a teleporter to somewhere else!
               //
               return false;
            }
         }
      }
      return true;
   }
   dovah::form_stub* teleport::_destination_of(const loaded_form_type& form) {
      if (auto* extra = form.extra_data.get<extra_data_type>()) {
         return extra->target_door.get_form_stub();
      }
      return nullptr;
   }
   void teleport::_disconnect_old_destination(dovah::form_stub& dst_stub) {
      auto loaded = dst_stub.load().ptr_cast<loaded_form_type>();
      if (!loaded)
         return;
      auto* extra = loaded->extra_data.get<extra_data_type>();
      if (!extra)
         return;
      if (extra->target_door == this->stub) {
         loaded->extra_data.remove<extra_data_type>(*loaded);
         dst_stub.set_edited(true);
      }
   }
   void teleport::_auto_place_teleport_marker(loaded_form_type& src_form, extra_data_type& src_extra, dovah::form_stub& dst_stub) {
      auto loaded = dst_stub.load().ptr_cast<loaded_form_type>();
      if (!loaded)
         return;
      if (this->state.ever_changed) {
         //
         // Try to automatically position this door's teleport marker in front of 
         // the destination door, and facing away from it.
         //
         auto coords = _calc_teleport_marker_position(*loaded);
         src_extra.position = coords.first;
         src_extra.rotation = coords.second;
      }
      //
      // Create teleport data on the destination door, so that the connection 
      // between these doors is bidirectional.
      //
      auto* extra = loaded->extra_data.get_or_create<extra_data_type>();
      if (extra->target_door != &src_form.stub) {
         extra->target_door.set(*loaded, &src_form.stub);
         //
         // Try to automatically position the destination's teleport marker in 
         // front of this door, and facing away from it.
         //
         auto coords = _calc_teleport_marker_position(src_form);
         extra->position = coords.first;
         extra->rotation = coords.second;
      }
   }
   
   #include <glm/glm.hpp>
   #include <glm/gtc/matrix_transform.hpp>
   #include <glm/gtx/euler_angles.hpp>
   #include "vulkan/helpers/glm_transform_from_beth.h"
   /*static*/ std::pair<cobb::vector3<float>, cobb::vector3<float>> teleport::_calc_teleport_marker_position(const loaded_form_type& in_front_of) {
      constexpr float distance = 128;

      std::pair<cobb::vector3<float>, cobb::vector3<float>> out;
      auto& [pos, rot] = out;

      pos = in_front_of.position;

      auto transform = vulkanDK::glm_transform_from_beth(in_front_of.position, in_front_of.rotation, 1);
      auto forward   = transform[0];
      pos += forward * distance;

      rot.x = 0;
      rot.y = 0;
      rot.z = atan2(forward.y, forward.x);

      static_assert(
         !render_window_displays_teleport_markers,
         "Hey, now that we can actually see teleport markers in the Render Window, "
         "maybe we should actually test this function and verify that it's producing "
         "correct results! Just a thought for later, yeah?"
      );

      return out;
   }

   teleport::loaded_form_type& teleport::loaded() {
      assert(this->stub != nullptr);
      return *this->stub->get_working_copy<loaded_form_type>();
   }
}