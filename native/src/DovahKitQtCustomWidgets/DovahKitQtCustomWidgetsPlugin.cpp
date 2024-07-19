#include "DovahKitQtCustomWidgetsPlugin.h"
#include <QtCore/QtPlugin>

#include "interfaces/DKCollapsiblePaneInterface.h"
#include "interfaces/DKColorPickerButtonInterface.h"
#include "interfaces/DKFormListPaneInterface.h"
#include "interfaces/DKGameFilePickerInterface.h"
#include "interfaces/DKTextureAssetPaneInterface.h"
#include "interfaces/DKObjectReferencePickerInterface.h"
#include "interfaces/DKQuestAliasPickerInterface.h"
#include "interfaces/DKFormPickerInterface.h"
#include "interfaces/DKPapyrusBoundScriptListPaneInterface.h"
#include "interfaces/DKFormNIFPickerInterface.h"
#include "interfaces/DKFormDestructionDataButtonInterface.h"
#include "interfaces/DKDelayedEnableButtonInterface.h"
#include "interfaces/DKCompactObjectReferencePickerInterface.h"
#include "interfaces/DKPapyrusFragmentFunctionPickerInterface.h"
#include "interfaces/DKConditionListInterface.h"
#include "interfaces/DKNavmeshGenerationImportOptionPickerInterface.h"
#include "interfaces/DKFormInventoryWidgetInterface.h"

DovahKitQtCustomWidgetsPlugin::DovahKitQtCustomWidgetsPlugin(QObject* parent) : QObject(parent) {
   widgets.append(new DKCollapsiblePaneInterface(this));
   widgets.append(new DKColorPickerButtonInterface(this));
   widgets.append(new DKDelayedEnableButtonInterface(this));

   widgets.append(new DKFormPickerInterface(this));
   widgets.append(new DKFormListPaneInterface(this));
   widgets.append(new DKObjectReferencePickerInterface(this));
   widgets.append(new DKCompactObjectReferencePickerInterface(this));
   widgets.append(new DKQuestAliasPickerInterface(this));
   widgets.append(new DKFormDestructionDataButtonInterface(this));
   widgets.append(new DKPapyrusBoundScriptListPaneInterface(this));
   widgets.append(new DKPapyrusFragmentFunctionPickerInterface(this));
   widgets.append(new DKConditionListInterface(this));
   widgets.append(new DKFormInventoryWidgetInterface(this));
   widgets.append(new DKNavmeshGenerationImportOptionPickerInterface(this));

   widgets.append(new DKGameFilePickerInterface(this));
   widgets.append(new DKFormNIFPickerInterface(this));
   widgets.append(new DKTextureAssetPaneInterface(this));
}

QList<QDesignerCustomWidgetInterface*> DovahKitQtCustomWidgetsPlugin::customWidgets() const {
   return widgets;
}