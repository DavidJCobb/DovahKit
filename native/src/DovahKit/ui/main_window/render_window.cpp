#include "render_window.h"
#include <QBoxLayout>
#include <QFileDialog>
#include <QInputDialog>
#include <QStatusBar>
#include <QStyle>
#include <QToolButton>
#include "widgets/DKVulkanView.h"
#include "../../vulkan/surface_renderer.h"

#include "nif/file.h"
#include "nif/notice_code_t.h"
#include "nif/blocks/NiNode.h"
#include "nif/blocks/NiGeometry.h"
#include "nif/blocks/NiGeometryData.h"

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
         auto path = QFileDialog::getOpenFileName(this, "Texture file", "", "Image (*.dds, *.png, *.bmp)");
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
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Set Camera Position");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         glm::fvec3 pos;
         bool ok;
         //
         pos.x = QInputDialog::getDouble(this, "X", "Input X-coordinate", 0.0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.y = QInputDialog::getDouble(this, "Y", "Input Y-coordinate", 0.0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         pos.z = QInputDialog::getDouble(this, "Z", "Input Z-coordinate", 0.0, -999999, 999999, 4, &ok);
         if (!ok)
            return;
         //
         view->surfaceRenderer()->set_camera_position(pos);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_VistaShield));
      //
      this->toolbar->addWidget(button);
   }
   //
   {
      auto* button = new QToolButton(this->toolbar);
      button->setText("Load NIF...");
      QObject::connect(button, &QAbstractButton::clicked, this, [this, view]() {
         auto path = QFileDialog::getOpenFileName(this, "Model", "", "NetImmerse Format model (*.nif)");
         if (path.isEmpty())
            return;
         nifDK::file model;
         {
            QFile file(path);
            if (!file.open(QIODevice::ReadOnly)) {
               qDebug("Failed to open file.");
               return;
            }
            QByteArray data = file.readAll();
            model.read((void*)data.constData(), data.size());
            //
            auto& error = model.read_error();
            if (error.code != nifDK::default_notice_code) {
               qDebug("NIF parsing failed with error code %08X.", error.code);
               #if _DEBUG
                  __debugbreak();
               #endif
            }
         }
         qDebug("NIF parsed. Passing to surface_renderer...");
         view->surfaceRenderer()->add_nif(model);
      });
      button->setIcon(this->style()->standardIcon(QStyle::SP_FileIcon));
      //
      this->toolbar->addWidget(button);
   }
}