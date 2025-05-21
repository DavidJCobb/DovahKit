#include "./music_type.h"
#include "dovah/data/game.h"
#include "ui/utils/bind.h"
#include "ui/utils/set_range.h"

FormDialogMusicType::FormDialogMusicType(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);
   
   QObject::connect(this->ui.prioritySlider, &QSlider::valueChanged, this, [this](int v) {
      const auto blocker = QSignalBlocker(this->ui.prioritySpinbox);
      this->ui.prioritySpinbox->setValue(v);
   });
   QObject::connect(this->ui.prioritySpinbox, qOverload<int>(&QSpinBox::valueChanged), this, [this](int v) {
      const auto blocker = QSignalBlocker(this->ui.prioritySlider);
      this->ui.prioritySlider->setValue(v);
   });

   ui::set_unsigned_range<float>(this->ui.fadeDuration);
   QObject::connect(this->ui.ducking, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
      this->form->ducking_db = v / 100.0;
   });

   this->load(); // this creates the working copy.
}
void FormDialogMusicType::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {  // Flags
      ui::bind(this->ui.flagAbrupt, working.flags, loaded_form_type::music_type_flag::abrupt_transition);
      ui::bind(this->ui.flagPlaysOne, working.flags, loaded_form_type::music_type_flag::plays_one_selection);
      ui::bind(this->ui.flagCycle, working.flags, loaded_form_type::music_type_flag::cycle_tracks);
      ui::bind(this->ui.flagMaintainOrder, working.flags, loaded_form_type::music_type_flag::maintain_track_order);
      ui::bind(this->ui.flagDucks, working.flags, loaded_form_type::music_type_flag::ducks_current_track);
      ui::bind(this->ui.flagDoesntQueue, working.flags, loaded_form_type::music_type_flag::does_not_queue);
      if (DovahKitCore::get().get_current_game() != dovah::game::skyrim_special) {
         this->ui.flagDoesntQueue->setVisible(false);
      }
   }
   ui::bind(this->ui.fadeDuration, working.fade_duration);
   ui::bind(this->ui.prioritySlider, working.priority);
   this->ui.ducking->setValue((double)working.ducking_db / 100.0);

   this->ui.musicTracks->pullStubs(working.tracks);
}
void FormDialogMusicType::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;
   
   this->ui.musicTracks->commitStubs(working.tracks, working);
}