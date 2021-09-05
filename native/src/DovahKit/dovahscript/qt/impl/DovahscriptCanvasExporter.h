#pragma once
#include <QObject>
#include <QString>

class CanvasWidget;
class CanvasWidgetEntity;
class CanvasWidgetLayer;
class CanvasWidgetLayerGroup;

class DovahscriptCanvasExporter {
   public:
      QVector<CanvasWidgetLayer*>      layers;
      QVector<CanvasWidgetLayerGroup*> groups;
      QString index_file;

      void process(CanvasWidget*);

   protected:
      void gather_layers(const CanvasWidgetLayerGroup&);
      void gather_layers(const CanvasWidget&);

      void print_layers(const CanvasWidget&);
      void print_layers(const CanvasWidgetLayerGroup&, const QString& indent);

      static QString entity_as_text(int index, const CanvasWidgetEntity&);
};