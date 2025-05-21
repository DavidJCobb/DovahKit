#include "./reverb_parameters.h"
#include "dovah/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

struct Preset {
   const char* name;

   uint16_t decay_time     = 0; // DATA+0x00
   uint16_t hf_reference   = 0; // DATA+0x02
   int8_t   room_filter    = 0; // DATA+0x04
   int8_t   room_hf_filter = 0; // DATA+0x05
   int8_t   reflections    = 0; // DATA+0x06
   int8_t   reverb_amp     = 0; // DATA+0x07
   uint8_t  decay_hf_ratio = 0; // DATA+0x08 // v*100
   uint8_t  reflect_delay  = 0; // DATA+0x09
   uint8_t  reverb_delay   = 0; // DATA+0x0A
   uint8_t  diffusion      = 0; // DATA+0x0B
   uint8_t  density        = 0; // DATA+0x0C
   uint8_t  unk0D          = 0; // DATA+0x0D
};

static constexpr const auto all_presets = std::array{
   []() constexpr -> Preset {
      Preset p;
      p.name = "Default";
      p.reverb_amp     = -100;
      p.decay_time     = 1000;
      p.reverb_delay   = 3;
      p.room_filter    = -100;
      p.room_hf_filter = 0;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.5 * 100;
      p.reflections    = -100;
      p.reflect_delay  = 2;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Generic";
      p.reverb_amp     = 2;
      p.decay_time     = 1500;
      p.reverb_delay   = 3;
      p.room_filter    = -10;
      p.room_hf_filter = 1;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.83 * 100;
      p.reflections    = -26;
      p.reflect_delay  = 1;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Padded Cell";
      p.reverb_amp     = 2;
      p.decay_time     = 170;
      p.reverb_delay   = 0;
      p.room_filter    = -10;
      p.room_hf_filter = -60;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.1 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 0;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Room";
      p.reverb_amp     = 0;
      p.decay_time     = 400;
      p.reverb_delay   = 0;
      p.room_filter    = -10;
      p.room_hf_filter = -45;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.83 * 100;
      p.reflections    = -16;
      p.reflect_delay  = 0;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Bathroom";
      p.reverb_amp     = 10;
      p.decay_time     = 1500;
      p.reverb_delay   = 1;
      p.room_filter    = -10;
      p.room_hf_filter = -12;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.54 * 100;
      p.reflections    = -3;
      p.reflect_delay  = 0;
      p.diffusion      = 100;
      p.density        = 60;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Livingroom";
      p.reverb_amp     = -11;
      p.decay_time     = 500;
      p.reverb_delay   = 1;
      p.room_filter    = -10;
      p.room_hf_filter = -60;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.1 * 100;
      p.reflections    = -14;
      p.reflect_delay  = 1;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Stoneroom";
      p.reverb_amp     = 1;
      p.decay_time     = 2310;
      p.reverb_delay   = 15;
      p.room_filter    = -10;
      p.room_hf_filter = -3;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.64 * 100;
      p.reflections    = -7;
      p.reflect_delay  = 14;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Auditorium";
      p.reverb_amp     = -3;
      p.decay_time     = 4320;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -5;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.59 * 100;
      p.reflections    = -8;
      p.reflect_delay  = 24;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Concert Hall";
      p.reverb_amp     = 0;
      p.decay_time     = 3920;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -5;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.7 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 24;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Cave";
      p.reverb_amp     = -3;
      p.decay_time     = 2910;
      p.reverb_delay   = 18;
      p.room_filter    = -10;
      p.room_hf_filter = 0;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 1.3 * 100;
      p.reflections    = -6;
      p.reflect_delay  = 18;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Arena";
      p.reverb_amp     = 0;
      p.decay_time     = 7240;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -7;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.33 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 24;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Hangar";
      p.reverb_amp     = 2;
      p.decay_time     = 1005;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -10;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.23 * 100;
      p.reflections    = -6;
      p.reflect_delay  = 24;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Carpeted Hallway";
      p.reverb_amp     = -16;
      p.decay_time     = 100;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -40;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.1 * 100;
      p.reflections    = -18;
      p.reflect_delay  = 0;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Hallway";
      p.reverb_amp     = 4;
      p.decay_time     = 1490;
      p.reverb_delay   = 10;
      p.room_filter    = -10;
      p.room_hf_filter = -3;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.59 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 8;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Stone Corridor";
      p.reverb_amp     = 4;
      p.decay_time     = 2700;
      p.reverb_delay   = 18;
      p.room_filter    = -10;
      p.room_hf_filter = -2;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.79 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 16;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Alley";
      p.reverb_amp     = 0;
      p.decay_time     = 1490;
      p.reverb_delay   = 10;
      p.room_filter    = -10;
      p.room_hf_filter = -3;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.86 * 100;
      p.reflections    = -12;
      p.reflect_delay  = 8;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Forest";
      p.reverb_amp     = -6;
      p.decay_time     = 1490;
      p.reverb_delay   = 80;
      p.room_filter    = -10;
      p.room_hf_filter = -33;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.54 * 100;
      p.reflections    = -25;
      p.reflect_delay  = 194;
      p.diffusion      = 79;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "City";
      p.reverb_amp     = -22;
      p.decay_time     = 1490;
      p.reverb_delay   = 10;
      p.room_filter    = -10;
      p.room_hf_filter = -8;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.67 * 100;
      p.reflections    = -23;
      p.reflect_delay  = 8;
      p.diffusion      = 50;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Mountains";
      p.reverb_amp     = -20;
      p.decay_time     = 1490;
      p.reverb_delay   = 100;
      p.room_filter    = -10;
      p.room_hf_filter = -25;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.21 * 100;
      p.reflections    = -27;
      p.reflect_delay  = 300;
      p.diffusion      = 27;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Quarry";
      p.reverb_amp     = 5;
      p.decay_time     = 1490;
      p.reverb_delay   = 25;
      p.room_filter    = -10;
      p.room_hf_filter = -10;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.83 * 100;
      p.reflections    = -100;
      p.reflect_delay  = 73;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Plain";
      p.reverb_amp     = -25;
      p.decay_time     = 1490;
      p.reverb_delay   = 100;
      p.room_filter    = -10;
      p.room_hf_filter = -20;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.5 * 100;
      p.reflections    = -25;
      p.reflect_delay  = 215;
      p.diffusion      = 21;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Parking Lot";
      p.reverb_amp     = -11;
      p.decay_time     = 1650;
      p.reverb_delay   = 10;
      p.room_filter    = -10;
      p.room_hf_filter = 0;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 1.5 * 100;
      p.reflections    = -14;
      p.reflect_delay  = 10;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Sewer Pipe";
      p.reverb_amp     = 6;
      p.decay_time     = 2810;
      p.reverb_delay   = 20;
      p.room_filter    = -10;
      p.room_hf_filter = -10;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.14 * 100;
      p.reflections    = 4;
      p.reflect_delay  = 17;
      p.diffusion      = 80;
      p.density        = 60;
      return p;
   }(),
   []() constexpr -> Preset {
      Preset p;
      p.name = "Underwater";
      p.reverb_amp     = 17;
      p.decay_time     = 1490;
      p.reverb_delay   = 10;
      p.room_filter    = -10;
      p.room_hf_filter = -40;
      p.hf_reference   = 5000;
      p.decay_hf_ratio = 0.1 * 100;
      p.reflections    = -4;
      p.reflect_delay  = 8;
      p.diffusion      = 100;
      p.density        = 100;
      return p;
   }(),
};

