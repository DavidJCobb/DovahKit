#pragma once
#include <cstdint>
#include <QDialog>
#include "./_base.h"
#include "dovah/forms/ObjectReference.h"
#include "ui_reference.h"
#include "./reference/fragments/emittance_source.h"
#include "./reference/fragments/light.h"
#include "./reference/fragments/linked_refs.h"
#include "./reference/fragments/lock.h"
#include "./reference/fragments/map_marker.h"
#include "./reference/fragments/ownership.h"
#include "./reference/fragments/primitive.h"
#include "./reference/fragments/reflected_refs.h"
#include "./reference/fragments/teleport.h"
#include "./reference/fragments/water_currents.h"
#include "./reference/fragments/water_lights.h"
#include "./reference/fragments/water_reflectee.h"
class ObjectReferenceActivateParentsModel;
class ObjectReferenceLinkedFromModel;

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
         ui::reference::fragments::emittance_source emittance_source;
         ui::reference::fragments::light            light;
         ui::reference::fragments::linked_refs      linked_refs;
         ui::reference::fragments::lock             lock;
         ui::reference::fragments::map_marker       map_marker;
         ui::reference::fragments::ownership        ownership;
         ui::reference::fragments::primitive        primitive;
         ui::reference::fragments::reflected_refs   reflected_refs;
         ui::reference::fragments::teleport         teleport;
         ui::reference::fragments::water_currents   water_currents;
         ui::reference::fragments::water_lights     water_lights;
         ui::reference::fragments::water_reflectee  water_reflectee;
      } fragments;
      struct {
         ObjectReferenceActivateParentsModel*  activate_parents  = nullptr;
         ObjectReferenceLinkedFromModel*       linked_from       = nullptr;
      } models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      // Validation steps that require checking data across multiple loaded forms.
      // I didn't want to jam these into ObjectReference::_load_impl. I don't much 
      // like the idea that loading one form could automatically load another; I 
      // don't want to have to worry about cycles.
      void _emit_warnings_on_load();

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

      #pragma region Patrol
         bool _can_be_a_patrol_marker() const;
         void _load_patrol();
         void _save_patrol();
      #pragma endregion

      bool _can_override_navmesh_gen() const noexcept;
      bool _is_roombound() const noexcept;

      bool _get_ignored_by_sandbox() const;
      void _set_ignored_by_sandbox(bool);

      void _reset_time_left();

      bool _can_have_attach_ref() const;
      bool _is_water_activator() const;
};
