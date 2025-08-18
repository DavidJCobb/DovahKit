#pragma once
#include "./_base.h"
#include "dovah/forms/MusicTrack.h"
#include "ui_music_track.h"

class MusicTrackCuePointsModel;

class FormDialogMusicTrack :
   public QDialog,
   public FormEditDialogMixin<dovah::loaded_forms::MusicTrack, true>
{
   Q_OBJECT;
   DOVAHKIT_FORM_EDIT_DIALOG;
   public:
      FormDialogMusicTrack(dovah::form_stub& stub, QWidget* parent = nullptr);
      
   protected:
      Ui::FormDialogMusicTrack ui;
      struct {
         MusicTrackCuePointsModel* cue_points = nullptr;
      } _models;
      struct {
         struct {
            size_t displayed_layer = 0;
            std::array<std::vector<dovah::form_stub*>, loaded_form_type::max_palette_layer_count> tracks_by_layer;
         } palette;
      } _state;
      
      virtual void _load_impl() override;
      virtual void _save_impl() override;

      void _remember_displayed_palette_layer();
      void _update_displayed_palette_layer();
};
