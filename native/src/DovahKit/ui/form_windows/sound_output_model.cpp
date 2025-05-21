#include "./sound_output_model.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"

FormDialogSoundOutputModel::FormDialogSoundOutputModel(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   ui::item_indices_to_data(this->ui.directionalMode);
   QObject::connect(this->ui.directionalMode, qOverload<int>(&QComboBox::currentIndexChanged), this, [this](int i) {
      this->ui.dsoGroupbox->setEnabled(i == 1);
   });
   this->ui.dsoGroupbox->setEnabled(false);

   QObject::connect(this->ui.dsoCurrentChannel, qOverload<int>(&QComboBox::currentIndexChanged), this, &FormDialogSoundOutputModel::_pull_channel_to_ui);
   QObject::connect(this->ui.dsoL, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoR, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoC, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoRL, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoRR, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoBL, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoBR, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);
   QObject::connect(this->ui.dsoLFE, qOverload<int>(&QSpinBox::valueChanged), this, &FormDialogSoundOutputModel::_push_channel_from_ui);

   auto _link_slider_spinbox_f = [](DKFloatSlider* slider, QDoubleSpinBox* spinbox) {
      QObject::connect(slider, &DKFloatSlider::valueChanged, spinbox, &QDoubleSpinBox::setValue);
      QObject::connect(spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), slider, [slider](double v) {
         auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   };
   auto _link_slider_spinbox = [](QSlider* slider, QSpinBox* spinbox) {
      QObject::connect(slider, &QSlider::valueChanged, spinbox, &QSpinBox::setValue);
      QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), slider, [slider](int v) {
         auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   };
   _link_slider_spinbox_f(this->ui.attenuateDistanceMinSlider, this->ui.attenuateDistanceMinSpinbox);
   _link_slider_spinbox_f(this->ui.attenuateDistanceMaxSlider, this->ui.attenuateDistanceMaxSpinbox);
   _link_slider_spinbox(this->ui.attenuateCurve0Slider, this->ui.attenuateCurve0Spinbox);
   _link_slider_spinbox(this->ui.attenuateCurve1Slider, this->ui.attenuateCurve1Spinbox);
   _link_slider_spinbox(this->ui.attenuateCurve2Slider, this->ui.attenuateCurve2Spinbox);
   _link_slider_spinbox(this->ui.attenuateCurve3Slider, this->ui.attenuateCurve3Spinbox);
   _link_slider_spinbox(this->ui.attenuateCurve4Slider, this->ui.attenuateCurve4Spinbox);
   _link_slider_spinbox(this->ui.reverbSendSlider, this->ui.reverbSendSpinbox);

   this->load(); // this creates the working copy.
}
void FormDialogSoundOutputModel::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   {  // Flags
      ui::bind(this->ui.flagAttenuatesWithDistance, working.flags, loaded_form_type::sound_output_flag::attenuates_with_distance);
      ui::bind(this->ui.flagAllowRumble, working.flags, loaded_form_type::sound_output_flag::allow_rumble);
   }
   ui::bind(this->ui.directionalMode, working.type);

   ui::bind(this->ui.attenuateDistanceMinSpinbox, working.attenuation.distance.minimum);
   ui::bind(this->ui.attenuateDistanceMaxSpinbox, working.attenuation.distance.maximum);
   ui::bind(this->ui.attenuateCurve0Spinbox, working.attenuation.curve[0]);
   ui::bind(this->ui.attenuateCurve1Spinbox, working.attenuation.curve[1]);
   ui::bind(this->ui.attenuateCurve2Spinbox, working.attenuation.curve[2]);
   ui::bind(this->ui.attenuateCurve3Spinbox, working.attenuation.curve[3]);
   ui::bind(this->ui.attenuateCurve4Spinbox, working.attenuation.curve[4]);

   ui::bind(this->ui.reverbSendSpinbox, working.reverb_send);

   this->_pull_channel_to_ui();
}
void FormDialogSoundOutputModel::_save_impl() {
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

static constexpr const size_t channel_count = std::tuple_size_v<decltype(FormDialogSoundOutputModel::loaded_form_type::channels.all)>;

void FormDialogSoundOutputModel::_pull_channel_to_ui() {
   size_t which = this->ui.dsoCurrentChannel->currentIndex();
   if (which >= channel_count)
      return;

   const auto blockers = std::array{
      QSignalBlocker(this->ui.dsoL),
      QSignalBlocker(this->ui.dsoR),
      QSignalBlocker(this->ui.dsoC),
      QSignalBlocker(this->ui.dsoRL),
      QSignalBlocker(this->ui.dsoRR),
      QSignalBlocker(this->ui.dsoBL),
      QSignalBlocker(this->ui.dsoBR),
      QSignalBlocker(this->ui.dsoLFE),
   };

   auto& src = this->form->channels.all[which];
   this->ui.dsoL->setValue(src.left);
   this->ui.dsoR->setValue(src.right);
   this->ui.dsoC->setValue(src.center);
   this->ui.dsoRL->setValue(src.surround_left);
   this->ui.dsoRR->setValue(src.surround_right);
   this->ui.dsoBL->setValue(src.rear_surround_left);
   this->ui.dsoBR->setValue(src.rear_surround_right);
   this->ui.dsoLFE->setValue(src.low_frequency_effects);
}
void FormDialogSoundOutputModel::_push_channel_from_ui() {
   size_t which = this->ui.dsoCurrentChannel->currentIndex();
   if (which >= channel_count)
      return;

   auto& src = this->form->channels.all[which];
   src.left = this->ui.dsoL->value();
   src.right = this->ui.dsoR->value();
   src.center = this->ui.dsoC->value();
   src.surround_left = this->ui.dsoRL->value();
   src.surround_right = this->ui.dsoRR->value();
   src.rear_surround_left = this->ui.dsoBL->value();
   src.rear_surround_right = this->ui.dsoBR->value();
   src.low_frequency_effects = this->ui.dsoLFE->value();
}