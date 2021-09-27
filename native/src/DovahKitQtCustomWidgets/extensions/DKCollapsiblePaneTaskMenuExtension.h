#pragma once
#include <QtDesigner/QDesignerTaskMenuExtension>
#include "../widgets/DKCollapsiblePane.h"

class DKCollapsiblePaneTaskMenuExtension : public QObject, public QDesignerTaskMenuExtension {
   Q_OBJECT;
   Q_INTERFACES(QDesignerTaskMenuExtension);
   public:
      DKCollapsiblePaneTaskMenuExtension(DKCollapsiblePane* widget, QObject* parent);

      QAction* preferredEditAction() const;
      QList<QAction*> taskActions() const;

   private slots:
      void openActionEditor();

   private:
      DKCollapsiblePane* widget = nullptr;
      struct {
         QAction* editActions = nullptr;
      } actions;
};