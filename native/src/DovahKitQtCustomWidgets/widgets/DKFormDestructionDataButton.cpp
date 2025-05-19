#include "./DKFormDestructionDataButton.h"
#include <QBoxLayout>
#if !defined(QT_PLUGIN)
   #include "./widget-dialogs/DKFormDestructionDataDialog.h"
#endif

DKFormDestructionDataButton::DKFormDestructionDataButton(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);

   layout->setContentsMargins(0, 0, 0, 0);

   auto* button = this->_button = new QPushButton(tr("Add Destruction Data"), this);
   layout->addWidget(button);

   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(button);

   #if !defined(QT_PLUGIN)
      QObject::connect(button, &QPushButton::clicked, this, [this]() {
         auto* dialog = new DKFormDestructionDataDialog(this);
         dialog->setData(this->_value);

         auto result = dialog->exec();
         if (result == QDialog::DialogCode::Rejected)
            return;

         this->_value = dialog->data();
         if (this->_value.has_value()) {
            this->_button->setText(tr("Edit Destruction Data"));
         } else {
            this->_button->setText(tr("Add Destruction Data"));
         }
      });
   #endif
}

#if !defined(QT_PLUGIN)
void DKFormDestructionDataButton::initializeFrom(const std::optional<form_data_type>& src_opt) {
   if (src_opt.has_value()) {
      auto& src = src_opt.value();
      auto& dst = this->_value.emplace();
      
      dst.health = src.health;
      dst.flags.vats_enabled = (src.flags & form_data_type::data_flag::vats_enabled) != 0;

      dst.stages.resize(src.stages.size());
      for (size_t i = 0; i < src.stages.size(); ++i) {
         auto& src_stage = src.stages[i];
         auto& dst_stage = dst.stages[i];

         dst_stage.damage_stage     = src_stage.damageStage;
         dst_stage.debris           = src_stage.debris.get_form_stub();
         dst_stage.debris_count     = src_stage.debrisCount;
         dst_stage.explosion        = src_stage.explosion.get_form_stub();
         dst_stage.flags            = (DestructionStageFlags)src_stage.flags;
         dst_stage.health_percent   = src_stage.healthPercent;
         dst_stage.replacement_model.initializeFrom(src_stage.replacementModel);
         dst_stage.self_damage_rate = src_stage.selfDamageRate;
      }
   } else {
      this->_value = {};
   }

   if (src_opt.has_value()) {
      this->_button->setText(tr("Edit Destruction Data"));
   } else {
      this->_button->setText(tr("Add Destruction Data"));
   }
}
void DKFormDestructionDataButton::commitTo(std::optional<form_data_type>& dst_opt, dovah::loaded_forms::Form& dst_owner) {
   if (dst_opt.has_value()) {
      dst_opt.value().clear(dst_owner);
      dst_opt = {};
   }
   if (this->_value.has_value()) {
      auto& src = this->_value.value();
      auto& dst = dst_opt.emplace();

      dst.health = src.health;
      if (src.flags.vats_enabled)
         dst.flags |= form_data_type::data_flag::vats_enabled;

      auto size = src.stages.size();
      dst.stages.resize(size);
      for (size_t i = 0; i < size; ++i) {
         auto& src_stage = src.stages[i];
         auto& dst_stage = dst.stages[i];

         dst_stage.damageStage = src_stage.damage_stage;
         dst_stage.debris.set(dst_owner, src_stage.debris);
         dst_stage.debrisCount = src_stage.debris_count;
         dst_stage.explosion.set(dst_owner, src_stage.explosion);
         dst_stage.flags = src_stage.flags;
         dst_stage.healthPercent = src_stage.health_percent;
         src_stage.replacement_model.commitTo(dst_stage.replacementModel, dst_owner);
         dst_stage.selfDamageRate = src_stage.self_damage_rate;
      }
   }
}
#endif