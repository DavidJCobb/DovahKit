#pragma once
#include <QtDesigner/QDesignerTaskMenuExtension>
#include <QPointer>
#include "../widgets/DKYesNoUnsetWidget.h"
#include "../tools/in_place_text_editor/InPlaceTextEditor.h"

class InPlaceTextEditor;

class DKYesNoUnsetWidgetTaskMenuExtension : public QObject, public QDesignerTaskMenuExtension {
   Q_OBJECT;
   Q_INTERFACES(QDesignerTaskMenuExtension);
   public:
      DKYesNoUnsetWidgetTaskMenuExtension(DKYesNoUnsetWidget* widget, QObject* parent);

      QAction* preferredEditAction() const;
      QList<QAction*> taskActions() const;

   private slots:
      void editText();

   private:
      DKYesNoUnsetWidget* widget = nullptr;
      struct {
         QAction* editText = nullptr;
      } actions;
      QPointer<InPlaceTextEditor> _last_in_place_editor;
};