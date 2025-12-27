#include "./reference.h"
#include <QMessageBox>
#include "dovah/core.h"
#include "dovah/form_stubs/helpers/get_base_form.h"
#include "editor/helpers/form_identifiers_to_string.h"
#include "editor/subsystems/message_log/core.h"
#include "editor/subsystems/worldedit/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/typical_tableview_config.h"
#include "./reference/ObjectReferenceLinkedRefsModel.h"
#include "./reference/ObjectReferenceNewLinkedRefDialog.h"

#include "dovah/data/all_carryable_form_types.h"
#include "dovah/data/hardcoded_form_ids.h"
#include "dovah/forms/components/model.h"
#include "dovah/forms/_component_access.h"
#include "dovah/utils/default_light_emitter_shadow_depth_bias.h"

namespace {
   constexpr bool render_window_displays_teleport_markers = false;
   constexpr bool render_window_displays_test_radii       = false;
}

#include "dovah/forms/Faction.h"
#include "dovah/forms/Light.h"
#pragma region Extra data includes
   #include "dovah/forms/components/extra_data/types/a/alpha_cutoff.h"
   #include "dovah/forms/components/extra_data/types/a/attach_ref.h"
   #include "dovah/forms/components/extra_data/types/c/charge.h"
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
      {
         using model_type = ObjectReferenceLinkedRefsModel;

         auto* listview = this->ui.linkedRefs;
         auto* model    = this->models.linked_refs = new model_type(listview);
         listview->setModel(model);

         ui::typical_tableview_config(listview);

         auto* sel_model = listview->selectionModel();
         QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model, sel_model](const QItemSelection& sel) {
            if (sel.empty()) {
               this->ui.currentLinkedRefGroupbox->setEnabled(false);
               return;
            }
            QModelIndex qmi = sel[0].topLeft();
            this->ui.currentLinkedRefGroupbox->setEnabled(true);

            const auto blockers = std::array{
               QSignalBlocker(this->ui.currentLinkedRefKYWD),
               QSignalBlocker(this->ui.currentLinkedRefREFR),
            };
            this->ui.currentLinkedRefKYWD->setFormStub(
               model->data(
                  qmi.siblingAtColumn(model_type::Column::KeywordName),
                  model_type::FormStubRole
               ).value<dovah::form_stub*>()
            );
            this->ui.currentLinkedRefREFR->setRef(
               model->data(
                  qmi.siblingAtColumn(model_type::Column::RefName),
                  model_type::FormStubRole
               ).value<dovah::form_stub*>()
            );
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
            auto* kywd = model->data(qmi.siblingAtColumn(model_type::Column::KeywordName), model_type::FormStubRole).value<dovah::form_stub*>();
            model->setLink(kywd, nullptr);
         });
      }
      this->ui.currentLinkedRefKYWD->setAllowedFormType(dovah::form_type::keyword);
      this->ui.currentLinkedRefREFR->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         return ref != &this->form->stub;
      });
   #pragma endregion
   #pragma region Linked From
      this->ui.linkedFrom->setReadOnly(true);
   #pragma endregion
   #pragma region Activate Parents
      this->ui.currentActivateParentRef->setValidationFunction([this](dovah::form_stub* ref) -> bool {
         return ref != &this->form->stub;
      });
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
      if (is_an_item) {
         add_page(tr("Item Properties", "page names"), this->ui.pageItem);
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
               auto* widget = this->ui.flagHiddenFromLocalMap;
               switch (base_type) {
                  case dovah::form_type::activator:
                  case dovah::form_type::door:
                  case dovah::form_type::statik:
                  case dovah::form_type::tree:
                     widget->setEnabled(true);
                     break;
                  default:
                     widget->setEnabled(false);
                     break;
               }
               QObject::connect(widget, &QCheckBox::toggled, this, [this, &working](bool checked) {
                  auto* base = working.base_form.get_form_stub();
                  if (!base)
                     return;
                  uint32_t mask = 0;
                  switch (base->form_type) {
                     case dovah::form_type::door:
                        mask = record_flag::hide_from_local_map_a;
                        break;
                     case dovah::form_type::activator:
                     case dovah::form_type::statik:
                     case dovah::form_type::tree:
                        mask = record_flag::hide_from_local_map_b;
                        break;
                  }
                  if (!mask)
                     return;
                  cobb::edit_bit(this->record_flags(), mask, checked);
               });
            }
            {
               auto* widget = this->ui.flagIgnoredBySandbox;
               widget->setChecked(_get_ignored_by_sandbox());
               QObject::connect(widget, &QCheckBox::toggled, this, &FormDialogObjectReference::_set_ignored_by_sandbox);
            }
            bind_typed_checkbox.operator()<dovah::form_type::door, record_flag::inaccessible>(this->ui.flagDoorInaccessible);
            ui::bind(this->ui.flagDisabled, record_flags(), record_flag::disabled);
            bind_typed_checkbox.operator()<dovah::form_type::light, record_flag::is_full_lod>(this->ui.flagIsFullLOD);
            ui::bind(this->ui.flagMotionBlur, record_flags(), record_flag::motion_blur);
         #pragma endregion
         #pragma region Right column
            ui::bind(this->ui.flagNoAIAcquire, record_flags(), record_flag::no_ai_acquire);
            {
               auto* widget = this->ui.flagOpenByDefault;
               switch (base_type) {
                  case dovah::form_type::container:
                  case dovah::form_type::door:
                     widget->setEnabled(true);
                     break;
                  default:
                     widget->setEnabled(false);
                     break;
               }
               static_assert(false, "TODO: How is this state stored?");
            }
            ui::bind(this->ui.flagReflectedByAutoWater, record_flags(), record_flag::reflected_by_auto_water);
            ui::bind_inverse(this->ui.flagRespawns, record_flags(), record_flag::no_respawn);
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
      bind_single_ref_extra_data<extra_data_types::attach_ref>(working, *this->ui.xAttachRef);
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
      static_assert(false, "TODO: Is Sky Marker");
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
            working.extra_data.get_or_create<form_extra_data>()->form.set_form_stub(working, stub);
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
      static_assert(false, "TODO: Activate Parents");
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
      static_assert(false, "TODO: Water Currents");
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
   }
   #pragma endregion
   #pragma region Linked Refs
      this->models.linked_refs->exportData(working);
   #pragma endregion
   #pragma region Activate Parents
      static_assert(false, "TODO: Activate Parents");
   #pragma endregion
   #pragma region Enable Parent
      static_assert(false, "TODO: Enable Parent");
   #pragma endregion
   #pragma region Lighting and Emittance
      #pragma region ExtraLightData
         static_assert(false, "TODO");
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
      static_assert(false, "TODO: Water Currents");
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

bool FormDialogObjectReference::_can_have_water_currents() const;
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
