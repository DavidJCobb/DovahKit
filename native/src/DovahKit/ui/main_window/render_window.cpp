#include "render_window.h"
#include <QBoxLayout>
#include "../../editor/DovahKitVulkanSubsystem.h"

//
// TODO: for drawing directly to a widget using non-Qt systems (e.g. WinAPI, maybe Vulkan);
// 
//  - https://www.qtcentre.org/threads/29232-Qt-WINAPI-direct-painting
//    Set widget attribute Qt::WA_PaintOnScreen; use widget subclass which overrides 
//    paintEngine() to return nullptr.
//

RenderWindow::RenderWindow(QWidget* parent) : QWidget(parent) {
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   auto& vulkan = DovahKitVulkanSubsystem::get();
   QObject::connect(&vulkan, &DovahKitVulkanSubsystem::ready, this, [this]() {
      auto& vulkan = DovahKitVulkanSubsystem::get();
      auto* render = vulkan.renderWindowWidget();
      if (!render)
         return;
      this->layout()->addWidget(render);
      qDebug("Render window adopted the Vulkan render-window widget.");
   });
   QObject::connect(&vulkan, &DovahKitVulkanSubsystem::teardownImminent, this, [this]() {
      // ...
   });
   //
   this->toolbar = new QToolBar(this);
   layout->setMenuBar(this->toolbar);


   vulkan.initialize();
}