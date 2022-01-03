#include "render_window.h"
#include <QBoxLayout>
#include <QFileDialog>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include "../../editor/DovahKitVulkanSubsystem.h"
#include "widgets/DKVulkanView.h"
#include "../../vulkan/surface_renderer.h"

namespace {
   static constexpr bool use_new_renderer = true;
}

RenderWindow::RenderWindow(QWidget* parent) : QWidget(parent) {
   this->setWindowTitle(tr("Render Window"));
   this->setMinimumSize({ 150, 150 });
   //
   auto* layout = new QVBoxLayout(this);
   layout->setContentsMargins({ 0, 0, 0, 0 });
   //
   DKVulkanView* view = nullptr;
   //
   if constexpr (use_new_renderer) {
      view = new DKVulkanView(this);
      view->setInputHandlingEnabled(true);
      //
      layout->addWidget(view, 1);
      QObject::connect(view, &DKVulkanView::renderedMeshClicked, this, [this](size_t index) {
         this->status->showMessage(QString("Mesh #%1 clicked.").arg(index), 2000);
      });
   } else {
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
   }
   //
   this->toolbar = new QToolBar(this);
   layout->setMenuBar(this->toolbar);
   {
      auto* sb = new QStatusBar(this);
      this->status = sb;
      layout->addWidget(sb, 0);
   }

   for (size_t i = 0; i < 3; ++i) {
      auto* button = new QToolButton(this->toolbar);
      button->setText(QString("Pause #%1").arg(i));
      button->setCheckable(true);
      QObject::connect(button, &QAbstractButton::toggled, this, [this, view, i](bool checked) {
         if constexpr (use_new_renderer) {
            view->surfaceRenderer()->set_animation_paused(i, checked);
         } else {
            DovahKitVulkanSubsystem::get().setAnimationPaused(i, checked);
         }
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_MediaPause));
      //
      this->toolbar->addWidget(button);
   }
   //
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("New Object");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto path = QFileDialog::getOpenFileName(this, "Texture file", "", "Image (*.png, *.bmp)");
         if (path.isEmpty())
            return;
         //
         if constexpr (use_new_renderer) {
            view->surfaceRenderer()->add_mesh(path);
         } else {
            DovahKitVulkanSubsystem::get().addRenderedObject(path);
         }
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Delete Last Object");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         if constexpr (use_new_renderer) {
            view->surfaceRenderer()->remove_last_mesh();
         } else {
            DovahKitVulkanSubsystem::get().removeRenderedObject();
         }
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_BrowserStop));
      //
      this->toolbar->addWidget(button);
   }

   if constexpr (!use_new_renderer) {
      DovahKitVulkanSubsystem::get().initialize();
   }
}