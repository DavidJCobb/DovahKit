#include "DovahKitQtCustomWidgetsPlugin.h"
#include <QtCore/QtPlugin>

#include "interfaces/DKCollapsiblePaneInterface.h"
#include "interfaces/DKColorPickerButtonInterface.h"
#include "interfaces/DKFormListPaneInterface.h"
#include "interfaces/DKGameFilePickerInterface.h"

DovahKitQtCustomWidgetsPlugin::DovahKitQtCustomWidgetsPlugin(QObject* parent) : QObject(parent) {
   widgets.append(new DKCollapsiblePaneInterface(this));
   widgets.append(new DKColorPickerButtonInterface(this));
   widgets.append(new DKFormListPaneInterface(this));
   widgets.append(new DKGameFilePickerInterface(this));
}

QList<QDesignerCustomWidgetInterface*> DovahKitQtCustomWidgetsPlugin::customWidgets() const {
   return widgets;
}