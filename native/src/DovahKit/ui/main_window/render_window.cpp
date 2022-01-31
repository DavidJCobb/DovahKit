#include "render_window.h"
#include <QBoxLayout>
#include <QFileDialog>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
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
   DKVulkanView* view = new DKVulkanView(this);
   view->setInputHandlingEnabled(true);
   //
   layout->addWidget(view, 1);
   QObject::connect(view, &DKVulkanView::renderedMeshClicked, this, [this](size_t index) {
      this->status->showMessage(QString("Mesh #%1 clicked.").arg(index), 2000);
   });
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
         view->surfaceRenderer()->set_animation_paused(i, checked);
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
         view->surfaceRenderer()->add_mesh(path);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileDialogNewFolder));
      //
      this->toolbar->addWidget(button);
   }
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Delete Last Object");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         view->surfaceRenderer()->remove_last_mesh();
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_BrowserStop));
      //
      this->toolbar->addWidget(button);
   }
}