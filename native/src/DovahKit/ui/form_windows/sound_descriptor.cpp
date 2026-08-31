#include "./sound_descriptor.h"
#include "ui/utils/bind.h"
#include "ui/utils/item_indices_to_data.h"
#include "ui/utils/set_range.h"
#include "./sound_descriptor/SoundDescriptorSoundFilesModel.h"

FormDialogSoundDescriptor::FormDialogSoundDescriptor(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   {
      using enumeration = loaded_form_type::descriptor_type;
      auto* widget = this->ui.descriptorType;
      widget->clear();
      widget->addItem(tr("Standard"), (int)enumeration::standard);
   }
   this->ui.category->setAllowedFormType(dovah::form_type::sound_category);
   this->ui.outputModel->setAllowedFormType(dovah::form_type::sound_output_model);
   this->ui.alternateFor->setAllowedFormType(dovah::form_type::sound_descriptor);

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
      this->_models.files = model;
      widget->setModel(model);
      
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
      QObject::connect(this->ui.buttonRemoveFile, &QPushButton::clicked, this, [widget, model]() {
         auto* sm   = widget->selectionModel();
         auto  rows = sm->selectedRows();
         if (rows.empty())
            return;
         model->deleteItems(rows[0].row(), 1);
      });

      auto* sm = widget->selectionModel();
      QObject::connect(sm, &QItemSelectionModel::selectionChanged, this, &FormDialogSoundDescriptor::_pull_file_to_ui);
      QObject::connect(this->ui.filePicker, &DKGameFilePicker::valueChanged, this, &FormDialogSoundDescriptor::_on_file_path_edited);
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

   this->_pull_file_to_ui();
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

void FormDialogSoundDescriptor::_pull_file_to_ui() {
   const auto blocker = QSignalBlocker(this->ui.filePicker);

   QModelIndex qmi;
   {
      auto* sm  = this->ui.fileList->selectionModel();
      auto  sel = sm->selectedRows();
      if (!sel.empty())
         qmi = sel[0];
   }
   bool has_selection = qmi.isValid();
   this->ui.buttonRemoveFile->setEnabled(has_selection);
   this->ui.filePlayer->setEnabled(has_selection);
   if (!has_selection) {
      this->ui.filePlayer->setPath({});
      this->ui.filePicker->setValue({});
      return;
   }

   ui::types::game_file_path root_relative_path;
   {
      //
      // The game seems to treat the "Data\\Sound\\" prefix as optional, so we have to 
      // account for it potentially only being partially present. A particularly major 
      // example is [SNDR:0003F206]MagShockFFFireSD, which includes a prefixed path and 
      // an unprefixed path together! The Creation Kit seems to always prefix new paths, 
      // though.
      // 
      // Some forms have a "\\Data\\Sound\\" prefix. The CK does not currently insert a 
      // leading slash. I assume these paths are old or otherwise edge-casey.
      // 
      // `BGSStandardSoundDef::LoadSound`, which loads the bulk of SNDR's subrecords, 
      // will check if a path in ANAM contains any directory named "sound\\"; if so, 
      // the path stored in memory is scoped to that directory; otherwise, the directory 
      // is prepended. So for example:
      // 
      //  - "data\\sound\\foo" becomes "sound\\foo"
      //  - "sound\\foo"       remains "sound\\foo"
      //  - "foo"              becomes "sound\\foo"
      //  - "lmao\\sound\\foo" becomes "sound\\foo"
      // 
      // This prefix check properly recognizes all directory separators (i.e. forward- 
      // and backslashes).
      // 
      // Later, when the game actually wants to load WAV/XWM/FUZ files, it will check 
      // for the case-insensitive prefix "data\\sound\\", as a substring (so it doesn't 
      // recognize forward slashes). If that prefix is found, then the above logic is 
      // run to scope the path to "sound\\", properly recognizing all separators; 
      // otherwise, the above logic is run to scope the path to "music\\".
      //
      auto* node = this->_models.files->item(qmi.row());
      assert(!!node);
      auto  path = node->filepath;

      auto _view_starts_with_dir = [](QStringView view, QLatin1StringView dir) {
         if (view.size() < dir.size() + 1)
            return false;
         if (!view.startsWith(dir, Qt::CaseInsensitive))
            return false;
         auto c = view[dir.size()];
         return c == '/' || c == '\\';
      };

      auto view = QStringView(path);
      if (view[0] == '\\')
         view = view.mid(1);

      if (_view_starts_with_dir(view, QLatin1StringView("data"))) {
         auto next = view.mid(5);
         if (!_view_starts_with_dir(next, QLatin1StringView("sound"))) {
            root_relative_path.append("Sound\\");
         }
      } else {
         root_relative_path.append("Data\\Sound\\");
      }
      root_relative_path.append(path);
   }

   this->ui.filePicker->setValue(root_relative_path);
   this->ui.filePlayer->setPath(root_relative_path.lexically_relative("Data\\").to_string());
}
void FormDialogSoundDescriptor::_on_file_path_edited() {
   auto path = this->ui.filePicker->value();

   QModelIndex qmi;
   {
      auto* sm = this->ui.fileList->selectionModel();
      auto  sel = sm->selectedRows();
      if (!sel.empty())
         qmi = sel[0];
   }
   if (!qmi.isValid())
      return;

   SoundDescriptorSoundFilesModelNode node;
   {
      auto* prior = this->_models.files->item(qmi.row());
      assert(!!prior);
      node = *prior;
   }
   if (!node.filepath.startsWith("Data\\Sound\\", Qt::CaseInsensitive)) {
      node.filepath = QString("Data\\Sound\\") + node.filepath;
   }
   this->_models.files->overwrite(qmi.row(), node);

   this->ui.filePlayer->setPath(path.to_string());
}