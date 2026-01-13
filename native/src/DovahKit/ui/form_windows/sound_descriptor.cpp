#include "./sound_descriptor.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"
#include "./sound_descriptor/SoundDescriptorSoundFilesModel.h"

FormDialogSoundDescriptor::FormDialogSoundDescriptor(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->ui.category->setAllowedFormType(dovah::form_type::sound_category);
   this->ui.outputModel->setAllowedFormType(dovah::form_type::sound_output_model);
   this->ui.alternateFor->setAllowedFormType(dovah::form_type::sound_descriptor);

   ui::item_indices_to_data(this->ui.descriptorType);
   {
      using enumeration = loaded_form_type::loop_type;
      auto* widget = this->ui.looping;
      widget->clear();
      widget->addItem(tr("None"), 0);
      widget->addItem(tr("Looping"), (int)enumeration::loop);
      widget->addItem(tr("Envelope Slow"), (int)enumeration::envelope_slow);
      widget->addItem(tr("Envelope Fast"), (int)enumeration::envelope_fast);
   }

   {  // Static Attenuation (dB)
      constexpr const auto maximum    = std::numeric_limits<decltype(dovah::loaded_forms::SoundDescriptor::static_attenuation)>::max();
      constexpr const auto conversion = 100.0F;

      auto* slider  = this->ui.staticAttenuationSlider;
      auto* spinbox = this->ui.staticAttenuationSpinbox;
      slider->setRange(0, maximum / conversion);
      spinbox->setRange(0, maximum / conversion);
      slider->setDecimals(2);
      spinbox->setDecimals(2);
      QObject::connect(slider, &DKFloatSlider::valueChanged, this, [spinbox](float v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [slider](double v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }
   {  // Priority
      constexpr const auto maximum = std::numeric_limits<decltype(dovah::loaded_forms::SoundDescriptor::priority)>::max();

      auto* slider  = this->ui.prioritySlider;
      auto* spinbox = this->ui.prioritySpinbox;
      ui::set_range<uint8_t>(slider);
      ui::set_range<uint8_t>(spinbox);
      QObject::connect(slider, &QSlider::valueChanged, this, [spinbox](float v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), this, [slider](int v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }
   {  // Rumble Send, Small
      auto* slider  = this->ui.rumbleSmallSlider;
      auto* spinbox = this->ui.rumbleSmallSpinbox;
      slider->setRange(0, 0b1111 * 7);
      spinbox->setRange(0, 0b1111 * 7);
      slider->setSingleStep(7);
      spinbox->setSingleStep(7);
      QObject::connect(slider, &QSlider::valueChanged, this, [spinbox](float v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), this, [slider](int v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }
   {  // Rumble Send, Large
      auto* slider  = this->ui.rumbleLargeSlider;
      auto* spinbox = this->ui.rumbleLargeSpinbox;
      slider->setRange(0, 0b1111 * 7);
      spinbox->setRange(0, 0b1111 * 7);
      slider->setSingleStep(7);
      spinbox->setSingleStep(7);
      QObject::connect(slider, &QSlider::valueChanged, this, [spinbox](float v) {
         spinbox->setValue(v);
      });
      QObject::connect(spinbox, qOverload<int>(&QSpinBox::valueChanged), this, [slider](int v) {
         const auto blocker = QSignalBlocker(slider);
         slider->setValue(v);
      });
   }

   {
      auto* widget = this->ui.fileList;
      auto* model  = new SoundDescriptorSoundFilesModel(widget);
      widget->setModel(model);

      QObject::connect(this->ui.buttonRemoveFile, &QPushButton::clicked, this, [widget, model]() {
         auto* sm   = widget->selectionModel();
         auto  rows = sm->selectedRows();
         if (rows.empty())
            return;
         model->deleteItems(rows[0].row(), 1);
      });
      QObject::connect(this->ui.buttonAddFile, &QPushButton::clicked, this, [this, model]() {
         auto* picker = this->ui.filePicker;
         auto  path   = picker->value();
         if (path.empty())
            return;

         auto qmi = model->create();
         if (qmi.isValid()) {
            SoundDescriptorSoundFilesModelNode data;
            data.filepath = path.lexically_relative("Data\\Sound\\").to_string();
            model->overwrite(qmi.row(), data);

            picker->clear();
         }
      });
   }

   this->load(); // this creates the working copy.
}

void FormDialogSoundDescriptor::forceRefreshParentCategory() {
   auto* widget  = this->ui.category;
   auto  blocker = QSignalBlocker(widget);
   widget->setFormStub(this->form->category.get_form_stub());
}

void FormDialogSoundDescriptor::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.descriptorType, working.type);
   this->ui.conditions->importFrom(working, working.conditions);

   {
      auto* widget = this->ui.fileList;
      auto* model  = dynamic_cast<SoundDescriptorSoundFilesModel*>(widget->model());
      assert(!!model);
      model->importItems(working.sound_files);
   }

   ui::bind(this->ui.category, working.category, working);
   ui::bind(this->ui.outputModel, working.output_model, working);
   {
      auto* widget = this->ui.staticAttenuationSpinbox;
      widget->setValue(working.static_attenuation / 100.0);
      QObject::connect(widget, qOverload<double>(&QDoubleSpinBox::valueChanged), this, [this](double v) {
         this->form->static_attenuation = v * 100.0F;
      });
   }
   ui::bind(this->ui.dbVariance, working.db_variance);
   ui::bind(this->ui.looping, working.length_characteristics.type);
   ui::bind(this->ui.alternateFor, working.alternate_for, working);
   ui::bind(this->ui.freqShift, working.frequency.shift);
   ui::bind(this->ui.freqVariance, working.frequency.variance);
   ui::bind(this->ui.prioritySpinbox, working.priority);
   ui::bind(this->ui.rumbleSmallSpinbox, working.length_characteristics.rumble_send.small);
   ui::bind(this->ui.rumbleLargeSpinbox, working.length_characteristics.rumble_send.large);
}
void FormDialogSoundDescriptor::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->ui.conditions->exportTo(working, working.conditions);
   {
      auto* widget = this->ui.fileList;
      auto* model  = dynamic_cast<SoundDescriptorSoundFilesModel*>(widget->model());
      assert(!!model);
      model->exportItems(working.sound_files);
   }
}