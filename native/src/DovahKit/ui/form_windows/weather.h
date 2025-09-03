#pragma once
#include "./_base.h"
#include "dovah/forms/Weather.h"
#include "ui_weather.h"
#include "./weather/WeatherIntpackedFloatEditor.h"
class WeatherSoundsModel;

class FormDialogWeather :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::Weather, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogWeather(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogWeather ui;
      struct {
         WeatherIntpackedFloatEditor sun_damage;
         WeatherIntpackedFloatEditor sun_glare;
         WeatherIntpackedFloatEditor trans_delta;
         struct {
            WeatherIntpackedFloatEditor fade_intro;
            WeatherIntpackedFloatEditor fade_outro;
         } precipitation;
         struct {
            WeatherIntpackedFloatEditor fade_intro;
            WeatherIntpackedFloatEditor fade_outro;
            WeatherIntpackedFloatEditor frequency;
         } thunderstorm;
         struct {
            WeatherIntpackedFloatEditor fade_intro;
            WeatherIntpackedFloatEditor fade_outro;
         } visual_effect;
         struct {
            WeatherIntpackedFloatEditor direction;
            WeatherIntpackedFloatEditor direction_range;
            WeatherIntpackedFloatEditor speed;
         } wind;
      } _handlers;
      struct {
         WeatherSoundsModel* sounds = nullptr;
      } _models;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _push_cloud_layer(int which);
      void _pull_cloud_layer(int which);

      void _update_ui_by_game();
};
