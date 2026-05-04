#include "./DKFormNIFPicker.h"
#include <QHBoxLayout>
#if !defined(QT_PLUGIN)
   #include "dovah/forms/components/model.h"
   #include "editor/core.h"
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
      {
         auto& editor = DovahKitCore::get();
         QObject::connect(&editor, &DovahKitCore::formDeletionImminent, this, [this](dovah::form_stub* stub, bool only_if_flagged) {
            bool  changed = false;
            auto& list    = this->_value.texture_swaps;
            if (!this->_value.supports_texture_swaps) {
               for (auto& item : list) {
                  if (item.texture_set) {
                     item.texture_set = nullptr;
                     changed = true;
                  }
               }
            } else {
               for(auto& item : list) {
                  if (item.texture_set == stub) {
                     item.texture_set = nullptr;
                     changed = true;
                  }
               }
            }
            if (changed)
               emit this->dataChanged();
         });
      }
      QObject::connect(this->_subwidgets.button, &QPushButton::clicked, this, [this]() {
         auto* dialog = new DKFormNIFPickerDialog(this);

         dialog->setWindowModality(Qt::WindowModality::WindowModal);

         dialog->setTextureSwapsAllowed(this->_value.supports_texture_swaps);
         dialog->setModelPath(QString::fromStdString(this->_value.model_path));
         dialog->setTextureSwaps(this->_value.texture_swaps);

         QObject::connect(dialog, &QDialog::accepted, this, [this, dialog]() {
            this->_value.model_path = dialog->modelPath().toStdString();
            if (this->_value.supports_texture_swaps)
               this->_value.texture_swaps = dialog->textureSwaps();
            this->_value.precached_nif_info = dialog->precachedNIFInfo();

            this->_subwidgets.path->setText(dialog->modelPath());
            emit this->dataChanged();
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
      this->_value = {};
      this->_value.initializeFrom(src);

      this->_subwidgets.path->setText(QString::fromStdString(this->_value.model_path));
      emit this->dataChanged();
   }
   void DKFormNIFPicker::commitTo(dovah::loaded_forms::components::model& dst, dovah::loaded_forms::Form& dst_owner) {
      this->_value.commitTo(dst, dst_owner);
   }

   void DKFormNIFPicker::setValue(const ui::types::nif_for_form& src) {
      this->_value = src;

      this->_subwidgets.path->setText(QString::fromStdString(src.model_path));
      emit this->dataChanged();
   }
#endif