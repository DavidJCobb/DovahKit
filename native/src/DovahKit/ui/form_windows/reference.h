#pragma once
#include <cstdint>
#include <utility> // std::pair
#include <QDialog>
#include "helpers/vector3.h"
#include "./_base.h"
#include "dovah/data/collision_layers.h"
#include "dovah/forms/ObjectReference.h"
#include "ui_reference.h"
class ObjectReferenceActivateParentsModel;
class ObjectReferenceLinkedRefsModel;

class FormDialogObjectReference :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::ObjectReference, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogObjectReference(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogObjectReference ui;
      struct {
         struct {
            dovah::collision_layer prior_layer = dovah::collision_layer::null;
         } primitive;
         struct {
            // Used so we know if we need to recalculate the teleport marker 
            // position on save.
            bool ever_changed = false;

            // Used if the user picks an invalid ref, to undo their choice.
            dovah::form_stub* prior_destination = nullptr;
         } teleport;
      } state;
      struct {
         ObjectReferenceActivateParentsModel* activate_parents = nullptr;
         ObjectReferenceLinkedRefsModel*      linked_refs      = nullptr;
      } models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      template<dovah::form_type FT>
      dovah::form_stub* _base_stub_of_type() const {
         auto* base = this->form->base_form.get_form_stub();
         if (base && base->form_type == FT)
            return base;
         return nullptr;
      }

      template<typename Loaded>
      dovah::loaded_form_ptr<Loaded> _base_loaded_as_type() const {
         if (auto* stub = _base_stub_of_type<Loaded::form_type>())
            return stub->load().ptr_cast<Loaded>();
         return {};
      }

      uint32_t _get_hide_from_local_map_flags_mask() const;

      #pragma region Lock
         bool _can_be_locked() const noexcept;
         void _load_lock();
         void _save_lock();
      #pragma endregion
      #pragma region Patrol
         bool _can_be_a_patrol_marker() const;
         void _load_patrol();
         void _save_patrol();
      #pragma endregion
      #pragma region Primitive
         bool _is_primitive() const noexcept;
         bool _can_change_primitive_shape() const;

         QString _primitive_type(const dovah::form_stub* base_form) const;
         void _load_primitive();
         void _save_primitive();
         //
         void _on_primitive_collision_layer_changed();
         void _on_primitive_player_activation_toggled(bool);
      #pragma endregion

      bool _can_override_navmesh_gen() const noexcept;
      bool _is_roombound() const noexcept;

      bool _get_ignored_by_sandbox() const;
      void _set_ignored_by_sandbox(bool);

      void _reset_time_left();
      void _update_ownership_rank_picker();

      bool _can_have_attach_ref() const;
      bool _can_have_water_currents() const;
      bool _is_legal_teleport_destination(dovah::form_stub&) const;
      static std::pair<cobb::vector3<float>, cobb::vector3<float>> _calc_teleport_marker_position(const loaded_form_type& in_front_of);
};
