#include "./reference.h"
#include <QMessageBox>
#include "dovah/core.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/localize/collision_layer.h"
#include "editor/subsystems/message_log/core.h"
#include "editor/subsystems/worldedit/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/typical_tableview_config.h"
#include "./reference/ObjectReferenceActivateParentsModel.h"
#include "./reference/ObjectReferenceLinkedRefsModel.h"
#include "./reference/ObjectReferenceNewLinkedRefDialog.h"
#include "widgets/widget-dialogs/DKCompactObjectReferencePickerDialog.h"

#include "dovah/data/all_carryable_form_types.h"
#include "dovah/data/collision_layers.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/model.h"
#include "dovah/forms/_component_access.h"
#include "dovah/utils/default_light_emitter_shadow_depth_bias.h"
#include "dovah/utils/default_primitive_color_for_base_form.h"

namespace {
   constexpr bool render_window_displays_teleport_markers = false;
   constexpr bool render_window_displays_test_radii       = false;
}

#include "dovah/forms/Faction.h"
#include "dovah/forms/Light.h"
#pragma region Extra data includes
   #include "dovah/forms/components/extra_data/types/a/action.h"
   #include "dovah/forms/components/extra_data/types/a/activate_parents.h"
   #include "dovah/forms/components/extra_data/types/a/alpha_cutoff.h"
   #include "dovah/forms/components/extra_data/types/a/attach_ref.h"
   #include "dovah/forms/components/extra_data/types/c/charge.h"
   #include "dovah/forms/components/extra_data/types/c/collision_data.h"
   #include "dovah/forms/components/extra_data/types/c/count.h"
   #include "dovah/forms/components/extra_data/types/e/emittance_source.h"
   #include "dovah/forms/components/extra_data/types/e/enable_state_parent.h"
   #include "dovah/forms/components/extra_data/types/e/encounter_zone.h"
   #include "dovah/forms/components/extra_data/types/f/favor_cost.h"
   #include "dovah/forms/components/extra_data/types/h/headtracking_weight.h"
   #include "dovah/forms/components/extra_data/types/h/health_percent.h"
   #include "dovah/forms/components/extra_data/types/h/horse.h"
   #include "dovah/forms/components/extra_data/types/i/ignored_by_sandbox.h"
   #include "dovah/forms/components/extra_data/types/l/leveled_creature_modifier.h"
   #include "dovah/forms/components/extra_data/types/l/leveled_item_base.h"
   #include "dovah/forms/components/extra_data/types/l/light.h"
   #include "dovah/forms/components/extra_data/types/l/linked_ref.h"
   #include "dovah/forms/components/extra_data/types/l/location.h"
   #include "dovah/forms/components/extra_data/types/l/location_ref_type.h"
   #include "dovah/forms/components/extra_data/types/l/lock.h"
   #include "dovah/forms/components/extra_data/types/m/map_marker.h"
   #include "dovah/forms/components/extra_data/types/m/multibound_ref.h"
   #include "dovah/forms/components/extra_data/types/o/ownership.h"
   #include "dovah/forms/components/extra_data/types/p/primitive.h"
   #include "dovah/forms/components/extra_data/types/r/radius.h"
   #include "dovah/forms/components/extra_data/types/r/rank.h"
   #include "dovah/forms/components/extra_data/types/r/room_ref_data.h"
   #include "dovah/forms/components/extra_data/types/s/spawn_container.h"
   #include "dovah/forms/components/extra_data/types/t/teleport.h"
   #include "dovah/forms/components/extra_data/types/t/teleport_name.h"
   #include "dovah/forms/components/extra_data/types/t/time_left.h"
   #include "dovah/forms/components/extra_data/types/w/water_current_zone_data.h"
   #include "dovah/forms/components/extra_data/types/w/water_data.h"
#pragma endregion

namespace {
   namespace extra_data_types {
      using namespace dovah::loaded_forms::components::extra_data_types;
   }

