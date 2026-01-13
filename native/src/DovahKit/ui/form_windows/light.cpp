#include "./light.h"
#include <limits>
#include "editor/subsystems/game_localized_strings/core.h"
#include "ui/utils/bind.h"

FormDialogLight::FormDialogLight(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   {
      const char* disambig = "light type";

      auto* types = this->ui.lightType;
      types->clear();
      types->addItem(tr("Omnidirectional",                disambig), (int)loaded_form_type::emitter_type::omni);
      types->addItem(tr("Shadow-Casting Hemisphere",      disambig), (int)loaded_form_type::emitter_type::hemi_shadow);
      types->addItem(tr("Shadow-Casting Omnidirectional", disambig), (int)loaded_form_type::emitter_type::omni_shadow);
      types->addItem(tr("Shadow-Casting Spotlight",       disambig), (int)loaded_form_type::emitter_type::spot_shadow);
      QObject::connect(types, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         auto type = (loaded_form_type::emitter_type) this->ui.lightType->currentData().toInt();

         bool is_spotlight = (type == loaded_form_type::emitter_type::spot) || (type == loaded_form_type::emitter_type::spot_shadow);
         this->ui.FOV->setEnabled(is_spotlight);
         this->ui.falloffExponent->setEnabled(is_spotlight);
      });
   }
   this->ui.fade->setRange(0, std::numeric_limits<float>::max());
   this->ui.FOV->setRange(0, 179.99);
   this->ui.falloffExponent->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
   this->ui.radius->setRange(0, std::numeric_limits<int32_t>::max());
   this->ui.nearClip->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());

   {
      auto* types = this->ui.flickerType;
      types->clear();
      types->addItem(tr("None"),          (int)loaded_form_type::flicker_type::none);
      types->addItem(tr("Flicker"),       (int)loaded_form_type::flicker_type::flicker);
      types->addItem(tr("Pulse"),         (int)loaded_form_type::flicker_type::pulse);
      types->addItem(tr("Flicker, Slow"), (int)loaded_form_type::flicker_type::flicker_slow);
      types->addItem(tr("Pulse, Slow"),   (int)loaded_form_type::flicker_type::pulse_slow);
      QObject::connect(types, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](bool checked) {
         auto data    = (loaded_form_type::flicker_type) this->ui.flickerType->currentData().toInt();
         bool is_none = data == loaded_form_type::flicker_type::none;

         this->ui.flickerPeriod->setDisabled(is_none);
         this->ui.flickerIntensityAmplitude->setDisabled(is_none);
         this->ui.flickerMovementAmplitude->setDisabled(is_none);
      });
   }
   this->ui.flickerPeriod->setRange(0, std::numeric_limits<float>::max());
   this->ui.flickerIntensityAmplitude->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
   this->ui.flickerMovementAmplitude->setRange(std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
   
   QObject::connect(this->ui.timeIsUnlimited, &QCheckBox::toggled, this, [this](bool checked) {
      this->ui.time->setDisabled(checked);
   });
   this->ui.weight->setRange(0, std::numeric_limits<float>::max());
   this->ui.value->setRange(0, std::numeric_limits<int32_t>::max());
   this->ui.time->setRange(0, std::numeric_limits<int32_t>::max()); // -1 == infinity

   this->load(); // this creates the working copy.
}
void FormDialogLight::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   this->ui.name->setText(gls.convert_localized_string(working.item_data.name));

   this->ui.model->initializeFrom(working.model);
   this->ui.destructionData->initializeFrom(working.destruction_data);
   
   ui::bind(this->ui.fade,            working.fade);
   ui::bind(this->ui.color,           working.color);
   ui::bind(this->ui.lightType,       working.light_type);
   ui::bind(this->ui.radius,          working.radius);
   ui::bind(this->ui.FOV,             working.fov);
   ui::bind(this->ui.falloffExponent, working.falloff_exponent);
   ui::bind(this->ui.nearClip,        working.near_clip);

   ui::bind(this->ui.flickerType,               working.flicker.type);
   ui::bind(this->ui.flickerPeriod,             working.flicker.period);
   ui::bind(this->ui.flickerIntensityAmplitude, working.flicker.amplitudes.intensity);
   ui::bind(this->ui.flickerMovementAmplitude,  working.flicker.amplitudes.movement);

   ui::bind(this->ui.inventoryIcon, working.item_data.icons.inventory);
   ui::bind(this->ui.messageIcon,   working.item_data.icons.message);
   ui::bind(this->ui.weight, working.item_data.weight);
   ui::bind(this->ui.value,  working.item_data.value);
   ui::bind(this->ui.time,   working.time);
   QObject::connect(this->ui.timeIsUnlimited, &QCheckBox::toggled, this, [this](bool checked) {
      auto& working = *this->form;
      if (checked) {
         working.time = loaded_form_type::unlimited_duration;
      } else {
         working.time = this->ui.time->value();
      }
   });

   {
      ui::bind(this->ui.flagPortalStrict,    this->record_flags(), loaded_form_type::form_flag::portal_strict);
      ui::bind(this->ui.flagRandomAnimStart, this->record_flags(), loaded_form_type::form_flag::random_anim_start);
      ui::bind(this->ui.flagObstacle,        this->record_flags(), loaded_form_type::form_flag::obstacle);

      ui::bind(this->ui.itemInfo,             working.light_flags, loaded_form_type::light_flag::can_be_carried);
      ui::bind(this->ui.flagItemOffByDefault, working.light_flags, loaded_form_type::light_flag::off_by_default);

      // This flag is in BOTH masks. What the heck, Bethesda?
      QObject::connect(this->ui.flagPortalStrict, &QCheckBox::toggled, this, [this](bool checked) {
         if (checked)
            this->form->light_flags |= loaded_form_type::light_flag::portal_strict;
         else
            this->form->light_flags &= ~loaded_form_type::light_flag::portal_strict;
      });
      this->ui.flagPortalStrict->toggled(this->ui.flagPortalStrict->isChecked()); // fire signal to force-sync the working form flags

      // Remaining flags (not in the CK UI):
      // "Dynamic"
      // "Negative"
   }

   this->ui.scriptListPane->setFormWorkingCopy(&working);
}
void FormDialogLight::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& gls     = dovahkit::subsystems::game_localized_strings::core::get();
   auto& working = *this->form;
   
   gls.assign_localized_string(working.item_data.name, this->ui.name->text());
   this->ui.model->commitTo(working.model, working);
   this->ui.destructionData->commitTo(working.destruction_data, working);

   this->ui.scriptListPane->commit();
}