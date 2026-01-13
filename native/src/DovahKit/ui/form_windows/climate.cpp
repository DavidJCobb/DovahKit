#include "./climate.h"
#include "widgets/DKFormNIFPicker.h"
#include "ui/utils/bind.h"
#include "ui/utils/enable_inbound_drag_and_drop_insertions.h"
#include "ui/utils/pair_slider_to_spinbox.h"
#include "ui/utils/set_tableview_column_flex.h"
#include "ui/utils/typical_tableview_config.h"
#include "./climate/ClimateWeathersModel.h"
#include "./climate/FormSubdialogClimateWeather.h"

FormDialogClimate::FormDialogClimate(dovah::form_stub& stub, QWidget* parent) : QDialog(parent) {
   this->initialize(stub);

   this->_models.weathers = new ClimateWeathersModel(this);
   {
      auto* widget = this->ui.weathers;
      auto* model  = this->_models.weathers;
      widget->setModel(model);
      ui::enable_inbound_drag_and_drop_insertions(widget);
      ui::typical_tableview_config(widget);
      widget->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
      ui::set_tableview_column_flex(widget, [](DKHeaderView& header, const QFontMetrics& metrics) {
         header.setColumnFlex(ClimateWeathersModel::Column::Weather, 2, 0);
         header.setColumnFlex(ClimateWeathersModel::Column::Chance,  0, 0, metrics.boundingRect("99999").width() * 1.5F + 4);
         header.setColumnFlex(ClimateWeathersModel::Column::Global,  1, 0);
      });

      auto* sel_model = widget->selectionModel();
      QObject::connect(this->ui.buttonWeatherAdd, &QPushButton::clicked, this, [this, model, sel_model]() {
         FormSubdialogClimateWeather dialog;
         if (dialog.exec() != QDialog::DialogCode::Accepted)
            return;
         auto qmi = model->create();
         if (qmi.isValid()) {
            model->overwrite(qmi.row(), dialog.value());
            auto col = model->columnCount({});

            auto tl = qmi.siblingAtColumn(0);
            auto br = qmi.siblingAtColumn(col - 1);
            sel_model->select({ tl, br }, QItemSelectionModel::SelectionFlag::ClearAndSelect);
         }
      });
      QObject::connect(this->ui.buttonWeatherEdit, &QPushButton::clicked, this, [this, model, sel_model]() {
         size_t row;
         {
            auto rows = sel_model->selectedRows();
            if (rows.isEmpty())
               return;
            row = rows[0].row();
         }
         const auto* node = model->item(row);
         if (!node)
            return;

         FormSubdialogClimateWeather dialog;
         dialog.setValue(*node);
         if (dialog.exec() == QDialog::DialogCode::Accepted) {
            model->overwrite(row, dialog.value());
         }
      });
      QObject::connect(this->ui.buttonWeatherRemove, &QPushButton::clicked, this, [this, model, sel_model]() {
         size_t row;
         {
            auto rows = sel_model->selectedRows();
            if (rows.isEmpty())
               return;
            row = rows[0].row();
         }
         model->deleteItems(row, 1);
      });
   }

   ui::bind(this->ui.sunTex, this->ui.sunTexPreview);
   ui::bind(this->ui.sunGlareTex, this->ui.sunGlareTexPreview);

   for (auto& pair : std::array{
      std::pair{ this->ui.sunTimeRiseBegin, this->ui.sunTimeRiseBeginText },
      std::pair{ this->ui.sunTimeRiseEnd,   this->ui.sunTimeRiseEndText },
      std::pair{ this->ui.sunTimeSetBegin,  this->ui.sunTimeSetBeginText },
      std::pair{ this->ui.sunTimeSetEnd,    this->ui.sunTimeSetEndText },
   }) {
      auto* slider = pair.first;
      auto* label  = pair.second;
      slider->setRange(0, 24 * 6); // the value is in increments of 10min
      QObject::connect(slider, QOverload<int>::of(&QSlider::valueChanged), this, [this, label](int v) {
         QString text;
         {
            int h = v / 6;
            int m = (v % 6) * 10;
            if (h >= 12) {
               if (h > 12)
                  h -= 12;
               text = tr("%1:%2 PM");
            } else {
               if (h == 0)
                  h = 12;
               text = tr("%1:%2 AM");
            }
            text = text.arg(h, 2, 10, QChar('0')).arg(m, 2, 10, QChar('0'));\
         }
         label->setText(text);
      });
      emit slider->valueChanged(slider->value());
   }

   ui::pair_slider_to_spinbox(this->ui.moonPhaseLengthSlider, this->ui.moonPhaseLengthSpinbox);
   QObject::connect(this->ui.moonPhaseLengthSpinbox, QOverload<int>::of(&QSpinBox::valueChanged), this, [this](int v) {
      this->ui.moonCycleDaysText->setText(tr("%1 days").arg(this->ui.moonPhaseLengthSpinbox->value() * 8));
   });

   this->load(); // this creates the working copy.
}
void FormDialogClimate::_load_impl() {
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   ui::bind(this->ui.editorID, this->editor_id());
   ui::bind(this->ui.sunTex, working.textures.sun);
   ui::bind(this->ui.sunGlareTex, working.textures.sun_glare);
   ui::bind(this->ui.sunTimeRiseBegin, working.timing.sunrise.begin.value);
   ui::bind(this->ui.sunTimeRiseEnd,   working.timing.sunrise.end.value);
   ui::bind(this->ui.sunTimeSetBegin,  working.timing.sunset.begin.value);
   ui::bind(this->ui.sunTimeSetEnd,    working.timing.sunset.end.value);
   this->ui.nightSky->initializeFrom(working.night_sky_nif);
   {
      loaded_form_type::moons moons;
      uint8_t phase_length;
      working.timing.moon_phase.split(moons, phase_length);

      auto* masser  = this->ui.flagMoonMasser;
      auto* secunda = this->ui.flagMoonSecunda;
      switch (moons) {
         case loaded_form_type::moons::none:
            masser->setChecked(false);
            secunda->setChecked(false);
            break;
         case loaded_form_type::moons::masser:
            masser->setChecked(true);
            secunda->setChecked(false);
            break;
         case loaded_form_type::moons::secunda:
            masser->setChecked(false);
            secunda->setChecked(true);
            break;
         case loaded_form_type::moons::both:
            masser->setChecked(true);
            secunda->setChecked(true);
            break;
      }
      this->ui.moonPhaseLengthSpinbox->setValue(phase_length);
   }
   ui::bind(this->ui.volatility, working.timing.volatility);

   {
      std::vector<ClimateWeathersModelNode> nodes;
      for (auto& item : working.weather_types) {
         auto& node = nodes.emplace_back();
         node.weather = item.weather.get_form_stub();
         node.global  = item.global.get_form_stub();
         node.chance  = item.chance;
      }
      this->_models.weathers->overwriteAllItems(nodes);
   }
}
void FormDialogClimate::_save_impl() {
   //
   // FormEditDialogMixin will handle the task of saving the working copy. We 
   // just have to save things not included in the working copy, namely form 
   // flags and the editor ID, as well as anything that doesn't modify the 
   // working copy in real-time (e.g. if a checkbox doesn't literally modify 
   // the working copy *as* it's (un)checked).
   //
   auto& editor  = DovahKitCore::get();
   auto& working = *this->form;

   this->ui.nightSky->commitTo(working.night_sky_nif, working);
   {  // Moons
      loaded_form_type::moons moons;
      uint8_t phase_length = this->ui.moonPhaseLengthSpinbox->value();
      {
         bool masser  = this->ui.flagMoonMasser->isChecked();
         bool secunda = this->ui.flagMoonSecunda->isChecked();
         if (masser && secunda) {
            moons = loaded_form_type::moons::both;
         } else if (masser) {
            moons = loaded_form_type::moons::masser;
         } else if (secunda) {
            moons = loaded_form_type::moons::secunda;
         } else {
            moons = loaded_form_type::moons::none;
         }
      }
      working.timing.moon_phase.join(moons, phase_length);
   }
   {  // Weathers
      auto&  dst_list = working.weather_types;
      size_t size = this->_models.weathers->rowCount();
      if (dst_list.size() < size)
         dst_list.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto* src = this->_models.weathers->item(i);
         auto& dst = dst_list[i];
         if (src) {
            dst.weather.set(working, src->weather);
            dst.global.set(working, src->global);
            dst.chance = src->chance;
         } else {
            dst.weather.set(working, nullptr);
            dst.global.set(working, nullptr);
            dst.chance = 0;
         }
      }
      if (dst_list.size() > size) {
         for (size_t i = size; i < dst_list.size(); ++i) {
            auto& item = dst_list[i];
            item.weather.set(working, nullptr);
            item.global.set(working, nullptr);
         }
         dst_list.resize(size);
      }
   }
}