#include "./FormSubdialogSceneActionBase.h"

FormSubdialogSceneActionBase::FormSubdialogSceneActionBase(QWidget* parent) : QDialog(parent) {
}

void FormSubdialogSceneActionBase::_refresh_base(
   QLineEdit* name,
   QComboBox* actor,
   QComboBox* phase_start,
   QComboBox* phase_end
) {
   auto _make_phase_list = [this](QComboBox* widget, size_t value) {
      widget->clear();

      auto&  list = this->scene_data.phases;
      size_t size = list.size();
      for (size_t i = 0; i < size; ++i) {
         widget->addItem(list[i], (int)i);
      }
      widget->setCurrentIndex(value);
   };

   name->setText(this->base_data.name);
   {
      auto* widget = actor;
      widget->clear();
      
      int i = -1;
      for (auto& item : this->scene_data.actors) {
         if (item.first == this->base_data.alias_id)
            i = widget->count();
         widget->addItem(item.second, item.first);
      }
      widget->setCurrentIndex(i);
   }
   _make_phase_list(phase_start, this->base_data.phase_indices.start);
   _make_phase_list(phase_end,   this->base_data.phase_indices.end);
   
   if (!this->_set_up_signals) {
      this->_set_up_signals = true;
      QObject::connect(this, &QDialog::accepted, this, [=]() {
         this->base_data.name     = name->text();
         this->base_data.alias_id = actor->currentData().toInt();
         this->base_data.phase_indices = {
            .start = (uint32_t)phase_start->currentData().toInt(),
            .end   = (uint32_t)phase_end->currentData().toInt(),
         };
      });
   }
}