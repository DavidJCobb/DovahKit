#include "render_window.h"
#include <QBoxLayout>
#include <QStyle>
#include <QToolButton>
#include "../../editor/DovahKitVulkanSubsystem.h"

//
// TODO: for drawing directly to a widget using non-Qt systems (e.g. WinAPI, maybe Vulkan);
// 
//  - https://www.qtcentre.org/threads/29232-Qt-WINAPI-direct-painting
//    Set widget attribute Qt::WA_PaintOnScreen; use widget subclass which overrides 
//    paintEngine() to return nullptr.
//

RenderWindow::RenderWindow(QWidget* parent) : QWidget(parent) {
   this->setWindowTitle(tr("Render Window"));
   this->setMinimumSize({ 150, 150 });
   //
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

   for (size_t i = 0; i < 3; ++i) {
      auto* button = new QToolButton(this->toolbar);
      button->setText(QString("Pause %#1").arg(i));
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::toggled, this, [this, i](bool checked) {
         auto& vulkan = DovahKitVulkanSubsystem::get();
         vulkan.setAnimationPaused(i, checked);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_MediaPause));
      //
      this->toolbar->addWidget(button);
   }

   vulkan.initialize();
}