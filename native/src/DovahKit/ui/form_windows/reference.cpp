#include "./reference.h"
#include <QMessageBox>
#include "dovah/core.h"
#include "dovah/form_stubs/helpers/get_activator_water_type.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/message_log/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/typical_tableview_config.h"
#include "./reference/ObjectReferenceActivateParentsModel.h"
#include "./reference/ObjectReferenceLinkedFromModel.h"
#include "widgets/widget-dialogs/DKCompactObjectReferencePickerDialog.h"

#include "dovah/data/all_carryable_form_types.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/keyword_list.h"
#include "dovah/forms/components/model.h"
#include "dovah/forms/_component_access.h"
#include "dovah/utils/form_component_accessors/keyword_list.h"
#include "dovah/utils/get_computed_location.h"
#include "dovah/utils/location_is_or_is_inside_of_location.h"

namespace {
   constexpr bool render_window_displays_test_radii = false;
}

#include "dovah/forms/Cell.h"
#include "dovah/forms/DefaultObjectManager.h"
#include "dovah/forms/Light.h" // for dealing with ExtraTimeLeft
#pragma region Extra data includes
   #include "dovah/forms/components/extra_data/types/a/action.h"
   #include "dovah/forms/components/extra_data/types/a/activate_parents.h"
   #include "dovah/forms/components/extra_data/types/a/alpha_cutoff.h"
   #include "dovah/forms/components/extra_data/types/a/attach_ref.h"
   #include "dovah/forms/components/extra_data/types/c/charge.h"
   #include "dovah/forms/components/extra_data/types/c/count.h"
   #include "dovah/forms/components/extra_data/types/e/enable_state_parent.h"
   #include "dovah/forms/components/extra_data/types/e/encounter_zone.h"
   #include "dovah/forms/components/extra_data/types/f/favor_cost.h"
   #include "dovah/forms/components/extra_data/types/h/headtracking_weight.h"
   #include "dovah/forms/components/extra_data/types/h/health_percent.h"
   #include "dovah/forms/components/extra_data/types/h/horse.h"
   #include "dovah/forms/components/extra_data/types/i/ignored_by_sandbox.h"
   #include "dovah/forms/components/extra_data/types/l/leveled_creature_modifier.h"
   #include "dovah/forms/components/extra_data/types/l/leveled_item_base.h"
   #include "dovah/forms/components/extra_data/types/l/location.h"
   #include "dovah/forms/components/extra_data/types/l/location_ref_type.h"
   #include "dovah/forms/components/extra_data/types/m/multibound_bounds.h"
   #include "dovah/forms/components/extra_data/types/m/multibound_ref.h"
   #include "dovah/forms/components/extra_data/types/p/patrol_ref_data.h"
   #include "dovah/forms/components/extra_data/types/r/radius.h"
   #include "dovah/forms/components/extra_data/types/r/room_ref_data.h"
   #include "dovah/forms/components/extra_data/types/s/spawn_container.h"
   #include "dovah/forms/components/extra_data/types/t/time_left.h"
#pragma endregion

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }

   static bool can_rescale_ref(dovah::form_stub& ref) {
      auto* base = dovah::form_stub_helpers::get_base_form(&ref);
      if (!base)
         return true;

      // These refs are considered markers, but are still allowed to be scaled.
      if (base->form_type == dovah::form_type::texture_set)
         return true;
      if (base->formID == dovah::hardcoded_form_ids::WaterCurrentMarker)
         return true;

      // TESBoundObject::IsMarker
      bool is_marker = [base]() {
         if (base->test_record_flags(1 << 23))
            return true;
         switch (base->form_type) {
            case dovah::form_type::activator:
               {
                  auto loaded = base->load();
                  if (loaded)
                     if (auto* model = dovah::loaded_forms::component_access::get_model(loaded))
                        return model->model_path == "EditorMarker.NIF";
               }
               break;
            case dovah::form_type::light:
               {
                  auto loaded = base->load();
                  if (loaded)
                     if (auto* model = dovah::loaded_forms::component_access::get_model(loaded))
                        return model->model_path.empty();
               }
               break;
            case dovah::form_type::idle_marker:
            case dovah::form_type::texture_set:
               return true;
         }
         return false;
      }();

      // Markers are not allowed to be scaled (barring the exceptions further above).
      return !is_marker;
   }

   template<typename ExtraData> requires (!ExtraData::allowed_form_types.empty())
   void bind_single_form_extra_data(FormDialogObjectReference::loaded_form_type& working, DKFormPicker& picker) {
      if constexpr (ExtraData::allowed_form_types.size() == 1) {
         picker.setAllowedFormType(ExtraData::allowed_form_types[0]);
      } else {
         picker.setAllowedFormTypes(QList{ ExtraData::allowed_form_types.begin(), ExtraData::allowed_form_types.end() });
      }
      if (const auto* extra = working.exta_data.get<ExtraData>()) {
         picker.setFormStub(extra->form.get_form_stub());
      }
      QObject::connect(&picker, &DKFormPicker::formChanged, [&working](dovah::form_stub* stub) {
         if (stub) {
            auto* extra = working.extra_data.get_or_create<ExtraData>();
            extra->form.set(working, stub);
         } else {
            working.extra_data.remove<ExtraData>(working);
         }
      });
   }
   template<typename ExtraData, typename RefPickerWidget> requires (!ExtraData::allowed_form_types.empty())
   void bind_single_ref_extra_data(FormDialogObjectReference::loaded_form_type& working, RefPickerWidget& picker) {
      if (const auto* extra = working.exta_data.get<ExtraData>()) {
         picker.setRef(extra->form.get_form_stub());
      }
      QObject::connect(&picker, &RefPickerWidget::refChanged, [&working](dovah::form_stub* stub) {
         if (stub) {
            auto* extra = working.extra_data.get_or_create<ExtraData>();
            extra->form.set(working, stub);
         } else {
            working.extra_data.remove<ExtraData>(working);
         }
      });
   }
   
   template<typename ExtraData>
   void bind_fundamental_extra_data(FormDialogObjectReference::loaded_form_type& working, QDoubleSpinBox& widget) {
      if (const auto* extra = working.extra_data.get<ExtraData>()) {
         widget.setValue(extra->value);
      } else {
         widget.setValue(ExtraData::default_value);
      }
      QObject::connect(&widget, qOverload<double>(&QDoubleSpinBox::valueChanged), [&working](double v) {
         working.extra_data.get_or_create<ExtraData>().value = v;
      });
   }
   template<typename ExtraData>
   void bind_fundamental_extra_data(FormDialogObjectReference::loaded_form_type& working, QSpinBox& widget) {
      if (const auto* extra = working.extra_data.get<ExtraData>()) {
         widget.setValue(extra->value);
      } else {
         widget.setValue(ExtraData::default_value);
      }
      QObject::connect(&widget, qOverload<double>(&QSpinBox::valueChanged), [&working](int v) {
         working.extra_data.get_or_create<ExtraData>().value = v;
      });
   }
}

