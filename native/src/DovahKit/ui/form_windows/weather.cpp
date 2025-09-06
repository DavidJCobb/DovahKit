#include "./weather.h"
#include "dovah/core.h"
#include "dovah/data/game_settings.h"
#include "dovah/forms/Sound.h"
#include "editor/core.h"
#include "ui/utils/bind.h"
#include "ui/utils/enable_inbound_drag_and_drop_insertions.h"
#include "ui/utils/pair_slider_to_spinbox.h"
#include "ui/utils/set_range.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./weather/WeatherSoundsModel.h"

namespace {
   float _get_float_game_setting(const std::string_view name) {
      auto& editor = DovahKitCore::get();
      {
         dovah::loaded_game_setting loaded;
         if (editor.get_loaded_game_setting(name.data(), loaded))
            return loaded.value.f;
      }
      for (const auto& info : dovah::game_settings)
         if (info.name == name)
            return info.default_value.f;
      return 0.0F;
   }
}

FormDialogWeather::FormDialogWeather(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   auto& editor = DovahKitCore::get();

   auto _setup_fade_intro_editor = [](
      WeatherIntpackedFloatEditor& handler,
      QSlider*        slider,
      QDoubleSpinBox* spinbox
   ) {
      handler.setWidgets(*slider, *spinbox);
      handler.setRange(0, 99.9);
   };
   auto _setup_fade_outro_editor = [](
      WeatherIntpackedFloatEditor& handler,
      QSlider*        slider,
      QDoubleSpinBox* spinbox
   ) {
      handler.setWidgets(*slider, *spinbox);
      handler.setRange(0.1, 100);
   };

   #pragma region General
      #pragma region Cloud layers
         ui::pair_slider_to_spinbox(this->ui.currentCloudLayerSpeedXSlider, this->ui.currentCloudLayerSpeedXSpinbox);
         ui::pair_slider_to_spinbox(this->ui.currentCloudLayerSpeedYSlider, this->ui.currentCloudLayerSpeedYSpinbox);
         ui::bind(this->ui.currentCloudLayerTexture, this->ui.currentCloudLayerTexturePreview);
      #pragma endregion
      #pragma region Wind
         {  // Wind direction
            auto& handler = this->_handlers.wind.direction;
            auto* slider  = this->ui.windDirSlider;
            auto* spinbox = this->ui.windDirSpinbox;
            handler.setWidgets(*slider, *spinbox);
            handler.setRange(0, 360);
         }
         {  // Wind direction range
            auto& handler = this->_handlers.wind.direction;
            auto* slider  = this->ui.windDirRangeSlider;
            auto* spinbox = this->ui.windDirRangeSpinbox;
            handler.setWidgets(*slider, *spinbox);
            handler.setRange(0, 180);
         }
         {  // Wind speed
            auto& handler = this->_handlers.wind.speed;
            auto* slider  = this->ui.windSpeedSlider;
            auto* spinbox = this->ui.windSpeedSpinbox;
            handler.setWidgets(*slider, *spinbox);
            handler.setRange(0, 1);
         }
      #pragma endregion
      #pragma region Sun
         {  // Sun Damage
            auto& handler = this->_handlers.sun_damage;
            auto* slider  = this->ui.sunDamageSlider;
            auto* spinbox = this->ui.sunDamageSpinbox;
            handler.setWidgets(*slider, *spinbox);
            handler.setRange(0, 1);
         }
         {  // Sun Glare
            auto& handler = this->_handlers.sun_glare;
            auto* slider  = this->ui.sunGlareSlider;
            auto* spinbox = this->ui.sunGlareSpinbox;
            handler.setWidgets(*slider, *spinbox);
            handler.setRange(0, 1);
         }
      #pragma endregion
      #pragma region Other
         {  // Trans Delta
            auto& handler = this->_handlers.trans_delta;
            auto* slider  = this->ui.transDeltaSlider;
            auto* spinbox = this->ui.transDeltaSpinbox;
            handler.setWidgets(*slider, *spinbox);

            slider->setRange(0, 0xFE); // override [0, 0xFF] default from the handler. [0, 0xFE] is consistent with CK.

            constexpr const std::string_view min_setting_name = "fTransDeltaMin";
            constexpr const std::string_view max_setting_name = "fTransDeltaMax";

            auto _update_bounds = [&handler, &min_setting_name, &max_setting_name]() {
               float min = _get_float_game_setting(min_setting_name);
               float max = _get_float_game_setting(max_setting_name);
               handler.setRange(min, max);
            };
            QObject::connect(&editor, &DovahKitCore::gameSettingValueChanged, this, [_update_bounds](const char* name_ptr) {
               std::string_view name = name_ptr;
               if (name == min_setting_name || name == max_setting_name)
                  _update_bounds();
            });
            _update_bounds();
         }
      #pragma endregion
   #pragma endregion
   #pragma region Colors
      this->ui.imagespaceSunrise->setAllowedFormType(dovah::form_type::imagespace);
      this->ui.imagespaceDaytime->setAllowedFormType(dovah::form_type::imagespace);
      this->ui.imagespaceSunset->setAllowedFormType(dovah::form_type::imagespace);
      this->ui.imagespaceNighttime->setAllowedFormType(dovah::form_type::imagespace);
   #pragma endregion
   #pragma region Precipitation
      this->ui.precipitationForm->setAllowedFormType(dovah::form_type::shader_particle_geometry_data);
      _setup_fade_intro_editor(this->_handlers.precipitation.fade_intro, this->ui.precipitationBeginFadeSlider, this->ui.precipitationBeginFadeSpinbox);
      _setup_fade_outro_editor(this->_handlers.precipitation.fade_outro, this->ui.precipitationEndFadeSlider,   this->ui.precipitationEndFadeSpinbox);
      _setup_fade_intro_editor(this->_handlers.thunderstorm.fade_intro, this->ui.thunderBeginFadeSlider, this->ui.thunderBeginFadeSpinbox);
      _setup_fade_outro_editor(this->_handlers.thunderstorm.fade_outro, this->ui.thunderEndFadeSlider,   this->ui.thunderEndFadeSpinbox);
      {
         auto& handler = this->_handlers.thunderstorm.frequency;
         auto* slider  = this->ui.thunderFrequencySlider;
         auto* spinbox = this->ui.thunderFrequencySpinbox;
         slider->setInvertedAppearance(true);
         handler.setWidgets(*slider, *spinbox);
         handler.setRange(0, 100);
      }
   #pragma endregion
   #pragma region Sounds
   this->ui.currentSoundForm->setAllowedFormType(dovah::form_type::sound_descriptor);
   {
      auto* widget = this->ui.currentSoundType;
      using enumeration = loaded_form_type::weather_sound_type;
      widget->clear();
      widget->addItem(tr("Default", "weather_sound_type"), (int)enumeration::default_);
      widget->addItem(tr("Precipitation", "weather_sound_type"), (int)enumeration::precipitation);
      widget->addItem(tr("Thunder", "weather_sound_type"), (int)enumeration::thunder);
      widget->addItem(tr("Wind", "weather_sound_type"), (int)enumeration::wind);
      widget->model()->sort(0);
   }
   {
      auto* view  = this->ui.sounds;
      auto* model = this->_models.sounds = new WeatherSoundsModel(this);
      view->setModel(model);
      ui::enable_inbound_drag_and_drop_insertions(view);
      ui::typical_tableview_config(view);
      view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      ui::set_tableview_column_flex(view, [](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(WeatherSoundsModel::Column::Sound, 1, 0);
         header.setColumnFlex(WeatherSoundsModel::Column::Type,  0, 0, metrics.boundingRect("Precipitationnnnn").width() * 1.5F + 4);
      });
      
      auto* sel_model = view->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model](const QItemSelection& selected, const QItemSelection& deselected) {
         bool empty = selected.empty() || selected[0].isEmpty();
         this->ui.buttonSoundsRemove->setEnabled(!empty);
         this->ui.currentSoundGroupbox->setEnabled(!empty);
         if (empty) {
            return;
         }

         auto  row  = selected[0].topLeft().row();
         auto* item = model->item(row);
         if (!item)
            return;

         const auto blockers = std::array{
            QSignalBlocker(this->ui.currentSoundForm),
            QSignalBlocker(this->ui.currentSoundType)
         };
         this->ui.currentSoundForm->setFormStub(item->sound);
         {
            auto i = this->ui.currentSoundType->findData((int)item->sound_type);
            if (i < 0)
               i = 0;
            this->ui.currentSoundType->setCurrentIndex(i);
         }
      });

      #pragma region List buttons
         QObject::connect(this->ui.buttonSoundsAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
            auto qmi = model->create();
            if (qmi.isValid()) {
               auto col = model->columnCount({});
               auto tl  = qmi.siblingAtColumn(0);
               auto br  = qmi.siblingAtColumn(col - 1);
               sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
            }
         });
         QObject::connect(this->ui.buttonSoundsRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
            size_t row;
            {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               row = rows[0].row();
            }
            model->deleteItems(row, 1);
         });
      #pragma endregion
      #pragma region Editing current sound
         QObject::connect(this->ui.currentSoundForm, &DKFormPicker::formChanged, this, [model, sel_model](dovah::form_stub* stub) {
            size_t row;
            {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               row = rows[0].row();
            }
            WeatherSoundsModel::node_type item;
            if (auto* src = model->item(row)) {
               item = *src;
            }
            item.sound = stub;
            model->overwrite(row, item);
         });
         QObject::connect(this->ui.currentSoundType, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, model, sel_model]() {
            size_t row;
            {
               auto rows = sel_model->selectedRows();
               if (rows.isEmpty())
                  return;
               row = rows[0].row();
            }
            WeatherSoundsModel::node_type item;
            if (auto* src = model->item(row)) {
               item = *src;
            }
            item.sound_type = (decltype(item.sound_type)) this->ui.currentSoundType->currentData().toInt();
            model->overwrite(row, item);
         });
      #pragma endregion
   }
   #pragma endregion
   #pragma region Effects
      this->ui.vfxForm->setAllowedFormType(dovah::form_type::visual_effect);
      _setup_fade_intro_editor(this->_handlers.visual_effect.fade_intro, this->ui.vfxBeginSlider, this->ui.vfxBeginSpinbox);
      _setup_fade_outro_editor(this->_handlers.visual_effect.fade_outro, this->ui.vfxEndSlider,   this->ui.vfxEndSpinbox);

      this->ui.skyStatics->setAllowedFormTypes({ dovah::form_type::statik });

      this->ui.volumetricSunrise->setAllowedFormType(dovah::form_type::volumetric_lighting);
      this->ui.volumetricDaytime->setAllowedFormType(dovah::form_type::volumetric_lighting);
      this->ui.volumetricSunset->setAllowedFormType(dovah::form_type::volumetric_lighting);
      this->ui.volumetricNighttime->setAllowedFormType(dovah::form_type::volumetric_lighting);
   #pragma endregion

   this->load(); // this creates the working copy.
}
void FormDialogWeather::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   QObject::connect(&editor, &DovahKitCore::dataSaveComplete, this, &FormDialogWeather::_update_ui_by_game);
   this->_update_ui_by_game();

   ui::bind(this->ui.editorID, this->editor_id());
   #pragma region General
      #pragma region Fog distances
         #pragma push_macro("BIND")
         #define BIND(data_name, ui_name) \
            ui::bind(this->ui.fogDist##ui_name##Day,   working.fog_distance.day.data_name); \
            ui::bind(this->ui.fogDist##ui_name##Night, working.fog_distance.night.data_name);
         //
         BIND(near,  Near);
         BIND(far,   Far);
         BIND(power, Power);
         BIND(max,   Max);
         //
         #undef BIND
         #pragma pop_macro("BIND")
      #pragma endregion
      #pragma region Wind
         this->_handlers.wind.direction.bindTo(working.wind.direction.base);
         this->_handlers.wind.direction_range.bindTo(working.wind.direction.variance);
         this->_handlers.wind.speed.bindTo(working.wind.speed);
      #pragma endregion
      #pragma region Sun
         this->_handlers.sun_damage.bindTo(working.sun.damage);
         this->_handlers.sun_glare.bindTo(working.sun.glare);
      #pragma endregion
      #pragma region Other parameters
         this->_handlers.trans_delta.bindTo(working.trans_delta);
      #pragma endregion
   #pragma endregion
   #pragma region Colors
      #pragma push_macro("BIND")
      #pragma region Basic colors
         #define BIND(data_name, ui_name) \
            ui::bind(this->ui.color##ui_name##Sunrise,   working.colors.data_name.sunrise); \
            ui::bind(this->ui.color##ui_name##Daytime,   working.colors.data_name.day); \
            ui::bind(this->ui.color##ui_name##Sunset,    working.colors.data_name.sunset); \
            ui::bind(this->ui.color##ui_name##Nighttime, working.colors.data_name.night);
         //
         BIND(ambient,           Ambient);
         BIND(cloud_lod_ambient, CloudLODAmbient);
         BIND(cloud_lod_diffuse, CloudLODDiffuse);
         BIND(effect_lighting,   Effect);
         BIND(fog_far,           FogFar);
         BIND(fog_near,          FogNear);
         BIND(moon_glare,        GlareMoon);
         BIND(sun_glare,         GlareSun);
         BIND(horizon,           Horizon);
         BIND(sky_lower,         SkyLower);
         BIND(sky_statics,       SkyStatics);
         BIND(sky_upper,         SkyUpper);
         BIND(stars,             Stars);
         BIND(sun,               Sun);
         BIND(sunlight,          Sunlight);
         BIND(unused,            Unused);
         BIND(water_multiplier,  Water);
         //
         #undef BIND
      #pragma endregion
      #pragma region Directional ambient colors
         #define BIND(data_name, ui_name) \
            ui::bind(this->ui.dalc##ui_name##Sunrise,   working.directional_ambient_lighting.sunrise.data_name); \
            ui::bind(this->ui.dalc##ui_name##Daytime,   working.directional_ambient_lighting.day.data_name); \
            ui::bind(this->ui.dalc##ui_name##Sunset,    working.directional_ambient_lighting.sunset.data_name); \
            ui::bind(this->ui.dalc##ui_name##Nighttime, working.directional_ambient_lighting.night.data_name);
         //
         BIND(fresnel,    Fresnel);
         BIND(specular,   Specular);
         BIND(x.negative, XNeg);
         BIND(x.positive, XPos);
         BIND(y.negative, YNeg);
         BIND(y.positive, YPos);
         BIND(z.negative, ZNeg);
         BIND(z.positive, ZPos);
         //
         #undef BIND
      #pragma endregion
      #pragma region Imagespaces
         ui::bind(this->ui.imagespaceSunrise, working.imagespaces.sunrise, working);
         ui::bind(this->ui.imagespaceDaytime, working.imagespaces.day, working);
         ui::bind(this->ui.imagespaceSunset, working.imagespaces.sunset, working);
         ui::bind(this->ui.imagespaceNighttime, working.imagespaces.night, working);
      #pragma endregion
      #pragma pop_macro("BIND")
   #pragma endregion
   #pragma region Precipitation
      #pragma region Metadata
         {
            auto* widget = this->ui.weatherClass;
            widget->clear();
            widget->addItem(tr("Pleasant"), loaded_form_type::weather_flag::class_pleasant);
            widget->addItem(tr("Cloudy"), loaded_form_type::weather_flag::class_cloudy);
            widget->addItem(tr("Rainy"), loaded_form_type::weather_flag::class_rainy);
            widget->addItem(tr("Snowy"), loaded_form_type::weather_flag::class_snowy);

            for (int i = 0; i < widget->count(); ++i) {
               auto flag = widget->itemData(i).toInt();
               if (working.flags & flag) {
                  widget->setCurrentIndex(i);
                  break;
               }
            }
            QObject::connect(widget, qOverload<int>(&QComboBox::currentIndexChanged), this, [this, widget]() {
               this->form->flags &= ~(
                  loaded_form_type::weather_flag::class_pleasant |
                  loaded_form_type::weather_flag::class_cloudy   |
                  loaded_form_type::weather_flag::class_rainy    |
                  loaded_form_type::weather_flag::class_snowy
               );
               this->form->flags |= widget->currentData().toInt();
            });
         }
      #pragma endregion
      #pragma region Precipitation
         ui::bind(this->ui.precipitationForm, working.precipitation.form, working);
         this->_handlers.precipitation.fade_intro.bindTo(working.precipitation.transition.intro.raw);
         this->_handlers.precipitation.fade_outro.bindTo(working.precipitation.transition.outro.raw);
      #pragma endregion
      #pragma region Thunder and Lightning
         this->_handlers.thunderstorm.fade_intro.bindTo(working.thunderstorm.transition.intro.raw);
         this->_handlers.thunderstorm.fade_outro.bindTo(working.thunderstorm.transition.outro.raw);
         this->_handlers.thunderstorm.frequency.bindTo(working.thunderstorm.frequency);
         {
            auto&  src = working.thunderstorm.lightning_color;
            QColor dst;
            dst.setRed(src.r);
            dst.setGreen(src.g);
            dst.setBlue(src.b);
            this->ui.lightningColor->setColor(dst);

            QObject::connect(this->ui.lightningColor, &DKColorPickerButton::colorChanged, this, [this](QColor src) {
               auto& dst = this->form->thunderstorm.lightning_color;
               dst.r = src.red();
               dst.g = src.green();
               dst.b = src.blue();
            });
         }
      #pragma endregion
   #pragma endregion
   #pragma region Sounds
   {
      auto* model = this->_models.sounds;

      std::vector<WeatherSoundsModel::node_type> nodes;
      nodes.reserve(working.sounds.size());
      for (const auto& item : working.sounds) {
         if (!item.form)
            continue;
         auto& node = nodes.emplace_back();
         node.sound      = item.form.get_form_stub();
         node.sound_type = item.type;
         if (node.sound && node.sound->form_type == dovah::form_type::sound) {
            auto loaded = node.sound->load().ptr_cast<dovah::loaded_forms::Sound>();
            if (loaded)
               node.sound = loaded->descriptor.get_form_stub();
         }
      }
      model->overwriteAllItems(nodes);
   }
   #pragma endregion
   #pragma region Effects
      #pragma region Visual Effect
         ui::bind(this->ui.vfxForm, working.visual_effect.form, working);
         this->_handlers.visual_effect.fade_intro.bindTo(working.visual_effect.transition.intro.raw);
         this->_handlers.visual_effect.fade_outro.bindTo(working.visual_effect.transition.outro.raw);
      #pragma endregion
      #pragma region Aurora
         this->ui.aurora->initializeFrom(working.aurora);
         ui::bind(this->ui.flagAuroraAlwaysVisible, working.flags, loaded_form_type::weather_flag::aurora_always_visible);
         ui::bind(this->ui.flagAuroraFollowsSun,    working.flags, loaded_form_type::weather_flag::aurora_follows_sun);
      #pragma endregion
      #pragma region Sky statics
         this->ui.skyStatics->pullStubs(working.sky_statics);
      #pragma endregion
      #pragma region Volumetric Lighting
         QObject::connect(this->ui.volumetricSunrise, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
            auto& working = *this->form;
            auto& opt     = working.volumetric;
            if (!opt.has_value()) {
               if (!stub)
                  return;
               opt.emplace();
            }
            auto& val = opt.value();
            val.sunrise.set(working, stub);
         });
         QObject::connect(this->ui.volumetricDaytime, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
            auto& working = *this->form;
            auto& opt     = working.volumetric;
            if (!opt.has_value()) {
               if (!stub)
                  return;
               opt.emplace();
            }
            auto& val = opt.value();
            val.day.set(working, stub);
         });
         QObject::connect(this->ui.volumetricSunset, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
            auto& working = *this->form;
            auto& opt     = working.volumetric;
            if (!opt.has_value()) {
               if (!stub)
                  return;
               opt.emplace();
            }
            auto& val = opt.value();
            val.sunset.set(working, stub);
         });
         QObject::connect(this->ui.volumetricNighttime, &DKFormPicker::formChanged, this, [this](dovah::form_stub* stub) {
            auto& working = *this->form;
            auto& opt     = working.volumetric;
            if (!opt.has_value()) {
               if (!stub)
                  return;
               opt.emplace();
            }
            auto& val = opt.value();
            val.night.set(working, stub);
         });
         if (working.volumetric.has_value()) {
            const auto blockers = std::array{
               QSignalBlocker(this->ui.volumetricSunrise),
               QSignalBlocker(this->ui.volumetricDaytime),
               QSignalBlocker(this->ui.volumetricSunset),
               QSignalBlocker(this->ui.volumetricNighttime),
            };
            auto& val = working.volumetric.value();
            this->ui.volumetricSunrise->setFormStub(val.sunrise.get_form_stub());
            this->ui.volumetricDaytime->setFormStub(val.day.get_form_stub());
            this->ui.volumetricSunset->setFormStub(val.sunset.get_form_stub());
            this->ui.volumetricNighttime->setFormStub(val.night.get_form_stub());
         }
      #pragma endregion
   #pragma endregion

   QObject::connect(this->ui.currentCloudLayerIndex, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogWeather::_pull_cloud_layer);
   _pull_cloud_layer(-1);
}
void FormDialogWeather::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   _push_cloud_layer(-1);
   
   #pragma region Sounds
   {
      auto* model = this->_models.sounds;
      auto& dst   = working.sounds;
      for (auto& item : dst)
         item.form.set(working, nullptr);
      dst.clear();

      const auto size = model->rowCount();
      if (size >= 0) {
         dst.reserve(size);
         for (int i = 0; i < size; ++i) {
            const auto* src_item = model->item(i);
            if (!src_item)
               continue;
            auto& dst_item = dst.emplace_back();
            dst_item.form.set(working, src_item->sound);
            dst_item.type = src_item->sound_type;
         }
      }
   }
   #pragma endregion
   #pragma region Effects
      #pragma region Aurora
         this->ui.aurora->commitTo(working.aurora, working);
      #pragma endregion
      #pragma region Sky statics
         this->ui.skyStatics->commitStubs(working.sky_statics, working);
      #pragma endregion
   #pragma endregion
}

void FormDialogWeather::_push_cloud_layer(int which) {
   if (which < 0) {
      //
      // Minus minimum in case we decide we want the layers to be numbered [1, 32] in the UI, 
      // even though they're indexed [0, 31] internally.
      //
      which = this->ui.currentCloudLayerIndex->value() - this->ui.currentCloudLayerIndex->minimum();
   } else {
      assert(which < 32);
   }

   {
      uint32_t mask    = 1 << which;
      auto&    target  = this->form->clouds.disabled_layers;
      bool     disable = !this->ui.currentCloudLayerEnabled->isChecked();
      if (disable) {
         target |= mask;
      } else {
         target &= ~mask;
      }
   }

   auto _set_color = [](dovah::loaded_forms::color_t& dst, QColor value) {
      dst.r = value.red();
      dst.g = value.green();
      dst.b = value.blue();
   };

   auto& layer = this->form->clouds.layers[which];
   layer.texture = this->ui.currentCloudLayerTexture->value().to_string().toStdString();
   layer.speed.x.set_float(this->ui.currentCloudLayerSpeedXSpinbox->value());
   layer.speed.y.set_float(this->ui.currentCloudLayerSpeedYSpinbox->value());
   _set_color(layer.time_of_day.sunrise.color, this->ui.currentCloudLayerColorSunrise->color());
   _set_color(layer.time_of_day.day.color, this->ui.currentCloudLayerColorDaytime->color());
   _set_color(layer.time_of_day.sunset.color, this->ui.currentCloudLayerColorSunset->color());
   _set_color(layer.time_of_day.night.color, this->ui.currentCloudLayerColorNighttime->color());
   layer.time_of_day.sunrise.alpha = this->ui.currentCloudLayerAlphaSunrise->value();
   layer.time_of_day.day.alpha = this->ui.currentCloudLayerAlphaDaytime->value();
   layer.time_of_day.sunset.alpha = this->ui.currentCloudLayerAlphaSunset->value();
   layer.time_of_day.night.alpha = this->ui.currentCloudLayerAlphaNighttime->value();
}
void FormDialogWeather::_pull_cloud_layer(int which) {
   if (which < 0) {
      //
      // Minus minimum in case we decide we want the layers to be numbered [1, 32] in the UI, 
      // even though they're indexed [0, 31] internally.
      //
      which = this->ui.currentCloudLayerIndex->value() - this->ui.currentCloudLayerIndex->minimum();
   } else {
      assert(which < 32);
   }

   this->ui.currentCloudLayerEnabled->setChecked(!(this->form->clouds.disabled_layers & (1 << which)));

   auto _set_color = [](const dovah::loaded_forms::color_t& src, DKColorPickerButton* widget) {
      QColor dst;
      dst.setRed(src.r);
      dst.setGreen(src.g);
      dst.setBlue(src.b);
      widget->setColor(dst);
   };

   const auto& layer = this->form->clouds.layers[which];
   this->ui.currentCloudLayerTexture->setValue(ui::types::game_file_path(layer.texture.data()));
   this->ui.currentCloudLayerSpeedXSpinbox->setValue((float)layer.speed.x);
   this->ui.currentCloudLayerSpeedYSpinbox->setValue((float)layer.speed.y);
   _set_color(layer.time_of_day.sunrise.color, this->ui.currentCloudLayerColorSunrise);
   _set_color(layer.time_of_day.day.color, this->ui.currentCloudLayerColorDaytime);
   _set_color(layer.time_of_day.sunset.color, this->ui.currentCloudLayerColorSunset);
   _set_color(layer.time_of_day.night.color, this->ui.currentCloudLayerColorNighttime);
   this->ui.currentCloudLayerAlphaSunrise->setValue(layer.time_of_day.sunrise.alpha);
   this->ui.currentCloudLayerAlphaDaytime->setValue(layer.time_of_day.day.alpha);
   this->ui.currentCloudLayerAlphaSunset->setValue(layer.time_of_day.sunset.alpha);
   this->ui.currentCloudLayerAlphaNighttime->setValue(layer.time_of_day.night.alpha);
}

void FormDialogWeather::_update_ui_by_game() {
   bool no_volumetric = DovahKitCore::get().get_current_game() == dovah::game::skyrim_classic;
   this->ui.volumetricGroupbox->setVisible(!no_volumetric);
}