   constexpr const dovah::collision_layer layer_for_player_activate_primitives = dovah::collision_layer::non_collidable;

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
      this->ui.owner->setAllowedFormTypes({ dovah::form_type::actor_base, dovah::form_type::faction });
   #pragma endregion
   #pragma region Primitive
      {
         using extra_data = extra_data_types::primitive;
         using shape_type = enum extra_data::shape;
         QComboBox* widget = this->ui.xPrimitiveShape;
         widget->clear();
         widget->addItem(tr("None"), (int)shape_type::none);
         widget->addItem(tr("Box"), (int)shape_type::box);
         widget->addItem(tr("Plane"), (int)shape_type::portal_box);
         widget->addItem(tr("Sphere"), (int)shape_type::sphere);
         widget->addItem(tr("Line"), (int)shape_type::line);
      }
      {
         QComboBox* widget = this->ui.xPrimitiveCollLayer;
         widget->clear();
         for (auto cl : dovah::all_collision_layers) {
            widget->addItem(
               editor::localize::collision_layer(cl),
               (int)cl
            );
         }
      }
   #pragma endregion
   #pragma region Item
      this->ui.xLeveledItemBase->setAllowedFormType(dovah::form_type::leveled_item);
   #pragma endregion
   #pragma region Lock
      {
         auto* widget = this->ui.xLockLevel;
         widget->clear();
         widget->addItem(tr("Novice", "lock levels"), 1);
         widget->addItem(tr("Apprentice", "lock levels"), 25);
         widget->addItem(tr("Adept", "lock levels"), 50);
         widget->addItem(tr("Expert", "lock levels"), 75);
         widget->addItem(tr("Master", "lock levels"), 100);
         widget->addItem(tr("Requires Key", "lock levels"), 255);
      }
      this->ui.xLockKey->setAllowedFormType(dovah::form_type::key);
   #pragma endregion
   #pragma region Teleport
      if constexpr (!render_window_displays_teleport_markers) {
         this->ui.buttonTeleportViewMarker->setEnabled(false);
      }
      this->ui.xTeleportName->setAllowedFormType(dovah::form_type::message);
   #pragma endregion
   #pragma region Reflected By
      this->ui.reflectedBy->setReadOnly(true);
   #pragma endregion
   #pragma region Linked Refs
      this->ui.currentLinkedRefKYWD->setAllowedFormType(dovah::form_type::keyword);
      {
         using model_type = ObjectReferenceLinkedRefsModel;

         auto* listview = this->ui.linkedRefs;
         auto* model    = this->models.linked_refs = new model_type(listview);
         listview->setModel(model);
         ui::typical_tableview_config(listview);

         auto* sel_model = listview->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, sel_model](const QItemSelection& sel) {
            bool no_selection = sel.empty();
            this->ui.currentLinkedRefGroupbox->setDisabled(no_selection);
            if (no_selection)
               return;
            QModelIndex qmi = sel[0].topLeft();

            const auto blockers = std::array{
               QSignalBlocker(this->ui.currentLinkedRefKYWD),
               QSignalBlocker(this->ui.currentLinkedRefREFR),
            };
            this->ui.currentLinkedRefKYWD->setFormStub(qmi.data(model_type::KeywordRole).value<dovah::form_stub*>());
            this->ui.currentLinkedRefREFR->setRef(     qmi.data(model_type::RefRole).value<dovah::form_stub*>());
         });

         QObject::connect(this->ui.buttonLinkedRefNew, &QPushButton::clicked, this, [this, model]() {
            auto* dialog = new ObjectReferenceNewLinkedRefDialog(this);
            QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
            dialog->setDisallowedKeywords(model->allKeywords());
            dialog->setDisallowedRef(this->form->stub);
            if (dialog->exec() == QDialog::Accepted) {
               auto* keyword = dialog->keyword();
               auto* ref     = dialog->ref();
               if (ref) {
                  model->setLink(keyword, ref);
               }
            }
         });
         QObject::connect(this->ui.buttonLinkedRefDelete, &QPushButton::clicked, this, [this, sel_model, model]() {
            auto sel = sel_model->selectedRows();
            if (sel.empty())
               return;
            auto  qmi  = sel[0];
            auto* kywd = qmi.data(model_type::KeywordRole).value<dovah::form_stub*>();
            model->setLink(kywd, nullptr);
         });
         
         auto* ref_picker     = this->ui.currentLinkedRefREFR;
         auto* keyword_picker = this->ui.currentLinkedRefKYWD;
         auto  on_changed = [ref_picker, keyword_picker, sel_model, model]() {
            auto sel = sel_model->selectedRows();
            if (sel.empty())
               return;
            if (auto* ref = ref_picker->ref())
               model->setRow(sel[0].row(), keyword_picker->formStub(), ref);
         };
         QObject::connect(ref_picker,     &DKCompactObjectReferencePicker::refChanged, this, on_changed);
         QObject::connect(keyword_picker, &DKFormPicker::formChanged, this, on_changed);
      }
      this->ui.currentLinkedRefREFR->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         return ref != &this->form->stub;
      });
   #pragma endregion
   #pragma region Linked From
      this->ui.linkedFrom->setReadOnly(true);
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
      #pragma region Light properties
         this->ui.xLightFOV->setRange(0, 179.9);
         ui::set_unsigned_range<float>(this->ui.xLightFade);
         ui::set_unsigned_range<float>(this->ui.xLightEndDistanceCap);
         this->ui.xLightDepthBiasSlider->setRange(0, 50);
         this->ui.xLightDepthBiasSpinbox->setRange(0, 50);

         QObject::connect(this->ui.xLightDepthBiasSlider, &DKFloatSlider::valueChanged, this, [this](float v) {
            this->ui.xLightDepthBiasSpinbox->setValue(v);
         });
         QObject::connect(this->ui.xLightDepthBiasSpinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
            const auto blocker = QSignalBlocker(this->ui.xLightDepthBiasSlider);
            this->ui.xLightDepthBiasSlider->setValue(v);
         });
      #pragma endregion
      #pragma region Emittance Source
         this->ui.xEmitLIGH->setAllowedFormType(dovah::form_type::light);
         this->ui.xEmitREGN->setAllowedFormType(dovah::form_type::region);
         
         QObject::connect(this->ui.xEmitTypeNONE, &QAbstractButton::toggled, this, [this](bool checked) {
            if (checked) {
               this->ui.xEmitLIGH->setEnabled(false);
               this->ui.xEmitREGN->setEnabled(false);
            }
         });
         QObject::connect(this->ui.xEmitTypeLIGH, &QAbstractButton::toggled, this, [this](bool checked) {
            if (checked) {
               this->ui.xEmitLIGH->setEnabled(true);
               this->ui.xEmitREGN->setEnabled(false);
            }
         });
         QObject::connect(this->ui.xEmitTypeREGN, &QAbstractButton::toggled, this, [this](bool checked) {
            if (checked) {
               this->ui.xEmitLIGH->setEnabled(false);
               this->ui.xEmitREGN->setEnabled(true);
            }
         });
      #pragma endregion
   #pragma endregion
   #pragma region Water Currents
      for (auto* spinbox : std::array{
         this->ui.xWaterCurrentsVelLinearX,
         this->ui.xWaterCurrentsVelLinearY,
         this->ui.xWaterCurrentsVelLinearZ
      }) {
         ui::set_range<float>(spinbox);
      }
      for (auto* spinbox : std::array{
         this->ui.xWaterCurrentsVelAngularX,
         this->ui.xWaterCurrentsVelAngularY,
         this->ui.xWaterCurrentsVelAngularZ
      }) {
         spinbox->setRange(-360, 360);
      }
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

   bool has_navmesh_import_option = false;
   switch (base_type) {
      case dovah::form_type::activator:
      case dovah::form_type::container:
      case dovah::form_type::movable_static:
      case dovah::form_type::statik:
      case dovah::form_type::static_collection:
         has_navmesh_import_option = true;
         break;
   }

   const bool is_a_primitive = _is_primitive();

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
         static_assert(false, "TODO: Add tab: Patrol Data");
      }
      switch (base_type) {
         case dovah::form_type::container:
         case dovah::form_type::door:
            add_page(tr("Lock", "page names"), this->ui.pageLock);
            break;
      }
      if (base_type == dovah::form_type::door) {
         add_page(tr("Teleport", "page names"), this->ui.pageTeleport);
      }
      if (base_form && base_form->formID == dovah::hardcoded_form_ids::MapMarker) {
         add_page(tr("Map Marker", "page names"), this->ui.pageMapMarker);
      }
      if (has_navmesh_import_option) {
         static_assert(false, "TODO: Add tab: Navmesh Generation");
      }
      add_page(tr("Reflected By", "page names"), this->ui.pageReflectedBy);
      add_page(tr("Linked Refs", "page names"), this->ui.pageLinkedRefs);
      add_page(tr("Linked From", "page names"), this->ui.pageLinkedFrom);
      add_page(tr("Activate Parents", "page names"), this->ui.pageActivateParents);
      add_page(tr("Enable State Parent", "page names"), this->ui.pageEnableParent);
      add_page(tr("Lighting and Emittance", "page names"), this->ui.pageEmittance);
      if (_can_have_water_currents()) {
         add_page(tr("Water Currents", "page names"), this->ui.pageWaterCurrents);
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
         static_assert(false, "TODO: position snap spinboxes");
         ui::bind(this->ui.rotationX, working.rotation.x);
         ui::bind(this->ui.rotationY, working.rotation.y);
         ui::bind(this->ui.rotationZ, working.rotation.z);
         static_assert(false, "TODO: rotation snap spinboxes");
         if (can_rescale_ref(working.stub)) {  // Scale
            this->ui.scale->setEnabled(true);
            this->ui.scaleSnap->setEnabled(true);
            QObject::connect(this->ui.scale, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, &working](float v) {
               working.set_scale(v);

               // In case the value is clamped or rounded:
               const auto blocker = QSignalBlocker(this->ui.scale);
               this->ui.scale->setValue(working.get_scale());
            });
            static_assert(false, "TODO: scale snap spinbox");
         } else {
            this->ui.scale->setEnabled(false);
            this->ui.scaleSnap->setEnabled(false);
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
                     bind_typed_checkbox.operator()<dovah::form_type::light, record_flag::is_full_lod>(widget);
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
            ui::bind(this->ui.flagReflectedByAutoWater, record_flags(), record_flag::reflected_by_auto_water);
            {
               auto* widget = this->ui.flagRespawns;
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
            bind_typed_checkbox.operator()<dovah::form_type::actor_base, record_flag::starts_dead>(this->ui.flagStartsDead);
            static_assert(false, "TODO: Turn Off Fire");
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
   #pragma endregion
   #pragma region Ownership
   {
      using form_extra_data = extra_data_types::ownership;
      using rank_extra_data = extra_data_types::rank;
      auto* form_picker = this->ui.owner;
      auto* rank_picker = this->ui.ownerRank;
      if (const auto* extra = working.extra_data.get<form_extra_data>()) {
         dovah::form_stub* owner = extra->form.get_form_stub();
         form_picker->setFormStub(owner);
         if (owner && owner->form_type == dovah::form_type::faction) {
            rank_picker->setEnabled(true);

            const auto blocker = QSignalBlocker(rank_picker);
            this->_update_ownership_rank_picker();

            if (auto* extra_rank = working.extra_data.get<rank_extra_data>()) {
               auto i = rank_picker->findData(extra_rank->value);
               if (i >= 0) {
                  rank_picker->setCurrentIndex(i);
               } else {
                  rank_picker->setCurrentIndex(rank_picker->findData(rank_extra_data::sentinel_value_for_unset));
                  working.extra_data.remove<rank_extra_data>(working);
               }
            } else {
               rank_picker->setCurrentIndex(rank_picker->findData(rank_extra_data::sentinel_value_for_unset));
            }
         } else {
            rank_picker->setEnabled(false);
         }
      } else {
         form_picker->setFormStub(nullptr);
         rank_picker->setEnabled(false);
      }
      QObject::connect(form_picker, &DKFormPicker::formChanged, this, [this, &working, rank_picker](dovah::form_stub* stub) {
         if (stub) {
            working.extra_data.get_or_create<form_extra_data>()->form.set(working, stub);
         } else {
            working.extra_data.remove<form_extra_data>(working);
         }
         this->_update_ownership_rank_picker();
      });

      QObject::connect(&DovahKitCore::get(), &DovahKitCore::formModified, this, [this, form_picker, rank_picker](dovah::form_stub* stub) {
         if (stub != form_picker->formStub())
            return;
         this->_update_ownership_rank_picker();
      });
   }
   #pragma endregion
   #pragma region Primitive
      if (is_a_primitive) {
         bool is_typically_flat = false;

         this->ui.xPrimitiveFunction->setText(tr("Static", "primitive function"));
         switch (base_type) {
            case dovah::form_type::acoustic_space:
               this->ui.xPrimitiveFunction->setText(tr("Acoustic Space", "primitive function"));
               break;
            case dovah::form_type::sound:
               this->ui.xPrimitiveFunction->setText(tr("Sound Emitter", "primitive function"));
               break;
            default:
               if (!base_form)
                  break;
               if (base_type == dovah::form_type::activator) {
                  this->ui.xPrimitiveFunction->setText(tr("Trigger", "primitive function"));
                  break;
               }
               switch (base_form->formID) {
                  case dovah::hardcoded_form_ids::CollisionMarker:
                     this->ui.xPrimitiveFunction->setText(tr("Collision Object", "primitive function"));
                     break;
                  case dovah::hardcoded_form_ids::MultiBoundMarker:
                     this->ui.xPrimitiveFunction->setText(tr("Multibound", "primitive function"));
                     break;
                  case dovah::hardcoded_form_ids::PlaneMarker:
                     is_typically_flat = true;
                     this->ui.xPrimitiveFunction->setText(tr("Occlusion Plane", "primitive function"));
                     break;
                  case dovah::hardcoded_form_ids::PortalMarker:
                     is_typically_flat = true;
                     this->ui.xPrimitiveFunction->setText(tr("Portal", "primitive function"));
                     break;
                  case dovah::hardcoded_form_ids::RoomMarker:
                     {
                        auto* extra = working.extra_data.get<extra_data_types::room_ref_data>();
                        if (extra && extra->is_master) {
                           this->ui.xPrimitiveFunction->setText(tr("Room (master)", "primitive function"));
                        } else {
                           // This is Bethesda's terminology. Can we figure out a better word? 
                           // When does the CK even decide to make a roombound a "master?"
                           this->ui.xPrimitiveFunction->setText(tr("Room (slave)", "primitive function"));
                        }
                     }
                     break;
               }
               break;
         }

         QObject::connect(this->ui.xPrimitiveCenterX, qOverload<double>(&QDoubleSpinBox::valueChanged), this->ui.positionX, &QDoubleSpinBox::setValue);
         QObject::connect(this->ui.xPrimitiveCenterY, qOverload<double>(&QDoubleSpinBox::valueChanged), this->ui.positionY, &QDoubleSpinBox::setValue);
         QObject::connect(this->ui.xPrimitiveCenterZ, qOverload<double>(&QDoubleSpinBox::valueChanged), this->ui.positionZ, &QDoubleSpinBox::setValue);
         extra_data_types::primitive* extra = nullptr;
         if (extra = working.extra_data.get<extra_data_types::primitive>()) {
            this->ui.xPrimitiveSizeX->setValue(extra->bounds.x);
            this->ui.xPrimitiveSizeY->setValue(extra->bounds.y);
            this->ui.xPrimitiveSizeZ->setValue(extra->bounds.z);
            static_assert(false, "TODO: If this is also a multibound, sync with the multibound half-extents. Warn if they and XPRM don't match.");
         } else {
            extra = working.extra_data.get_or_create<extra_data_types::primitive>();
            if (is_typically_flat) {
               extra->shape = extra_data_types::primitive::shape::portal_box;
            } else {
               extra->shape = extra_data_types::primitive::shape::box;
            }
            if (base_form) {
               extra->color = dovah::utils::default_primitive_color_for_base_form(*base_form);
            }
            static_assert(false, "TODO: If this is also a multibound, sync with the multibound half-extents.");
         }
         this->ui.xPrimitiveColor->setColor(QColor::fromRgbF(extra->color.r, extra->color.g, extra->color.b));
         this->ui.xPrimitiveShape->setCurrentIndex(this->ui.xPrimitiveShape->findData((int)extra->shape));
         this->ui.xPrimitiveShape->setEnabled(_can_change_primitive_shape());
         //
         // The "Player Activation" checkbox just changes the layer type to L_NONCOLLIDABLE. 
         // If the base form is an Activator, then this enables an activation prompt on the 
         // primitive.
         //
         {
            QComboBox* widget = this->ui.xPrimitiveCollLayer;
            if (auto* extra = working.extra_data.get<extra_data_types::collision_data>()) {
               auto i = widget->findData((int)extra->value);
               if (i < 0)
                  i = widget->findData((int)dovah::collision_layer::unidentified);
               widget->setCurrentIndex(i);
            } else {
               widget->setCurrentIndex(widget->findData((int)dovah::collision_layer::null));
            }

            auto _update_layer = [this, widget]() {
               auto layer = (dovah::collision_layer)widget->currentData().toInt();
               if (layer == layer_for_player_activate_primitives) {
                  auto* base_form = this->form->base_form.get_form_stub();
                  if (base_form && base_form->form_type == dovah::form_type::activator) {
                     this->ui.xPrimitivePlayerActivation->setChecked(true);
                  }
               } else {
                  this->state.primitive.prior_layer = layer;
               }
            };
            _update_layer();
            QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, _update_layer);
         }
         this->ui.xPrimitivePlayerActivation->setEnabled(base_type == dovah::form_type::activator);
         QObject::connect(this->ui.xPrimitivePlayerActivation, &QCheckBox::toggled, this, [this](bool checked) {
            QComboBox* widget  = this->ui.xPrimitiveCollLayer;
            const auto blocker = QSignalBlocker(widget);

            dovah::collision_layer layer;
            if (checked) {
               layer = layer_for_player_activate_primitives;
            } else {
               layer = this->state.primitive.prior_layer;
            }
            widget->setCurrentIndex(widget->findData((int)layer));
         });
      }
   #pragma endregion
   #pragma region Item Options
      bind_single_form_extra_data<extra_data_types::leveled_item_base>(working, *this->ui.xLeveledItemBase);
      bind_single_ref_extra_data<extra_data_types::spawn_container>(working, *this->ui.xSpawnContainer);
   #pragma endregion
   #pragma region Lock
   {
      using extra_data = extra_data_types::lock;
      auto* enable = this->ui.xLockGroupbox;
      if (auto* extra = working.extra_data.get<extra_data>()) {
         enable->setChecked(true);
         this->ui.xLockKey->setFormStub(extra->key.get_form_stub());
         this->ui.xLockLevel->setCurrentIndex(this->ui.xLockLevel->findData((int)extra->level));
         this->ui.xLockIsLeveled->setChecked(extra->flags & extra_data::flag::leveled);
      } else {
         enable->setChecked(false);
      }
   }
   #pragma endregion
   #pragma region Teleport
      {
         using extra_data = extra_data_types::teleport;
         auto* enable = this->ui.xTeleportGroupbox;
         if (auto* extra = working.extra_data.get<extra_data>()) {
            enable->setChecked(true);

            auto* ref = extra->target_door.get_form_stub();
            if (ref && !dovah::form_type_is_reference(ref->form_type))
               ref = nullptr;

            this->state.teleport.prior_destination = ref;
            this->ui.xTeleportDoor->setRef(ref);
            this->ui.buttonTeleportViewLinkedDoor->setEnabled(ref != nullptr);
            if constexpr (render_window_displays_teleport_markers) {
               this->ui.buttonTeleportViewMarker->setEnabled(ref != nullptr);
            }
         } else {
            enable->setChecked(false);
            this->ui.buttonTeleportViewLinkedDoor->setEnabled(false);
            this->ui.buttonTeleportViewMarker->setEnabled(false);
         }
         QObject::connect(this->ui.buttonTeleportViewLinkedDoor, &QPushButton::clicked, this, [this]() {
            auto* ref = this->ui.xTeleportDoor->ref();
            if (!ref)
               return;
            dovahkit::subsystems::worldedit::core::get().center_on_refr(*ref);
         });
         static_assert(!render_window_displays_teleport_markers, "TODO: If the Render Window can display teleport markers, then the View Teleport Marker button should be implemented!");

         QObject::connect(this->ui.xTeleportDoor, &DKObjectReferencePicker::refChanged, this, [this](dovah::form_stub* ref) {
            if (!ref) {
               this->state.teleport.ever_changed = true;
               this->state.teleport.prior_destination = ref;
               this->ui.buttonTeleportViewLinkedDoor->setEnabled(false);
               this->ui.buttonTeleportViewMarker->setEnabled(false);
               return;
            }
            if (!_is_legal_teleport_destination(*ref)) {
               const auto blocker = QSignalBlocker(this->ui.xTeleportDoor);
               this->ui.xTeleportDoor->setRef(this->state.teleport.prior_destination);
               return;
            }
            this->state.teleport.ever_changed = true;
            this->state.teleport.prior_destination = ref;
            this->ui.buttonTeleportViewLinkedDoor->setEnabled(true);
            this->ui.buttonTeleportViewMarker->setEnabled(true);
         });
      }
      bind_single_form_extra_data<extra_data_types::teleport_name> (working, *this->ui.xTeleportName);
   #pragma endregion
   #pragma region Map Marker
   {
      using extra_data = extra_data_types::map_marker;
      auto* enable = this->ui.xMapMarkerGroupbox;
      if (auto* extra = working.extra_data.get<extra_data>()) {
         enable->setChecked(true);
         this->ui.xMapMarkerName->setText(DovahKitCore::get().convert_localized_string(extra->name));
         this->ui.xMapMarkerType->setCurrentIndex(this->ui.xMapMarkerType->findData((int)extra->type));
         this->ui.xMapMarkerVisible->setChecked(extra->flags & extra_data::flag::visible);
         this->ui.xMapMarkerCanTravel->setChecked(extra->flags & extra_data::flag::can_travel_to);
         this->ui.xMapMarkerShowAllHidden->setChecked(extra->flags & extra_data::flag::show_all_hidden);
      } else {
         enable->setChecked(false);
      }

      if (auto* extra = working.extra_data.get<extra_data_types::radius>())
         this->ui.xMapMarkerRadius->setValue(extra->value);
   }
   #pragma endregion
   #pragma region Reflected By
      static_assert(false, "TODO: Reflected By");
   #pragma endregion
   #pragma region Linked Refs
      this->models.linked_refs->importData(working);
   #pragma endregion
   #pragma region Linked From
      static_assert(false, "TODO: Linked From");
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
      #pragma region ExtraLightData
      {
         using extra_data = extra_data_types::light;
         if (auto* extra = working.extra_data.get<extra_data>()) {
            this->ui.xLightFOV->setValue(extra->fov);
            this->ui.xLightFade->setValue(extra->fade);
            this->ui.xLightEndDistanceCap->setValue(extra->end_distance_cap);
            this->ui.xLightDepthBiasSpinbox->setValue(extra->shadow_depth_bias);
         } else {
            if (base_type == dovah::form_type::light) {
               auto loaded = base_form->load().ptr_cast<dovah::loaded_forms::Light>();
               if (loaded) {
                  this->ui.xLightFOV->setValue(loaded->fov);
                  this->ui.xLightFade->setValue(loaded->fade);
                  this->ui.xLightEndDistanceCap->setValue(loaded->radius); // TODO: is this correct?
               }
            }
         }
      }
      if (base_type == dovah::form_type::light) {
         if (auto* extra = working.extra_data.get<extra_data_types::radius>())
            this->ui.xLightRadius->setValue(extra->value);

         this->ui.xLightGroupbox->setEnabled(true);
         QObject::connect(this->ui.xLightFOVReset, &QPushButton::clicked, [this, &working]() {
            auto loaded = _base_loaded_as_type<dovah::loaded_forms::Light>();
            if (loaded)
               this->ui.xLightFOV->setValue(loaded->fov);
         });
         QObject::connect(this->ui.xLightFadeReset, &QPushButton::clicked, [this, &working]() {
            auto loaded = _base_loaded_as_type<dovah::loaded_forms::Light>();
            if (loaded)
               this->ui.xLightFade->setValue(loaded->fade);
         });
         QObject::connect(this->ui.xLightDepthBiasReset, &QPushButton::clicked, [this, &working]() {
            float bias = dovah::utils::default_light_emitter_shadow_depth_bias(this->form->stub, true);
            this->ui.xLightDepthBiasSpinbox->setValue(bias);
         });
         ui::bind(this->ui.flagCastShadows,          record_flags(), loaded_form_type::form_flag::casts_shadows);
         ui::bind(this->ui.flagDoesntLightLandscape, record_flags(), loaded_form_type::form_flag::doesnt_light_landscape);
         ui::bind(this->ui.flagDoesntLightWater,     record_flags(), loaded_form_type::form_flag::doesnt_light_water);
         ui::bind(this->ui.flagNeverFades,           record_flags(), loaded_form_type::form_flag::never_fades);
      } else {
         this->ui.xLightGroupbox->setEnabled(false);
      }
      #pragma endregion
      #pragma region EmittanceSource
      {
         using extra_data = extra_data_types::emittance_source;
         if (auto* extra = working.extra_data.get<extra_data>()) {
            auto* form = extra->form.get_form_stub();
            if (form && form->form_type != dovah::form_type::light && form->form_type != dovah::form_type::region)
               form = nullptr;

            if (form) {
               if (form->form_type == dovah::form_type::light) {
                  this->ui.xEmitTypeLIGH->setChecked(true);
                  this->ui.xEmitLIGH->setEnabled(true);
                  this->ui.xEmitREGN->setEnabled(false);
                  this->ui.xEmitLIGH->setFormStub(form);
               } else {
                  this->ui.xEmitTypeREGN->setChecked(true);
                  this->ui.xEmitLIGH->setEnabled(false);
                  this->ui.xEmitREGN->setEnabled(true);
                  this->ui.xEmitREGN->setFormStub(form);
               }
            } else {
               this->ui.xEmitTypeNONE->setChecked(true);
               this->ui.xEmitLIGH->setEnabled(false);
               this->ui.xEmitREGN->setEnabled(false);
            }
         } else {
            this->ui.xEmitTypeNONE->setChecked(true);
            this->ui.xEmitLIGH->setEnabled(false);
            this->ui.xEmitREGN->setEnabled(false);
         }
      }
      #pragma endregion
   #pragma endregion
   #pragma region Water Currents
      if (_can_have_water_currents()) {
         cobb::vector3<float> vel_linear;
         cobb::vector3<float> vel_angular;

         if (base_form && base_form->formID == dovah::hardcoded_form_ids::WaterCurrentZoneMarker) {
            if (auto* extra = working.extra_data.get<extra_data_types::water_current_zone_data>()) {
               vel_linear  = extra->velocity.linear;
               vel_angular = extra->velocity.angular;
            }
         } else {
            if (auto* extra = working.extra_data.get<extra_data_types::water_data>()) {
               const auto size = extra->data.size();
               if (size >= 1) {
                  vel_linear = extra->data[0].velocity;
                  if (size >= 2) {
                     vel_angular = extra->data[1].velocity;
                  }
               }
            }
         }

         this->ui.xWaterCurrentsVelLinearX->setValue(vel_linear.x);
         this->ui.xWaterCurrentsVelLinearY->setValue(vel_linear.y);
         this->ui.xWaterCurrentsVelLinearZ->setValue(vel_linear.z);
         this->ui.xWaterCurrentsVelAngularX->setValue(vel_angular.x);
         this->ui.xWaterCurrentsVelAngularY->setValue(vel_angular.y);
         this->ui.xWaterCurrentsVelAngularZ->setValue(vel_angular.z);
      }
   #pragma endregion
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
   
   #pragma region Primitive
      if (_is_primitive()) {
         auto* extra_prim = working.extra_data.get_or_create<extra_data_types::primitive>();
         extra_prim->bounds = {
            this->ui.xPrimitiveSizeX->value(),
            this->ui.xPrimitiveSizeY->value(),
            this->ui.xPrimitiveSizeZ->value(),
         };
         {
            auto color = this->ui.xPrimitiveColor->color();
            extra_prim->color = {
               .r = (float)color.redF(),
               .g = (float)color.greenF(),
               .b = (float)color.blueF(),
               .a = extra_prim->color.a,
            };
         }
         if (_can_change_primitive_shape()) {
            extra_prim->shape = (enum extra_data_types::primitive::shape) this->ui.xPrimitiveShape->currentData().toInt();
         }

         auto layer = (dovah::collision_layer) this->ui.xPrimitiveCollLayer->currentData().toInt();
         if (layer == dovah::collision_layer::unidentified) {
            working.extra_data.remove<extra_data_types::collision_data>(working);
         } else {
            working.extra_data.get_or_create<extra_data_types::collision_data>()->set_layer_id(layer);
         }

         static_assert(false, "TODO: If this is also a multibound, sync with the multibound half-extents.");
      }
   #pragma endregion
   #pragma region Lock
   {
      using extra_data = extra_data_types::lock;
      if (this->ui.xLockGroupbox->isChecked()) {
         working.extra_data.remove<extra_data>(working);
      } else {
         auto* extra = working.extra_data.get_or_create<extra_data>();
         extra->key.set(working, this->ui.xLockKey->formStub());
         extra->level = this->ui.xLockLevel->currentData().toInt();
         cobb::edit_bit(extra->flags, extra_data::flag::leveled, this->ui.xLockIsLeveled->isChecked());
      }
   }
   #pragma endregion
   #pragma region Teleport
   {
      using extra_data = extra_data_types::teleport;

      dovah::form_stub* prior_destination = nullptr;
      bool disconnect_prior_destination = false;
      if (auto* extra = working.extra_data.get<extra_data>()) {
         prior_destination = extra->target_door.get_form_stub();
      }

      if (this->ui.xTeleportGroupbox->isChecked()) {
         working.extra_data.remove<extra_data>(working);
         disconnect_prior_destination = true;
      } else if (base_type == dovah::form_type::door) {
         auto* destination = this->ui.xTeleportDoor->ref();
         if (destination && !_is_legal_teleport_destination(*destination)) {
            QMessageBox::critical(
               this,
               tr("Error"),
               tr(
                  "The destination that you selected for this load door was edited at some "
                  "point between you selecting it and you clicking OK. Those edits have "
                  "caused it to no longer be a valid destination (e.g. because it's not a "
                  "door anymore, or because it's been connected to another load door. This "
                  "ref will not be connected to that destination."
               )
            );
            destination = nullptr;
         }
         auto* extra = working.extra_data.get_or_create<extra_data>();
         extra->target_door.set(working, destination);

         if (destination) {
            auto loaded = destination->load().ptr_cast<loaded_form_type>();
            if (loaded) {
               if (this->state.teleport.ever_changed) {
                  //
                  // Try to automatically position this door's teleport marker in front of 
                  // the destination door, and facing away from it.
                  //
                  auto coords = _calc_teleport_marker_position(*loaded);
                  extra->position = coords.first;
                  extra->rotation = coords.second;
               }
               //
               // Create teleport data on the destination door, so that the connection 
               // between these doors is bidirectional.
               //
               auto* extra = loaded->extra_data.get_or_create<extra_data>();
               if (extra->target_door != &this->form->stub) {
                  extra->target_door.set(*loaded, &this->form->stub);
                  //
                  // Try to automatically position the destination's teleport marker in 
                  // front of this door, and facing away from it.
                  //
                  auto coords = _calc_teleport_marker_position(working);
                  extra->position = coords.first;
                  extra->rotation = coords.second;
               }
            }
         }
         disconnect_prior_destination = destination != prior_destination;
      }
      if (disconnect_prior_destination && prior_destination) {
         //
         // This door was previously connected to some other door. We've broken that 
         // connection, so delete the old destination's teleport data.
         //
         auto loaded = prior_destination->load().ptr_cast<loaded_form_type>();
         if (loaded) {
            if (auto* extra = loaded->extra_data.get<extra_data>()) {
               if (extra->target_door == &this->form->stub) {
                  loaded->extra_data.remove<extra_data>(*loaded);
                  prior_destination->set_edited(true);
               }
            }
         }
      }
   }
   #pragma endregion
   #pragma region Map Marker
   {
      using extra_data = extra_data_types::map_marker;
      if (this->ui.xMapMarkerGroupbox->isChecked()) {
         working.extra_data.remove<extra_data>(working);
      } else {
         auto* extra = working.extra_data.get_or_create<extra_data>();
         editor.assign_localized_string(extra->name, this->ui.xMapMarkerName->text());
         extra->type  = this->ui.xMapMarkerType->currentData().toInt();
         cobb::edit_bit(extra->flags, extra_data::flag::visible, this->ui.xMapMarkerVisible->isChecked());
         cobb::edit_bit(extra->flags, extra_data::flag::can_travel_to, this->ui.xMapMarkerCanTravel->isChecked());
         cobb::edit_bit(extra->flags, extra_data::flag::show_all_hidden, this->ui.xMapMarkerShowAllHidden->isChecked());
      }
      
      float radius = this->ui.xMapMarkerRadius->value();
      if (radius) {
         working.extra_data.get_or_create<extra_data_types::radius>()->value = radius;
      } else {
         working.extra_data.remove<extra_data_types::radius>(working);
      }
   }
   #pragma endregion
   #pragma region Linked Refs
      this->models.linked_refs->exportData(working);
   #pragma endregion
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
      #pragma region ExtraLightData
         if (base_type == dovah::form_type::light) {
            const float radius = this->ui.xLightRadius->value();
            const float fov    = this->ui.xLightFOV->value();
            const float fade   = this->ui.xLightFade->value();
            const float cap    = this->ui.xLightEndDistanceCap->value();
            const float bias   = this->ui.xLightDepthBiasSpinbox->value();

            bool any_changed = true;
            {
               auto loaded = _base_loaded_as_type<dovah::loaded_forms::Light>();
               if (loaded) {
                  any_changed = false;
                  if (loaded->radius != radius
                   || loaded->fade != fade
                   || loaded->fov  != fov
                  ) {
                     any_changed = true;
                  }
               }
            }
            if (any_changed) {
               auto* extra = working.extra_data.get_or_create<extra_data_types::light>();
               extra->fade = fade;
               extra->fov  = fov;
               extra->end_distance_cap  = cap;
               extra->shadow_depth_bias = bias;
            } else {
               working.extra_data.remove<extra_data_types::light>(working);
            }

            working.extra_data.get_or_create<extra_data_types::radius>()->value = radius;
         }
      #pragma endregion
      #pragma region EmittanceSource
      {
         using extra_data = extra_data_types::emittance_source;
         dovah::form_stub* form = nullptr;
         if (this->ui.xEmitTypeLIGH->isChecked()) {
            form = this->ui.xEmitLIGH->formStub();
         } else if (this->ui.xEmitTypeREGN->isChecked()) {
            form = this->ui.xEmitREGN->formStub();
         }
         if (form) {
            working.extra_data.get_or_create<extra_data>()->form.set(working, form);
         } else {
            working.extra_data.remove<extra_data>(working);
         }
      }
      #pragma endregion
   #pragma endregion
   #pragma region Water Currents
      if (_can_have_water_currents()) {
         cobb::vector3<float> vel_linear = {
            this->ui.xWaterCurrentsVelLinearX->value(),
            this->ui.xWaterCurrentsVelLinearY->value(),
            this->ui.xWaterCurrentsVelLinearZ->value(),
         };
         cobb::vector3<float> vel_angular = {
            this->ui.xWaterCurrentsVelAngularX->value(),
            this->ui.xWaterCurrentsVelAngularY->value(),
            this->ui.xWaterCurrentsVelAngularZ->value(),
         };

         if (base_form && base_form->formID == dovah::hardcoded_form_ids::WaterCurrentZoneMarker) {
            auto* extra = working.extra_data.get_or_create<extra_data_types::water_current_zone_data>();
            extra->velocity.linear  = vel_linear;
            extra->velocity.angular = vel_angular;
         } else {
            auto* extra = working.extra_data.get_or_create<extra_data_types::water_data>();
            if (extra->data.size() < 2) {
               extra->data.resize(2);
            }
            extra->data[0].velocity = vel_linear;
            extra->data[1].velocity = vel_angular;
         }
      }
   #pragma endregion
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

bool FormDialogObjectReference::_is_primitive() const noexcept {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;

   switch (base_form->form_type) {
      case dovah::form_type::acoustic_space:
      case dovah::form_type::sound:
         return true;
      case dovah::form_type::activator:
         return this->form->extra_data.get<extra_data_types::primitive>() != nullptr;
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
void FormDialogObjectReference::_update_ownership_rank_picker() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   auto* form_picker = this->ui.owner;
   auto* rank_picker = this->ui.ownerRank;

   auto* form = form_picker->formStub();
   if (!form || form->form_type != dovah::form_type::faction) {
      rank_picker->clear();
      working.extra_data.remove<extra_data_types::rank>(working);
      return;
   }

   int prior_value = extra_data_types::rank::sentinel_value_for_unset;
   {
      auto data = rank_picker->currentData();
      if (data.isValid())
         prior_value = data.toInt();
   }
   const auto blocker = QSignalBlocker(rank_picker);
   rank_picker->clear();
   rank_picker->addItem(tr("Any Rank", "ownership: no faction rank"), -1);

   auto loaded = form->load().ptr_cast<dovah::loaded_forms::Faction>();
   if (!loaded) {
      working.extra_data.remove<extra_data_types::rank>(working);
      return;
   }
   for (auto& rank : loaded->ranks) {
      QString text;
      {
         auto masc = editor.convert_localized_string(rank.title_masc);
         auto fem = editor.convert_localized_string(rank.title_fem);
         if (masc == fem) {
            text = fem;
         } else {
            text = tr("%1 / %2", "ownership: faction rank names").arg(masc).arg(fem);
         }
      }
      rank_picker->addItem(text, rank.id);
   }
   auto i = rank_picker->findData(prior_value);
   if (i >= 0) {
      rank_picker->setCurrentIndex(i);
   } else {
      rank_picker->setCurrentIndex(0);
      working.extra_data.remove<extra_data_types::rank>(working);
   }
}

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
bool FormDialogObjectReference::_can_change_primitive_shape() const {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;
   return base_form->form_type == dovah::form_type::activator;
}
bool FormDialogObjectReference::_can_have_attach_ref() const;
bool FormDialogObjectReference::_can_have_water_currents() const {
   dovah::form_stub* base_form = this->form->base_form.get_form_stub();
   if (!base_form)
      return false;

   return base_form->test_record_flags(1 << 19);
}
bool FormDialogObjectReference::_is_legal_teleport_destination(dovah::form_stub& ref) const {
   auto* base = dovah::form_stub_helpers::get_base_form(&ref);
   if (!base || base->form_type != dovah::form_type::door) {
      return false;
   }

   auto loaded = ref.load().ptr_cast<loaded_form_type>();
   if (loaded) {
      auto* extra = loaded->extra_data.get<extra_data_types::teleport>();
      if (extra) {
         if (extra->target_door.get_form_stub() != &this->form->stub) {
            //
            // The desired door is already a teleporter to somewhere else!
            //
            return false;
         }
      }
   }
   return true;
}

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/euler_angles.hpp>
#include "vulkan/helpers/glm_transform_from_beth.h"
/*static*/ std::pair<cobb::vector3<float>, cobb::vector3<float>> FormDialogObjectReference::_calc_teleport_marker_position(const loaded_form_type& in_front_of) {
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