FormDialogObjectReference::FormDialogObjectReference(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.baseForm->setEnabled(false);

   #pragma region Basic Properties
      for (auto* spinbox : std::array{
         this->ui.positionX,
         this->ui.positionY,
         this->ui.positionZ,
         this->ui.rotationX,
         this->ui.rotationY,
         this->ui.rotationZ,
      }) {
         ui::set_range<float>(spinbox);
      }
      this->ui.scale->setDecimals(2);
      this->ui.scale->setRange(0.01, 10);
      this->ui.scaleSnap->setDecimals(2);
      this->ui.scaleSnap->setRange(0.01, 10);
      ui::set_unsigned_range<float>(this->ui.testRadius);
      if constexpr (!render_window_displays_test_radii) {
         this->ui.testRadiusLabel->setVisible(false);
         this->ui.testRadius->setVisible(false);
      }
      {
         using enumeration = extra_data_types::leveled_creature_modifier::difficulty;
         auto* widget = this->ui.xLevCreaMod;
         widget->clear();
         widget->addItem(tr("Easy"), (int)enumeration::easy);
         widget->addItem(tr("Medium"), (int)enumeration::normal);
         widget->addItem(tr("Hard"), (int)enumeration::hard);
         widget->addItem(tr("Very Hard"), (int)enumeration::very_hard);
         widget->addItem(tr("None"), (int)enumeration::none);
      }
   #pragma endregion
   #pragma region Extra
      this->ui.xAlphaCutoff->setRange(0, 255);
      ui::set_unsigned_range<float>(this->ui.xCharge);
      ui::set_unsigned_range<int>(this->ui.xCount);
      ui::set_unsigned_range<float>(this->ui.xFavorCost);
      ui::set_range<float>(this->ui.xHeadtrackingWeight);
      ui::set_unsigned_range<float>(this->ui.xHealth);
      this->ui.xHorse->setRequiredFormType(dovah::form_type::actor);
      ui::set_unsigned_range<float>(this->ui.xTimeLeft);
   #pragma endregion
   #pragma region Ownership
      this->fragments.ownership.setup(*this, {
         .owner_form = this->ui.owner,
         .owner_rank = this->ui.ownerRank,
      });
   #pragma endregion
   #pragma region Primitive
      this->fragments.primitive.setup(*this, {
         .color   = this->ui.xPrimitiveColor,
         .extents = {
            this->ui.xPrimitiveSizeX,
            this->ui.xPrimitiveSizeY,
            this->ui.xPrimitiveSizeZ,
         },
         .function = this->ui.xPrimitiveFunction,
         .layer    = this->ui.xPrimitiveCollLayer,
         .origin   = {
            this->ui.xPrimitiveCenterX,
            this->ui.xPrimitiveCenterY,
            this->ui.xPrimitiveCenterZ,
         },
         .player_activation = this->ui.xPrimitivePlayerActivation,
         .ref_position = {
            this->ui.positionX,
            this->ui.positionY,
            this->ui.positionZ,
         },
         .shape = this->ui.xPrimitiveShape,
      });
   #pragma endregion
   #pragma region Item
      this->ui.xLeveledItemBase->setAllowedFormType(dovah::form_type::leveled_item);
   #pragma endregion
   #pragma region Lock
      this->fragments.lock.setup(*this, {
         .groupbox   = this->ui.xLockGroupbox,
         .is_leveled = this->ui.xLockIsLeveled,
         .key        = this->ui.xLockKey,
         .level      = this->ui.xLockLevel,
      });
   #pragma endregion
   #pragma region Teleport
      this->fragments.teleport.setup(*this, {
         .buttons = {
            .view_marker = this->ui.buttonTeleportViewMarker,
            .view_ref    = this->ui.buttonTeleportViewLinkedDoor,
         },
         .groupbox = this->ui.xTeleportGroupbox,
         .name     = this->ui.xTeleportName,
         .ref      = this->ui.xTeleportDoor,
      });
   #pragma endregion
   #pragma region Map Marker
      this->fragments.map_marker.setup(*this, {
         .flags = {
            .can_be_fast_traveled_to   = this->ui.xMapMarkerCanTravel,
            .is_unaffected_by_show_all = this->ui.xMapMarkerShowAllHidden,
            .is_visible                = this->ui.xMapMarkerVisible,
         },
         .groupbox = this->ui.xMapMarkerGroupbox,
         .icon     = this->ui.xMapMarkerType,
         .name     = this->ui.xMapMarkerName,
         .radius   = this->ui.xMapMarkerRadius,
      });
   #pragma endregion
   #pragma region Patrol Data
      ui::set_unsigned_range<float>(this->ui.xPatrolIdleTime);
   #pragma endregion
   #pragma region Linked Refs
      this->fragments.linked_refs.setup(*this, {
         .buttons = {
            .add    = this->ui.buttonLinkedRefNew,
            .remove = this->ui.buttonLinkedRefDelete,
         },
         .edit = {
            .groupbox = this->ui.currentLinkedRefGroupbox,
            .keyword  = this->ui.currentLinkedRefKYWD,
            .ref      = this->ui.currentLinkedRefREFR,
         },
         .view = this->ui.linkedRefs
      });
   #pragma endregion
   #pragma region Linked From
   {
      using model_type = ObjectReferenceLinkedFromModel;

      auto* listview = this->ui.linkedFrom;
      auto* model    = this->models.linked_from = new model_type(listview);
      listview->setModel(model);
      ui::typical_tableview_config(listview);
   }
   #pragma endregion
   #pragma region Activate Parents
      {
         using model_type = ObjectReferenceActivateParentsModel;

         auto* listview = this->ui.activateParents;
         auto* model    = this->models.activate_parents = new model_type(listview);
         listview->setModel(model);
         ui::typical_tableview_config(listview);
         
         auto* sel_model = listview->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model, sel_model](const QItemSelection& sel) {
            if (sel.empty()) {
               this->ui.currentActivateParentGroupbox->setEnabled(false);
               return;
            }
            QModelIndex qmi = sel[0].topLeft();
            this->ui.currentActivateParentGroupbox->setEnabled(true);

            const auto blockers = std::array{
               QSignalBlocker(this->ui.currentActivateParentRef),
               QSignalBlocker(this->ui.currentActivateParentDelay),
            };
            this->ui.currentActivateParentDelay->setValue(
               model->data(qmi, model_type::DelayRole).value<float>()
            );
            this->ui.currentActivateParentRef->setRef(
               model->data(qmi, model_type::FormStubRole).value<dovah::form_stub*>()
            );
         });

         QObject::connect(this->ui.buttonActivateParentNew, &QPushButton::clicked, this, [this, model, sel_model]() {
            auto* dialog = new DKCompactObjectReferencePickerDialog(this);
            QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
            {
               auto all_refs = model->allRefs();
               dialog->setValidationFunction([all_refs](dovah::form_stub* ref) {
                  auto it = std::find(all_refs.begin(), all_refs.end(), ref);
                  if (it == all_refs.end())
                     return false;
                  return true;
               });
            }
            if (dialog->exec() == QDialog::Accepted) {
               auto* ref = dialog->value();
               if (ref) {
                  auto qmi = model->setRefDelay(*ref, 0);
                  if (qmi.isValid()) {
                     sel_model->select(
                        {
                           qmi.siblingAtColumn(0),
                           qmi.siblingAtColumn(model->columnCount({}) - 1)
                        },
                        QItemSelectionModel::SelectionFlag::ClearAndSelect
                     );
                  }
               }
            }
         });
         QObject::connect(this->ui.buttonActivateParentDelete, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto sel = sel_model->selectedRows();
            if (sel.empty())
               return;
            model->removeRow(sel[0].row());
         });

         auto* ref_picker = this->ui.currentActivateParentRef;
         auto* delay_edit = this->ui.currentActivateParentDelay;
         auto  on_changed = [ref_picker, delay_edit, sel_model, model]() {
            auto sel = sel_model->selectedRows();
            if (sel.empty())
               return;
            auto* ref = ref_picker->ref();
            if (!ref)
               return;
            model->setRow(sel[0].row(), *ref, delay_edit->value());
         };
         QObject::connect(ref_picker, &DKCompactObjectReferencePicker::refChanged, this, on_changed);
         QObject::connect(delay_edit, qOverload<double>(&QDoubleSpinBox::valueChanged), this, on_changed);
         ref_picker->setValidationFunction([this, model, ref_picker](dovah::form_stub* ref) -> bool {
            if (ref == ref_picker->ref())
               return true;
            if (!ref)
               return true;
            return !model->containsRef(*ref);
         });
      }
   #pragma endregion
   #pragma region Enable Parent
      this->ui.xEnableParentRef->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         return ref != &this->form->stub;
      });
   #pragma endregion
   #pragma region Lighting and Emittance
      #pragma region ExtraEmittanceSource
         this->fragments.emittance_source.setup(*this, {
            .none = {
               .radio = this->ui.xEmitTypeNONE,
            },
            .light = {
               .radio = this->ui.xEmitTypeLIGH,
               .form  = this->ui.xEmitLIGH,
            },
            .region = {
               .radio = this->ui.xEmitTypeREGN,
               .form = this->ui.xEmitREGN,
            },
         });
      #pragma endregion
      #pragma region Light properties
         this->fragments.light.setup(*this, {
            .depth_bias = {
               .reset   = this->ui.xLightDepthBiasReset,
               .slider  = this->ui.xLightDepthBiasSlider,
               .spinbox = this->ui.xLightDepthBiasSpinbox,
            },
            .end_cap = {
               .same_as_radius = this->ui.xLightEndDistanceSameAsRadius,
               .spinbox        = this->ui.xLightEndDistanceCap,
            },
            .fade = {
               .reset   = this->ui.xLightFadeReset,
               .spinbox = this->ui.xLightFade,
            },
            .flags = {
               .can_cast_shadows     = this->ui.flagCastShadows,
               .does_not_light_land  = this->ui.flagDoesntLightLandscape,
               .does_not_light_water = this->ui.flagDoesntLightWater,
               .never_fades          = this->ui.flagNeverFades,
            },
            .fov = {
               .reset   = this->ui.xLightFOVReset,
               .spinbox = this->ui.xLightFOV,
            },
            .groupbox = this->ui.xLightGroupbox,
            .radius = {
               .reset   = this->ui.xLightRadiusReset,
               .spinbox = this->ui.xLightRadius,
            },
         });
      #pragma endregion
   #pragma endregion
   #pragma region Water
      #pragma region Water Currents
         this->fragments.water_currents.setup(*this, {
            .groupbox = this->ui.xWaterCurrentsGroupbox,
            .velocity = {
               .angular = {
                  this->ui.xWaterCurrentsVelAngularX,
                  this->ui.xWaterCurrentsVelAngularY,
                  this->ui.xWaterCurrentsVelAngularZ
               },
               .angular_groupbox = this->ui.xWaterCurrentsVelAngular,
               .linear = {
                  this->ui.xWaterCurrentsVelLinearX,
                  this->ui.xWaterCurrentsVelLinearY,
                  this->ui.xWaterCurrentsVelLinearZ
               },
               .linear_groupbox = this->ui.xWaterCurrentsVelLinear,
            },
         });
      #pragma endregion
      #pragma region Reflected Refs
         this->fragments.reflected_refs.setup(*this, {
            .groupbox = this->ui.reflectedObjectsGroupbox,
            .view     = this->ui.reflectedObjects,
         });
      #pragma endregion
      #pragma region Water Lights
         this->fragments.water_lights.setup(*this, {
            .groupbox = this->ui.waterLightsGroupbox,
            .view     = this->ui.waterLights,
         });
      #pragma endregion
   #pragma endregion
   #pragma region Water (reflectee)
      this->fragments.water_reflectee.setup(*this, {
         .buttons  = {
            .add    = this->ui.xReflectorsButtonInsert,
            .remove = this->ui.xReflectorsButtonRemove,
         },
         .groupbox = this->ui.xReflectorsGroupbox,
         .view     = this->ui.xReflectors,
      });
   #pragma endregion
   #pragma region Rendering
      #pragma region Roombound Options
         this->ui.xRoomboundImagespace->setAllowedFormType(dovah::form_type::imagespace);
         this->ui.xRoomboundLightingTemplate->setAllowedFormType(dovah::form_type::lighting_template);
      #pragma endregion
   #pragma endregion

   this->load();
}
void FormDialogObjectReference::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->_emit_warnings_on_load();

   dovah::form_stub* base_form = working.base_form.get_form_stub();
   dovah::form_type  base_type = dovah::form_type::none;
   if (base_form)
      base_type = base_form->form_type;

   bool has_bounds = false;
   if (base_form) {
      switch (base_form->formID) {
         case dovah::hardcoded_form_ids::MultiBoundMarker:
         case dovah::hardcoded_form_ids::RoomMarker:
            has_bounds = true;
            break;
      }
   }
   
   const bool can_have_currents    = this->fragments.water_currents.can_have_currents(working);
   const bool is_a_primitive       = this->fragments.primitive.is_primitive(working);
   const bool is_a_water_activator = _is_water_activator();

   bool is_an_item = false;
   for (auto ft : dovah::all_carryable_form_types) {
      if (ft == base_type) {
         is_an_item = true;
         break;
      }
   }

   #pragma region Page visibility
   {
      auto* nav = this->ui.nav;
      nav->clear();

      auto add_page = [nav](QString label, QWidget* page) {
         auto* item = new QListWidgetItem(nav);
         item->setText(label);
         item->setData(Qt::UserRole, QVariant::fromValue(page));
         nav->addItem(item);
      };
      add_page(tr("Basic Properties", "page names"), this->ui.pageBasics);
      add_page(tr("Extra", "page names"), this->ui.pageExtra);
      add_page(tr("Ownership", "page names"), this->ui.pageOwnership);
      if (is_a_primitive) {
         add_page(tr("Primitive", "page names"), this->ui.pagePrimitive);
      }
      if (is_an_item) {
         add_page(tr("Item Properties", "page names"), this->ui.pageItem);
      }
      if (_can_be_a_patrol_marker()) {
         add_page(tr("Patrol Data", "page names"), this->ui.pagePatrolData);
      }
      if (this->fragments.lock.can_be_locked(working)) {
         add_page(tr("Lock", "page names"), this->ui.pageLock);
      }
      if (base_type == dovah::form_type::door) {
         add_page(tr("Teleport", "page names"), this->ui.pageTeleport);
      }
      if (this->fragments.map_marker.is_map_marker(working)) {
         add_page(tr("Map Marker", "page names"), this->ui.pageMapMarker);
      }
      add_page(tr("Linked Refs", "page names"), this->ui.pageLinkedRefs);
      add_page(tr("Linked From", "page names"), this->ui.pageLinkedFrom);
      add_page(tr("Activate Parents", "page names"), this->ui.pageActivateParents);
      add_page(tr("Enable State Parent", "page names"), this->ui.pageEnableParent);
      add_page(tr("Lighting and Emittance", "page names"), this->ui.pageEmittance);
      if (can_have_currents || is_a_water_activator) {
         add_page(tr("Water", "page names"), this->ui.pageWater);
      } else {
         add_page(tr("Water", "page names"), this->ui.pageWaterReflectee);
      }
      add_page(tr("Rendering", "page names"), this->ui.pageRendering);
      add_page(tr("Scripts", "page names"), this->ui.pageScripts);

      QObject::connect(nav->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, nav](const QItemSelection& sel) {
         if (sel.empty())
            return;
         auto  qmi    = sel[0].topLeft();
         auto* item   = nav->item(qmi.row());
         if (!item)
            return;
         auto* widget = item->data(Qt::UserRole).value<QWidget*>();
         if (!widget)
            return;
         this->ui.stack->setCurrentWidget(widget);
      });
      this->ui.stack->setCurrentWidget(this->ui.pageBasics);
   }
   #pragma endregion

   ui::bind(this->ui.editorID,  this->editor_id());
   #pragma region Basic Properties
      #pragma region World coordinates
         ui::bind(this->ui.positionX, working.position.x);
         ui::bind(this->ui.positionY, working.position.y);
         ui::bind(this->ui.positionZ, working.position.z);
         ui::bind(this->ui.rotationX, working.rotation.x);
         ui::bind(this->ui.rotationY, working.rotation.y);
         ui::bind(this->ui.rotationZ, working.rotation.z);
         if (can_rescale_ref(working.stub)) {  // Scale
            this->ui.scale->setEnabled(true);
            this->ui.scaleSnap->setEnabled(true);
            QObject::connect(this->ui.scale, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, &working](float v) {
               working.set_scale(v);

               // In case the value is clamped or rounded:
               const auto blocker = QSignalBlocker(this->ui.scale);
               this->ui.scale->setValue(working.get_scale());
            });
         } else {
            this->ui.scale->setEnabled(false);
            this->ui.scaleSnap->setEnabled(false);
         }
         {
            auto _hook_up_snap = [](QDoubleSpinBox* target, QDoubleSpinBox* snap) {
               snap->setValue(target->singleStep());
               QObject::connect(snap, qOverload<double>(&QDoubleSpinBox::valueChanged), target, &QDoubleSpinBox::setSingleStep);
            };
            _hook_up_snap(this->ui.positionX, this->ui.positionXSnap);
            _hook_up_snap(this->ui.positionY, this->ui.positionYSnap);
            _hook_up_snap(this->ui.positionZ, this->ui.positionZSnap);
            _hook_up_snap(this->ui.rotationX, this->ui.rotationXSnap);
            _hook_up_snap(this->ui.rotationY, this->ui.rotationYSnap);
            _hook_up_snap(this->ui.rotationZ, this->ui.rotationZSnap);
            _hook_up_snap(this->ui.scale, this->ui.scaleSnap);
         }
         static_assert(!render_window_displays_test_radii, "TODO: If the Render Window can display Test Radii, then we need to implement them.");
      #pragma endregion
      #pragma region Miscellaneous options
      {  // Flags
         using record_flag = loaded_form_type::form_flag;

         auto bind_typed_checkbox = [this, &working, base_type]<dovah::form_type Type, auto Mask>(QCheckBox* checkbox) {
            if (base_type == Type) {
               checkbox->setEnabled(true);
               ui::bind(checkbox, this->record_flags(), Mask);
            } else {
               checkbox->setEnabled(false);
            }
         };

         #pragma region Left column
            ui::bind(this->ui.flagDontHavokSettle, record_flags(), record_flag::dont_havok_settle);
            {
               auto*    widget = this->ui.flagHiddenFromLocalMap;
               uint32_t mask   = _get_hide_from_local_map_flags_mask();;
               if (mask) {
                  widget->setEnabled(true);
                  widget->setChecked(this->record_flags()& mask);
                  QObject::connect(widget, &QCheckBox::toggled, this, [this, &working](bool checked) {
                     uint32_t mask = _get_hide_from_local_map_flags_mask();
                     if (!mask)
                        return;
                     cobb::edit_bit(this->record_flags(), mask, checked);
                  });
               } else {
                  widget->setEnabled(false);
               }
            }
            {
               auto* widget = this->ui.flagIgnoredBySandbox;
               widget->setChecked(_get_ignored_by_sandbox());
               QObject::connect(widget, &QCheckBox::toggled, this, &FormDialogObjectReference::_set_ignored_by_sandbox);
            }
            bind_typed_checkbox.operator()<dovah::form_type::door, record_flag::inaccessible>(this->ui.flagDoorInaccessible);
            ui::bind(this->ui.flagDisabled, record_flags(), record_flag::disabled);
            {
               auto* widget = this->ui.flagIsFullLOD;
               switch (base_type) {
                  case dovah::form_type::actor_base:
                  case dovah::form_type::light:
                     widget->setEnabled(false);
                     break;
                  default:
                     ui::bind(widget, record_flags(), record_flag::is_full_lod);
                     break;
               }
            }
            bind_typed_checkbox.operator() <dovah::form_type::movable_static, record_flag::motion_blur>(this->ui.flagMotionBlur);
         #pragma endregion
         #pragma region Right column
            {
               auto* widget = this->ui.flagNoAIAcquire;
               if (is_an_item || base_type == dovah::form_type::container || base_type == dovah::form_type::actor_base) {
                  ui::bind(widget, record_flags(), record_flag::no_ai_acquire);
                  widget->setEnabled(true);
               } else {
                  widget->setEnabled(false);
               }
            }
            {
               auto* widget = this->ui.flagOpenByDefault;
               switch (base_type) {
                  case dovah::form_type::container:
                  case dovah::form_type::door:
                     widget->setEnabled(true);
                     if (auto* extra = working.extra_data.get<extra_data_types::action>()) {
                        widget->setChecked(extra->get_door_is_open_by_default());
                     } else {
                        widget->setChecked(false);
                     }
                     QObject::connect(widget, &QCheckBox::toggled, this, [this, &working](bool checked) {
                        auto* extra = working.extra_data.get_or_create<extra_data_types::action>();
                        extra->set_door_is_open_by_default(checked);
                     });
                     break;
                  default:
                     widget->setEnabled(false);
                     break;
               }
            }
            {
               auto* widget = this->ui.flagReflectedByAutoWater;
               if (base_type == dovah::form_type::actor_base) {
                  widget->setEnabled(false);
               } else {
                  widget->setEnabled(true);
                  ui::bind(widget, record_flags(), record_flag::reflected_by_auto_water);
               }
            }
            {
               auto* widget = this->ui.flagRespawns;
               if (_can_override_navmesh_gen()) {
                  widget->setEnabled(false);
               } else {
                  switch (base_type) {
                     case dovah::form_type::actor_base:
                     case dovah::form_type::container:
                        widget->setEnabled(false);
                        break;
                     default:
                        widget->setEnabled(true);
                        ui::bind_inverse(widget, record_flags(), record_flag::no_respawn);
                        break;
                  }
               }
            }
            bind_typed_checkbox.operator()<dovah::form_type::actor_base, record_flag::starts_dead>(this->ui.flagStartsDead);
            ui::bind(this->ui.flagStartsDead, record_flags(), record_flag::turn_off_fire);
         #pragma endregion
      }
      #pragma endregion
      bind_single_form_extra_data<extra_data_types::encounter_zone>    (working, *this->ui.encounterZone);
      bind_single_form_extra_data<extra_data_types::location>          (working, *this->ui.persistLocation);
      bind_single_form_extra_data<extra_data_types::location_ref_type> (working, *this->ui.locRefType);
      {
         using extra_data  = extra_data_types::leveled_creature_modifier;
         using enumeration = extra_data::difficulty;
         auto* widget = this->ui.xLevCreaMod;
         if (auto* extra = working.extra_data.get<extra_data>()) {
            widget->setCurrentIndex(widget->findData((int)extra->value));
         } else {
            widget->setCurrentIndex(widget->findData((int)enumeration::none));
         }
         QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [widget, &working]() {
            auto v = widget->currentData().toInt();
            working.extra_data.get_or_create<extra_data>()->value = (enumeration)v;
         });
      }
   #pragma endregion
   #pragma region Extra
      bind_fundamental_extra_data<extra_data_types::alpha_cutoff>(working, *this->ui.xAlphaCutoff);
      if (_can_have_attach_ref()) {
         bind_single_ref_extra_data<extra_data_types::attach_ref>(working, *this->ui.xAttachRef);
      } else {
         this->ui.xAttachRef->setEnabled(false);
      }
      {
         using extra_data = extra_data_types::charge;
         auto* enable = this->ui.xChargePresent;
         auto* editor = this->ui.xCharge;
         if (auto* extra = working.extra_data.get<extra_data>()) {
            enable->setChecked(true);
            editor->setEnabled(true);
            editor->setValue(extra->value);
         } else {
            enable->setChecked(false);
            editor->setEnabled(false);
         }
         QObject::connect(enable, &QCheckBox::toggled, [&working, editor](bool checked) {
            editor->setEnabled(checked);
            if (checked) {
               working.extra_data.get_or_create<extra_data>()->value = editor->value();
            } else {
               working.extra_data.remove<extra_data>(working);
            }
         });
         QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), [&working](double v) {
            working.extra_data.get_or_create<extra_data>()->value = v;
         });
      }
      {
         bind_fundamental_extra_data<extra_data_types::count>(working, *this->ui.xCount);
         this->ui.xCount->setEnabled(is_an_item);
      }
      bind_fundamental_extra_data<extra_data_types::favor_cost>(working, *this->ui.xFavorCost);
      bind_fundamental_extra_data<extra_data_types::headtracking_weight>(working, *this->ui.xHeadtrackingWeight);
      bind_fundamental_extra_data<extra_data_types::health_percent>(working, *this->ui.xHealth);
      {
         auto* widget = this->ui.xHorse;
         bind_single_ref_extra_data<extra_data_types::horse>(working, *widget);
         if (base_type != dovah::form_type::actor) {
            this->ui.xHorseLabel->setEnabled(false);
            widget->setEnabled(false);
         }
      }
      if (base_form && base_form->formID == dovah::hardcoded_form_ids::XMarkerHeading) {
         this->ui.flagIsSkyMarker->setEnabled(true);
         ui::bind(this->ui.flagIsSkyMarker, record_flags(), loaded_form_type::form_flag::is_sky_marker);
      } else {
         this->ui.flagIsSkyMarker->setEnabled(false);
      }
      {
         using extra_data = extra_data_types::time_left;
         auto* editor = this->ui.xTimeLeft;
         auto* reset  = this->ui.xTimeLeftReset;
         if (base_type == dovah::form_type::light) {
            reset->setEnabled(true);
            if (auto* extra = working.extra_data.get<extra_data>()) {
               editor->setValue(extra->value);
            } else {
               _reset_time_left();
            }
            QObject::connect(editor, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, &working](double v) {
               working.extra_data.get_or_create<extra_data>()->value = v;
            });
            QObject::connect(reset, &QPushButton::clicked, this, &FormDialogObjectReference::_reset_time_left);
         } else {
            reset->setEnabled(false);
            bind_fundamental_extra_data<extra_data_types::time_left>(working, *this->ui.xTimeLeft);
         }
      }
      //
      // Not extra-data but I don't have a better place to stash it:
      //
      {
         QGroupBox* groupbox = this->ui.navmeshGenOverride;
         auto*      navmesh  = this->ui.navmeshGen;
         if (_can_override_navmesh_gen()) {
            using ui_value_type = DKNavmeshGenerationImportOptionPicker::Value;

            constexpr const auto all_navmesh_flags = (
               loaded_form_type::form_flag::navmesh_generation_obb |
               loaded_form_type::form_flag::navmesh_generation_filter |
               loaded_form_type::form_flag::navmesh_generation_ground
            );
            constexpr const auto flags_for_collision_geo = (
               loaded_form_type::form_flag::navmesh_generation_filter |
               loaded_form_type::form_flag::navmesh_generation_ground
            );

            groupbox->setEnabled(true);
            if (record_flags() & all_navmesh_flags) {
               groupbox->setChecked(true);
               uint32_t mask = record_flags() & all_navmesh_flags;
               if (mask == flags_for_collision_geo) {
                  navmesh->setValue(ui_value_type::CollisionGeometry);
               } else {
                  navmesh->setValueByMask(mask);
               }
            } else {
               groupbox->setChecked(false);
            }

            QObject::connect(navmesh, &DKNavmeshGenerationImportOptionPicker::valueChanged, this, [this](ui_value_type v) {
               record_flags() &= ~all_navmesh_flags;
               if (v == ui_value_type::CollisionGeometry)
                  record_flags() |= flags_for_collision_geo;
               else
                  record_flags() |= (uint32_t)v;
            });
         } else {
            groupbox->setEnabled(false);
            groupbox->setChecked(false);
         }
      }
   #pragma endregion
   this->fragments.ownership.load(working);
   this->fragments.primitive.load(working);
   #pragma region Item Options
      bind_single_form_extra_data<extra_data_types::leveled_item_base>(working, *this->ui.xLeveledItemBase);
      bind_single_ref_extra_data<extra_data_types::spawn_container>(working, *this->ui.xSpawnContainer);
   #pragma endregion
   this->fragments.lock.load(working);
   this->fragments.teleport.load(working);
   _load_patrol();
   this->fragments.map_marker.load(working);
   this->fragments.linked_refs.load(working);
   #pragma region Linked From
      this->models.linked_from->setSubject(&working.stub);
   #pragma endregion
   #pragma region Activate Parents
      this->models.activate_parents->importData(working);
   #pragma endregion
   #pragma region Enable Parent
   {
      using extra_data = extra_data_types::enable_state_parent;
      if (auto* extra = working.extra_data.get<extra_data>()) {
         this->ui.xEnableParentRef->setRef(extra->ref.get_form_stub());
         this->ui.xEnableParentOpposite->setChecked(extra->flags & extra_data::flag::opposite);
         this->ui.xEnableParentPopIn->setChecked(extra->flags & extra_data::flag::pop_in);
      }
   }
   #pragma endregion
   #pragma region Lighting and Emittance
      this->fragments.emittance_source.load(working);
      this->fragments.light.load(working, record_flags());
   #pragma endregion
   #pragma region Water
      this->fragments.water_currents.load(working);
      this->fragments.reflected_refs.load(working);
      this->fragments.water_lights.load(working);
   #pragma endregion
   this->fragments.water_reflectee.load(working);
   #pragma region Rendering
      #pragma region Override multibound ref
         bind_single_ref_extra_data<extra_data_types::multibound_ref>(working, *this->ui.xMultiboundRef);
      #pragma endregion
      #pragma region Roombound Options
         if (_is_roombound()) {
            this->ui.xRoomboundGroupbox->setEnabled(true);
            if (auto* extra = working.extra_data.get<extra_data_types::room_ref_data>()) {
               this->ui.xRoomboundImagespace->setFormStub(extra->imagespace.get_form_stub());
               this->ui.xRoomboundLightingTemplate->setFormStub(extra->lighting_template.get_form_stub());
            }
         } else {
            this->ui.xRoomboundGroupbox->setEnabled(false);
         }
      #pragma endregion
   #pragma endregion
   #pragma region Scripts
      this->ui.scriptListPane->setFormWorkingCopy(&working);
   #pragma endregion
}
void FormDialogObjectReference::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   dovah::form_stub* base_form = working.base_form.get_form_stub();
   dovah::form_type  base_type = dovah::form_type::none;
   if (base_form)
      base_type = base_form->form_type;

   if (record_flags() & loaded_form_type::form_flag::is_full_lod) {
      //
      // Refs placed in exterior cells need to be flagged as persistent in order 
      // to display with "full LOD." The CK does this automatically, the instant 
      // you tick the "Is Full LOD" box. We'll defer it until you click "OK."
      //
      if ((record_flags() & loaded_form_type::form_flag::persistent) == 0) {
         auto* parent_form = this->form->stub.get_parent_form();
         if (parent_form && parent_form->is_exterior_cell()) {
            record_flags() |= loaded_form_type::form_flag::persistent;
            //
            // And inform the user that we've flagged the form thus.
            //
            QString descriptor;
            if (editor_id().empty()) {
               if (base_form) {
                  descriptor = tr("%1 (base %2)")
                     .arg(editor_helpers::form_id_to_string(this->form->stub.formID))
                     .arg(QString::fromStdString(base_form->editorID));
               } else {
                  descriptor = editor_helpers::form_id_to_string(this->form->stub.formID);
               }
            } else {
               descriptor = QString::fromStdString(editor_id());
            }
            auto& logger = dovahkit::subsystems::message_log::core::get();
            logger.addLogItem(ui::types::log_item(
               tr(
                  "Reference %1 is being flagged as \"Is Full LOD.\" Because the reference is "
                  "placed in an exterior cell, it must also be flagged as \"Persistent\" for "
                  "the \"Is Full LOD\" setting to work. DovahKit just made the ref persistent "
                  "for you."
               ).arg(descriptor),
               ui::types::log_item_type::message,
               ui::types::log_item_context::unspecified
            ));
         }
      }
   }

   this->fragments.ownership.save(working);
   this->fragments.primitive.save(working);
   this->fragments.lock.save(working);
   this->fragments.teleport.save(working);
   _save_patrol();
   this->fragments.map_marker.save(working);
   this->fragments.linked_refs.save(working);
   #pragma region Activate Parents
      this->models.activate_parents->exportData(working);
      {
         using extra_data = extra_data_types::activate_parents;

         bool parent_only = this->ui.flagOnlyAllowActivateViaParent->isChecked();
         if (parent_only) {
            auto* extra = working.extra_data.get_or_create<extra_data>();
            extra->flags |= extra_data::flag::parent_activate_only;
         } else {
            if (auto* extra = working.extra_data.get<extra_data>())
               extra->flags &= ~extra_data::flag::parent_activate_only;
         }
      }
   #pragma endregion
   #pragma region Enable Parent
   {
      using extra_data = extra_data_types::enable_state_parent;
      auto* ref = this->ui.xEnableParentRef->ref();
      if (ref) {
         auto* extra = working.extra_data.get_or_create<extra_data>();
         extra->ref.set(working, ref);
         cobb::edit_bit(extra->flags, extra_data::flag::opposite, this->ui.xEnableParentOpposite->isChecked());
         cobb::edit_bit(extra->flags, extra_data::flag::pop_in,   this->ui.xEnableParentPopIn->isChecked());
      } else {
         working.extra_data.remove<extra_data>(working);
      }
   }
   #pragma endregion
   #pragma region Lighting and Emittance
      this->fragments.emittance_source.save(working);
      this->fragments.light.save(working, record_flags());
   #pragma endregion
   #pragma region Water
      this->fragments.water_currents.save(working);
      this->fragments.reflected_refs.load(working);
      this->fragments.water_lights.load(working);
   #pragma endregion
   this->fragments.water_reflectee.save(working);
   #pragma region Rendering
      #pragma region Override multibound ref
      {
         using extra_data = extra_data_types::multibound_ref;
         if (auto* ref = this->ui.xMultiboundRef->ref()) {
            working.extra_data.get_or_create<extra_data>()->form.set(working, ref);
         } else {
            working.extra_data.remove<extra_data>(working);
         }
      }
      #pragma endregion
      #pragma region Roombound Options
         if (_is_roombound()) {
            auto* extra = working.extra_data.get_or_create<extra_data_types::room_ref_data>();
            extra->imagespace.set(working, this->ui.xRoomboundImagespace->formStub());
            extra->lighting_template.set(working, this->ui.xRoomboundLightingTemplate->formStub());
         } else {
            working.extra_data.remove<extra_data_types::room_ref_data>(working);
         }
      #pragma endregion
   #pragma endregion
   #pragma region Scripts
      this->ui.scriptListPane->commit();
   #pragma endregion
}

