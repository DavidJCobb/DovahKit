#include "./qt_3d_tests.h"
#include <QDialog>
#include <QGridLayout>
#if !defined(QT_3DCORE_LIB) || !defined(QT_3DEXTRAS_LIB) || !defined(QT_3DRENDER_LIB)
   #include <QMessageBox>
#else
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
#endif

namespace DovahKitDebug::features::qt {
   /*static*/ void qt_3d_tests::execute(QWidget* from) {
      #if !defined(QT_3DCORE_LIB) || !defined(QT_3DEXTRAS_LIB) || !defined(QT_3DRENDER_LIB)
         QMessageBox::information(from, "Not enabled", "Program was compiled without Qt 3D");
      #else
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
      #endif
   }
}
