#include "./music_track.h"
#include "dovah/data/music_track_type.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"
#include "ui/utils/form_list_pane_columns/music_track_type.h"
#include "./music_track/MusicTrackCuePointsModel.h"

FormDialogMusicTrack::FormDialogMusicTrack(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      auto* widget = this->ui.trackType;
      widget->clear();
      QObject::connect(widget, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this]() {
         auto type = (dovah::music_track_type)this->ui.trackType->currentData().toInt();
         this->ui.tabPaletteTrack->setEnabled(type == dovah::music_track_type::palette);
         this->ui.tabSilentTrack->setEnabled(type == dovah::music_track_type::silent);
         this->ui.tabSingleTrack->setEnabled(type == dovah::music_track_type::single);
         switch (type) {
            case dovah::music_track_type::palette:
               this->ui.tabview->setCurrentWidget(this->ui.tabPaletteTrack);
               break;
            case dovah::music_track_type::silent:
               this->ui.tabview->setCurrentWidget(this->ui.tabSilentTrack);
               break;
            case dovah::music_track_type::single:
               this->ui.tabview->setCurrentWidget(this->ui.tabSingleTrack);
               break;
         }
      });
      widget->addItem(tr("Palette Track"), (int)dovah::music_track_type::palette);
      widget->addItem(tr("Silent Track"), (int)dovah::music_track_type::silent);
      widget->addItem(tr("Single Track"), (int)dovah::music_track_type::single);
   }

   ui::set_unsigned_range<int32_t>(this->ui.loopCount);
   ui::set_unsigned_range<float>(this->ui.loopStartTime);
   ui::set_unsigned_range<float>(this->ui.loopEndTime);
   ui::set_unsigned_range<float>(this->ui.currentCue);
   {
      auto* widget = this->ui.cueList;
      auto* model  = new MusicTrackCuePointsModel(this);
      this->_models.cue_points = model;
      widget->setModel(model);

      widget->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
      widget->setHorizontalScrollMode(QAbstractItemView::ScrollMode::ScrollPerPixel);
      widget->setVerticalScrollMode(QAbstractItemView::ScrollMode::ScrollPerItem);

      auto* sel_model = widget->selectionModel();
      QObject::connect(sel_model, &QItemSelectionModel::selectionChanged, this, [this, model, sel_model]() {
         auto* editor = this->ui.currentCue;

         QModelIndex qmi;
         {
            auto rows = sel_model->selectedRows();
            if (rows.empty()) {
               editor->setEnabled(false);
               this->ui.buttonRemoveCue->setEnabled(false);
               return;
            }
            editor->setEnabled(true);
            this->ui.buttonRemoveCue->setEnabled(true);
            if (!rows.empty())
               qmi = rows[0];
         }
         const auto data = model->data(qmi, Qt::UserRole).toFloat();

         const auto blocker = QSignalBlocker(editor);
         editor->setValue(data);
      });
      QObject::connect(this->ui.currentCue, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this, model, sel_model](double v) {
         QModelIndex qmi;
         {
            auto rows = sel_model->selectedRows();
            if (!rows.empty())
               qmi = rows[0];
         }
         model->setData(qmi, v, Qt::UserRole);
      });

      QObject::connect(this->ui.buttonAddCue, &QPushButton::clicked, this, [this, model, sel_model]() {
         auto qmi = model->append();
         sel_model->setCurrentIndex(qmi, QItemSelectionModel::SelectionFlag::ClearAndSelect);
      });
      QObject::connect(this->ui.buttonRemoveCue, &QPushButton::clicked, this, [this, model, sel_model]() {
         QModelIndex qmi;
         {
            auto rows = sel_model->selectedRows();
            if (!rows.empty())
               qmi = rows[0];
         }
         model->removeRow(qmi.row());
      });
   }

   ui::set_unsigned_range<float>(this->ui.paletteDuration);
   ui::set_unsigned_range<float>(this->ui.paletteFadeOutTime);
   this->ui.paletteCurrentLayer->setRange(1, loaded_form_type::max_palette_layer_count);
   this->ui.paletteTrackList->setAllowedFormTypes({ dovah::form_type::music_track });
   ui::form_list_pane_columns::music_track_type(*this->ui.paletteTrackList);
   QObject::connect(this->ui.paletteCurrentLayer, qOverload<int>(&QSpinBox::valueChanged), this, [this]() {
      this->_remember_displayed_palette_layer();
      this->_update_displayed_palette_layer();
   });

   ui::set_unsigned_range<float>(this->ui.silentDuration);

   this->load(); // this creates the working copy.
}
void FormDialogMusicTrack::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {
      auto* widget = this->ui.trackType;
      auto  i      = widget->findData((int)working.get_track_type());
      if (i < 0)
         i = 0;
      widget->setCurrentIndex(i);
   }
   switch (working.get_track_type()) {
      case dovah::music_track_type::palette:
         {
            auto& casted = std::get<loaded_form_type::palette_data>(working.data);
            this->ui.paletteDuration->setValue(casted.duration);
            this->ui.paletteFadeOutTime->setValue(casted.fade_out);
            {
               auto& src_layers = casted.tracks_by_layer;
               auto& dst_layers = this->_state.palette.tracks_by_layer;
               size_t size = src_layers.size();
               for (size_t i = 0; i < size; ++i) {
                  auto& src_layer = src_layers[i];
                  auto& dst_layer = dst_layers[i];
                  size_t count = src_layer.size();
                  dst_layer.resize(count);
                  for (size_t i = 0; i < count; ++i)
                     dst_layer[i] = src_layer[i].get_form_stub();
               }
               this->_update_displayed_palette_layer();
            }
         }
         break;
      case dovah::music_track_type::silent:
         {
            auto& casted = std::get<loaded_form_type::silent_data>(working.data);
            this->ui.silentDuration->setValue(casted.duration);
         }
         break;
      case dovah::music_track_type::single:
         {
            auto& casted = std::get<loaded_form_type::single_data>(working.data);
            {
               const auto& src = casted.filenames.main;
               auto dst = ui::types::game_file_path(src.data());
               this->ui.fileMain->setValue(dst);
            }
            {
               const auto& src = casted.filenames.finale;
               auto dst = ui::types::game_file_path(src.data());
               this->ui.fileFinale->setValue(dst);
            }
            {  // Contains loop
               auto& opt = casted.loop;
               if (opt.has_value()) {
                  this->ui.flagLoops->setChecked(true);
                  auto& val = opt.value();
                  this->ui.loopStartTime->setValue(val.begin);
                  this->ui.loopEndTime->setValue(val.end);
                  this->ui.loopCount->setValue(val.count);
               } else {
                  this->ui.flagLoops->setChecked(false);
               }
            }
            this->_models.cue_points->importData(casted.cue_points);
         }
         break;
   }
}
void FormDialogMusicTrack::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   working.set_track_type((dovah::music_track_type)this->ui.trackType->currentData().toInt());
   {
      auto& typed_data = working.data;
      if (auto* casted = std::get_if<loaded_form_type::palette_data>(&typed_data)) {
         casted->duration = this->ui.paletteDuration->value();
         casted->fade_out = this->ui.paletteFadeOutTime->value();
         {  // Tracks by layer
            this->_remember_displayed_palette_layer();
            casted->set_all_tracks(working, this->_state.palette.tracks_by_layer);
         }
      } else if (auto* casted = std::get_if<loaded_form_type::silent_data>(&typed_data)) {
         casted->duration = this->ui.silentDuration->value();
      } else if (auto* casted = std::get_if<loaded_form_type::single_data>(&typed_data)) {
         casted->filenames.main   = this->ui.fileMain->value().to_string().toStdString();
         casted->filenames.finale = this->ui.fileFinale->value().to_string().toStdString();
         {  // Contains loop
            auto& dst = casted->loop;
            if (this->ui.flagLoops->isChecked()) {
               auto& val = dst.emplace();
               val.begin = this->ui.loopStartTime->value();
               val.end   = this->ui.loopEndTime->value();
               val.count = this->ui.loopCount->value();
            } else {
               dst.reset();
            }
         }
         this->_models.cue_points->exportData(casted->cue_points);
      }
   }
   this->ui.conditions->exportTo(working, working.conditions);
}

void FormDialogMusicTrack::_remember_displayed_palette_layer() {
   auto& layer_set = this->_state.palette.tracks_by_layer;
   auto  layer_idx = this->_state.palette.displayed_layer;
   if (layer_idx >= layer_set.size()) {
      return;
   }
   auto& layer = layer_set[layer_idx];
   auto  stubs = this->ui.paletteTrackList->stubs();
   layer = { stubs.begin(), stubs.end() };
}
void FormDialogMusicTrack::_update_displayed_palette_layer() {
   auto& layer_set = this->_state.palette.tracks_by_layer;
   auto  layer_idx = this->ui.paletteCurrentLayer->value() - 1;
   if (layer_idx >= layer_set.size()) {
      return;
   }
   auto& layer = layer_set[layer_idx];
   this->ui.paletteTrackList->setUpdatesEnabled(false);
   this->ui.paletteTrackList->clear();
   for (auto* stub : layer)
      this->ui.paletteTrackList->addStub(stub);
   this->ui.paletteTrackList->setUpdatesEnabled(true);
   this->_state.palette.displayed_layer = layer_idx;
}