void FormDialogObjectReference::_emit_warnings_on_load() {
   auto& working = *this->form;
   auto& logger  = dovahkit::subsystems::message_log::core::get();

   auto* base_form = working.base_form.get_form_stub();

   //
   // If the ref has a Persist Location, warn if its parent cell isn't part of 
   // that location.
   //
   [&working, &logger]() {
      auto* extra = working.extra_data.get<extra_data_types::location>();
      if (!extra)
         return;
      dovah::form_stub* persist_loc = extra->form.get_form_stub();
      if (!persist_loc || persist_loc->form_type != dovah::form_type::location)
         return;

      dovah::form_stub* computed_loc = nullptr;
      dovah::form_stub* failed_loc   = nullptr;
      [&working, &computed_loc, &failed_loc]() {
         auto* parent_cell = working.stub.get_parent_form();
         if (!parent_cell || parent_cell->form_type != dovah::form_type::cell)
            return;
         auto loaded_cell = parent_cell->load().ptr_cast<dovah::loaded_forms::Cell>();
         if (!loaded_cell)
            return;
         computed_loc = dovah::utils::get_computed_location(*loaded_cell);
         if (auto* extra = loaded_cell->extra_data.get<extra_data_types::location>()) {
            failed_loc = extra->form.get_form_stub();
            if (failed_loc && failed_loc->form_type != dovah::form_type::location)
               failed_loc = nullptr;
         }
      }();
      if (computed_loc == persist_loc)
         return;
      QString message;
      if (failed_loc) {
         message = tr(
            "Ref %1 is not in its persist location %2. (Parent cell %3 attempts to tag itself with "
            "that location, but also tags itself with an encounter zone. An encounter zone's location "
            "overrides the locations of cells and worldspaces belonging to the zone.)"
         )
            .arg(editor_helpers::form_identifiers_to_string(&working.stub))
            .arg(editor_helpers::form_identifiers_to_string(persist_loc))
            .arg(editor_helpers::form_identifiers_to_string(failed_loc))
         ;
      } else {
         message = tr("Ref %1 is not in its persist location %2.")
            .arg(editor_helpers::form_identifiers_to_string(&working.stub))
            .arg(editor_helpers::form_identifiers_to_string(persist_loc))
         ;
      }
      logger.addLogItem(ui::types::log_item(
         message,
         ui::types::log_item_type::warning,
         ui::types::log_item_context::form_load
      ));
   }();

   //
   // If the ref is a dummy item, warn if it sets no Leveled Item Base.
   //
   [&working, &logger, &base_form]() {
      if (!base_form)
         return;
      auto* keywords = dovah::utils::form_component_accessors::keyword_list(*base_form);
      if (!keywords)
         return;

      const auto* dummy_keyword = []() -> dovah::form_stub* {
         auto& editor   = DovahKitCore::get();
         auto* dobj_man = editor.get_singleton_form(dovah::form_type::default_object_manager);
         if (!dobj_man)
            return nullptr;
         auto loaded = dobj_man->load().ptr_cast<dovah::loaded_forms::DefaultObjectManager>();
         if (loaded)
            return loaded->get_entry('KWDM');
         return nullptr;
      }();
      if (!dummy_keyword)
         return;

      bool is_dummy = false;
      for (auto& use : keywords->forms) {
         if (use == dummy_keyword) {
            is_dummy = true;
            break;
         }
      }
      if (is_dummy) {
         auto* extra = working.extra_data.get<extra_data_types::leveled_item_base>();
         if (!extra || !extra->form) {
            logger.addLogItem(ui::types::log_item(
               tr("Ref %1 is a dummy object but has no Leveled Item Base. The ref will never be visible in-game."),
               ui::types::log_item_type::warning,
               ui::types::log_item_context::form_load
            ));
         }
      }
   }();

   this->fragments.primitive.issue_initial_warnings(working);
   this->fragments.teleport.issue_initial_warnings(working);
}