FormDialogReverbParameters::FormDialogReverbParameters(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* picker = this->ui.preset;
      auto* button = this->ui.buttonApplyPreset;
      picker->clear();
      for (auto& preset : all_presets) {
         picker->addItem(tr(preset.name, "preset name"));
      }
      QObject::connect(button, &QPushButton::clicked, this, [this]() {
         if (!this->form)
            return;
         int i = this->ui.preset->currentIndex();
         if (i < 0 || i >= all_presets.size())
            return;
         auto& preset = all_presets[i];
         this->form->decay_time = preset.decay_time;
         this->form->hf_reference = preset.hf_reference;
         this->form->room_filter = preset.room_filter;
         this->form->room_hf_filter = preset.room_hf_filter;
         this->form->reflections = preset.reflections;
         this->form->reverb_amp = preset.reverb_amp;
         this->form->decay_hf_ratio = preset.decay_hf_ratio;
         this->form->reflect_delay = preset.reflect_delay;
         this->form->reverb_delay = preset.reverb_delay;
         this->form->diffusion = preset.diffusion;
         this->form->density = preset.density;
         this->form->unk0D = preset.unk0D;
      });
   }

   #pragma push_macro("MAKE_EDIT");
   #define MAKE_EDIT(field, uiName) \
      { \
         using value_type   = std::decay_t<decltype(this->ui.uiName##Slider->value())>; \
         using spinbox_value_type = std::decay_t<decltype(this->ui.uiName##Spinbox->value())>; \
         using slider_type  = std::decay_t<decltype(*this->ui.uiName##Slider)>; \
         using spinbox_type = std::decay_t<decltype(*this->ui.uiName##Spinbox)>; \
         QObject::connect(this->ui.uiName##Slider, &slider_type::valueChanged, this, [this](value_type v) { \
            this->ui.uiName##Spinbox->setValue(v); \
         });\
         QObject::connect(this->ui.uiName##Spinbox, qOverload<spinbox_value_type>(&spinbox_type::valueChanged), this, [this](value_type v) { \
            const auto blocker = QSignalBlocker(this->ui.uiName##Slider); \
            this->ui.uiName##Slider->setValue(v); \
         });\
      }
   MAKE_EDIT(reverb_amp, reverbAmp);
   MAKE_EDIT(decay_time, decayTime);
   MAKE_EDIT(reverb_delay, reverbDelay);
   MAKE_EDIT(room_filter, roomFilter);
   MAKE_EDIT(room_hf_filter, roomHFFilter);
   MAKE_EDIT(hf_reference, hfReference);
   MAKE_EDIT(decay_hf_ratio, decayHFRatio);
   MAKE_EDIT(reflections, reflections);
   MAKE_EDIT(reflect_delay, reflectDelay);
   MAKE_EDIT(diffusion, diffusion);
   MAKE_EDIT(density, density);
   #pragma pop_macro("MAKE_EDIT");

   this->load(); // this creates the working copy.
}
void FormDialogReverbParameters::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.reverbAmpSpinbox, working.reverb_amp);
   ui::bind(this->ui.decayTimeSpinbox, working.decay_time);
   ui::bind(this->ui.reverbDelaySpinbox, working.decay_time);
   #pragma push_macro("MAKE_EDIT");
   #define MAKE_EDIT(field, uiName) ui::bind(this->ui.uiName##Spinbox, working.field);
   MAKE_EDIT(reverb_amp, reverbAmp);
   MAKE_EDIT(decay_time, decayTime);
   MAKE_EDIT(reverb_delay, reverbDelay);
   MAKE_EDIT(room_filter, roomFilter);
   MAKE_EDIT(room_hf_filter, roomHFFilter);
   MAKE_EDIT(hf_reference, hfReference);
   {
      auto* widget = this->ui.decayHFRatioSpinbox;
      widget->setValue((double)working.decay_hf_ratio / 100);
      QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
         this->form->decay_hf_ratio = v * 100;
      });
   }
   MAKE_EDIT(reflections, reflections);
   {
      auto* widget = this->ui.reflectDelaySpinbox;
      widget->setValue(working.reflect_delay * 300 / 255);
      QObject::connect(widget, qOverload<int>(&QSpinBox::valueChanged), this, [this](int v) {
         this->form->reflect_delay = v * 255 / 300;
      });
   }
   //MAKE_EDIT(reflect_delay, reflectDelay);
   MAKE_EDIT(diffusion, diffusion);
   MAKE_EDIT(density, density);
   #pragma pop_macro("MAKE_EDIT");
}
void FormDialogReverbParameters::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
}