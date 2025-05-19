#include "./DKFormNIFPicker.h"
#include <QHBoxLayout>
#if !defined(QT_PLUGIN)
   #include "dovah/core.h"
   #include "dovah/forms/components/model.h"
   #include "./widget-dialogs/DKFormNIFPickerDialog.h"
#endif

DKFormNIFPicker::DKFormNIFPicker(QWidget* parent) : QWidget(parent) {
   auto* layout = new QHBoxLayout(this);
   this->setLayout(layout);
   layout->setContentsMargins(0, 0, 0, 0);

   this->_subwidgets.path   = new QLineEdit(this);
   this->_subwidgets.button = new QPushButton(tr("Edit"), this);

   this->_subwidgets.path->setReadOnly(true);

   layout->addWidget(this->_subwidgets.path);
   layout->addWidget(this->_subwidgets.button);
   
   this->setFocusPolicy(Qt::FocusPolicy::TabFocus);
   this->setFocusProxy(this->_subwidgets.path);
   QWidget::setTabOrder(this->_subwidgets.path, this->_subwidgets.button);

   #if !defined(QT_PLUGIN)
   QObject::connect(this->_subwidgets.button, &QPushButton::clicked, this, [this]() {
      auto* dialog = new DKFormNIFPickerDialog(this);

      dialog->setWindowModality(Qt::WindowModality::WindowModal);

      dialog->setTextureSwapsAllowed(this->_state.supports_texture_swaps);
      dialog->setModelPath(QString::fromStdString(this->_state.model_path));
      dialog->setTextureSwaps(this->_state.texture_swaps);

      QObject::connect(dialog, &QDialog::accepted, this, [this, dialog]() {
         this->_state.model_path = dialog->modelPath().toStdString();
         if (this->_state.supports_texture_swaps)
            this->_state.texture_swaps = dialog->textureSwaps();
         this->_state.precached_nif_info = dialog->precachedNIFInfo();

         this->_subwidgets.path->setText(dialog->modelPath());
      });
      QObject::connect(dialog, &QDialog::finished, dialog, [dialog]() {
         dialog->deleteLater();
      });

      dialog->show();
   });
   #endif
}

#if !defined(QT_PLUGIN)
   void DKFormNIFPicker::initializeFrom(const dovah::loaded_forms::components::model& src) {
      this->_state = {};

      this->_state.model_path             = src.model_path;
      this->_state.supports_texture_swaps = src.supports_texture_swaps;
      if (src.supports_texture_swaps) {
         const auto& ts = static_cast<const dovah::loaded_forms::components::model_ts&>(src);
         for (auto& item : ts.texture_swaps) {
            auto& dst = this->_state.texture_swaps.emplace_back();
            dst.block_name  = item.nif_block_name;
            dst.leaf_index  = item.nif_leaf_index;
            dst.texture_set = item.texture_set.get_form_stub();
         }
      }

      this->_subwidgets.path->setText(QString::fromStdString(this->_state.model_path));
   }
   void DKFormNIFPicker::commitTo(dovah::loaded_forms::components::model& dst, dovah::loaded_forms::Form& dst_owner) {
      dst.model_path     = this->_state.model_path;
      dst.precached_info = this->_state.precached_nif_info;
      if (dst.supports_texture_swaps) {
         auto& ts = static_cast<dovah::loaded_forms::components::model_ts&>(dst);

         for (auto& item : ts.texture_swaps) {
            item.texture_set.set(dst_owner, nullptr);
         }
         ts.texture_swaps.clear();

         for (auto& src : this->_state.texture_swaps) {
            if (!src.texture_set)
               continue;
            auto& dst = ts.texture_swaps.emplace_back();
            dst.nif_block_name = src.block_name;
            dst.nif_leaf_index = src.leaf_index;
            dst.texture_set.set(dst_owner, src.texture_set);
         }
      }
   }
#endif