uint32_t FormDialogObjectReference::_get_hide_from_local_map_flags_mask() const {
   auto* base = this->form->base_form.get_form_stub();
   if (!base)
      return 0;
   switch (base->form_type) {
      case dovah::form_type::door:
         return loaded_form_type::form_flag::hide_from_local_map_a;
      case dovah::form_type::activator:
      case dovah::form_type::statik:
      case dovah::form_type::tree:
         return loaded_form_type::form_flag::hide_from_local_map_b;
   }
   return 0;
}

#pragma region Patrol
   bool FormDialogObjectReference::_can_be_a_patrol_marker() const {
      dovah::form_stub* base_form = this->form->base_form.get_form_stub();
      if (!base_form)
         return false;
      switch (base_form->form_type) {
         case dovah::form_type::furniture:
         case dovah::form_type::idle_marker:
            return true;
      }
      switch (base_form->formID) {
         case dovah::hardcoded_form_ids::XMarker:
         case dovah::hardcoded_form_ids::XMarkerHeading:
            return true;
      }
      return false;
   }
   void FormDialogObjectReference::_load_patrol() {
      if (!_can_be_a_patrol_marker())
         return;
      auto& working = *this->form;
      if (auto* extra = working.extra_data.get<extra_data_types::patrol_ref_data>()) {
         this->ui.xPatrolGroupbox->setChecked(true);
         this->ui.xPatrolIdleTime->setValue(extra->idle_time);
         if (extra->event.type == dovah::loaded_forms::components::package_event_dialogue::topic_type::ref) {
            this->ui.xPatrolTopic->setTopic(extra->event.topic.get_form_stub());
         } else {
            this->ui.xPatrolTopic->setSubtype(extra->event.topic_subtype);
         }
      } else {
         this->ui.xPatrolGroupbox->setChecked(false);
      }
   }
   void FormDialogObjectReference::_save_patrol() {
      auto& working = *this->form;
      if (this->ui.xPatrolGroupbox->isChecked()) {
         auto* extra = working.extra_data.get_or_create<extra_data_types::patrol_ref_data>();
         extra->idle_time = this->ui.xPatrolIdleTime->value();
         if (auto* form = this->ui.xPatrolTopic->topic()) {
            extra->event.topic.set(working, form);
            extra->event.type = dovah::loaded_forms::components::package_event_dialogue::topic_type::ref;
         } else {
            extra->event.topic.set(working, nullptr);
            extra->event.topic_subtype = this->ui.xPatrolTopic->subtype();
            extra->event.type = dovah::loaded_forms::components::package_event_dialogue::topic_type::subtype;
         }
      } else {
         working.extra_data.remove<extra_data_types::patrol_ref_data>(working);
      }
   }
