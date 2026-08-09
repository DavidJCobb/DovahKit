#pragma once
#include <QDesignerCustomWidgetInterface>
#include <type_traits>

class DovahKitQtCustomWidgetsPlugin : public QObject, public QDesignerCustomWidgetCollectionInterface {
   Q_OBJECT;
   Q_PLUGIN_METADATA(IID "org.qt-project.Qt.QDesignerCustomWidgetCollectionInterface" FILE "dovahkitqtcustomwidgetsplugin.json");
   Q_INTERFACES(QDesignerCustomWidgetCollectionInterface);
   public:
      DovahKitQtCustomWidgetsPlugin(QObject* parent = Q_NULLPTR);

      QList<QDesignerCustomWidgetInterface*> customWidgets() const override;

   private:
      QList<QDesignerCustomWidgetInterface*> widgets;
};
