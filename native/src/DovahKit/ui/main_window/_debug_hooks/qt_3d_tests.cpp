#include "qt_3d_tests.h"
#include <QDialog>
#include <QGridLayout>
#pragma region Qt 3D includes
   #if !defined(QT_3DCORE_LIB)
      #error Add 3dcore to the list of enabled modules in your Qt project settings.
   #endif
   #if !defined(QT_3DEXTRAS_LIB)
      #error Add 3dextras to the list of enabled modules in your Qt project settings.
   #endif
   #if !defined(QT_3DRENDER_LIB)
      #error Add 3drender to the list of enabled modules in your Qt project settings.
   #endif
   #include <Qt3DCore/QEntity>
   #include <Qt3DRender/QCamera>
   #include <Qt3DRender/QCameraLens>
   #include <Qt3DCore/QTransform>
   #include <Qt3DCore/QAspectEngine>

   #include <Qt3DInput/QInputAspect>

   #include <Qt3DRender/QRenderAspect>
   #include <Qt3DExtras/QForwardRenderer>
   #include <Qt3DExtras/QPhongMaterial>
   #include <Qt3DExtras/QSphereMesh>

   #include <Qt3DExtras/Qt3DWindow>
#pragma endregion

namespace DovahKitDebug::features {
   /*static*/ void qt_3d_tests::execute(QWidget* from) {
      auto* view       = new Qt3DExtras::Qt3DWindow;
      auto* scene_root = new Qt3DCore::QEntity;
      auto* material   = new Qt3DExtras::QPhongMaterial(scene_root);
      {  // Create sphere
         auto* entity = new Qt3DCore::QEntity(scene_root);
         auto* mesh   = new Qt3DExtras::QSphereMesh;
         mesh->setRadius(3);
         mesh->setGenerateTangents(true);
         //
         entity->addComponent(mesh);
         entity->addComponent(material);
      }
      view->setRootEntity(scene_root);
      //
      if (auto* camera = view->camera()) {
         camera->lens()->setPerspectiveProjection(45.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
         camera->setPosition({ 0, 0, 40 });
         camera->setViewCenter({ 0, 0, 0 });
      }
      //
      auto* dialog = new QDialog(from);
      {
         auto* wrapper = QWidget::createWindowContainer(view, dialog);
         auto* layout  = new QGridLayout(dialog);
         layout->setContentsMargins({ 0, 0, 0, 0 });
         layout->addWidget(wrapper);
         dialog->setLayout(layout);
         dialog->setMinimumSize(150, 150);
      }
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      dialog->show();
   }
}