#pragma endregion

bool FormDialogObjectReference::_can_override_navmesh_gen() const noexcept {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;
   switch (base_form->form_type) {
      case dovah::form_type::activator:
      case dovah::form_type::container:
      case dovah::form_type::movable_static:
      case dovah::form_type::statik:
      case dovah::form_type::static_collection:
         return true;
   }
   return false;
}
bool FormDialogObjectReference::_is_roombound() const noexcept {
   auto* base = this->form->base_form.get_form_stub();
   if (!base)
      return false;
   return base->formID == dovah::hardcoded_form_ids::RoomMarker;
}

bool FormDialogObjectReference::_get_ignored_by_sandbox() const {
   auto& working = *this->form;
   return !!working.extra_data.get<extra_data_types::ignored_by_sandbox>();
}
void FormDialogObjectReference::_set_ignored_by_sandbox(bool ignored) {
   auto& working = *this->form;
   if (ignored)
      working.extra_data.get_or_create<extra_data_types::ignored_by_sandbox>();
   else
      working.extra_data.remove<extra_data_types::ignored_by_sandbox>(working);
}

void FormDialogObjectReference::_reset_time_left() {
   using extra_data = extra_data_types::time_left;
   auto& working = *this->form;
   auto* widget  = this->ui.xTimeLeft;

   working.extra_data.remove<extra_data>(working);

   const auto blocker = QSignalBlocker(widget);
   if (auto* base = working.base_form.get_form_stub()) {
      auto loaded = base->load().ptr_cast<dovah::loaded_forms::Light>();
      if (loaded) {
         widget->setValue(loaded->time);
         return;
      }
   }
   widget->setValue(0);
}

bool FormDialogObjectReference::_can_have_attach_ref() const {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;
   switch (base_form->form_type) {
      case dovah::form_type::activator:
      case dovah::form_type::container:
      case dovah::form_type::door:
      case dovah::form_type::flora:
      case dovah::form_type::movable_static:
      case dovah::form_type::statik:
      case dovah::form_type::tree:
         return true;
   }
   return false;
}
bool FormDialogObjectReference::_is_water_activator() const {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;
   return dovah::form_stub_helpers::get_activator_water_type(*base_form) != nullptr;
}
