#include "vulkan_renderer_instance.h"
#include <QCheckBox>
#include <QDialog>
#include <QGridLayout>
#include "widgets/DKVulkanView.h"

namespace DovahKitDebug::features::renderwin {
   /*static*/ void vulkan_renderer_instance::execute(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QGridLayout(dialog);
      dialog->setLayout(layout);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      {
         auto* view = new DKVulkanView(dialog);
         layout->addWidget(view, 0, 0);
      }
      //
      dialog->show();
   }
}