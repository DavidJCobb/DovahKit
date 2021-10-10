#include "DovahKitQtCustomWidgetsPlugin.h"
#include <QtCore/QtPlugin>

#include "interfaces/DKCollapsiblePaneInterface.h"
#include "interfaces/DKFormListPaneInterface.h"

DovahKitQtCustomWidgetsPlugin::DovahKitQtCustomWidgetsPlugin(QObject* parent) : QObject(parent) {
   widgets.append(new DKCollapsiblePaneInterface(this));
   widgets.append(new DKFormListPaneInterface(this));
}

QList<QDesignerCustomWidgetInterface*> DovahKitQtCustomWidgetsPlugin::customWidgets() const {
   return widgets;
}