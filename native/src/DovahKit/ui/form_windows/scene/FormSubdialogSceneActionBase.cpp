#include "./FormSubdialogSceneActionBase.h"

FormSubdialogSceneActionBase::FormSubdialogSceneActionBase(QWidget* parent) : QDialog(parent) {
}

void FormSubdialogSceneActionBase::_make_alias_picker(QComboBox& widget, uint32_t alias_id) {
   widget.clear();
      
   int i = -1;
   for (auto& item : this->scene_data.actors) {
      if (item.first == alias_id)
         i = widget.count();
      widget.addItem(item.second, item.first);
   }
   widget.setCurrentIndex(i);
}

void FormSubdialogSceneActionBase::_refresh_base(
   QLineEdit* name,
   QComboBox* actor,
   QComboBox* phase_start,
   QComboBox* phase_end
) {
   std::sort(
      this->scene_data.phases.begin(),
      this->scene_data.phases.end(),
      [](const auto& a, const auto& b) {
         return a.first < b.first;
      }
   );

   name->setText(this->base_data.name);
   this->_make_alias_picker(*actor, this->base_data.alias_id);
   {
      phase_start->clear();
      for (auto& pair : this->scene_data.phases) {
         phase_start->addItem(pair.second, (int)pair.first);
      }
      int i = phase_start->findData(this->base_data.phase_indices.start);
      if (i >= 0)
         phase_start->setCurrentIndex(i);
   }
   if (!this->_set_up_signals) {
      QObject::connect(phase_start, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, phase_start, phase_end]() {
         uint32_t min = phase_start->currentData().toInt();
         uint32_t max = min;
         for (auto& pair : this->scene_data.phases) {
            if (pair.first <= min)
               continue;
            if (pair.first > max + 1)
               break;
            max = pair.first;
         }

         auto current_end = phase_end->currentData();
         phase_end->clear();
         for (auto& pair : this->scene_data.phases) {
            if (pair.first < min || pair.first > max)
               continue;
            phase_end->addItem(pair.second, (int)pair.first);
         }
         if (current_end.isValid()) {
            int i = phase_end->findData(current_end);
            if (i < 0)
               i = phase_end->findData(min);
            if (i >= 0)
               phase_end->setCurrentIndex(i);
         }
      });
   }
   {
      emit phase_start->currentIndexChanged(phase_start->currentIndex());
      //
      int i = phase_end->findData(this->base_data.phase_indices.end);
      if (i < 0)
         i = phase_end->findData(this->base_data.phase_indices.start);
      if (i >= 0)
         phase_end->setCurrentIndex(i);
   }

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