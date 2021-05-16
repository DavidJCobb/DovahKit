#include "canvas_widget_tests.h"
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QScrollArea>
#include "../../generic/CanvasWidget.h"

namespace DovahKitDebug {
   void debug_canvas_widget(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QBoxLayout(QBoxLayout::Direction::LeftToRight);
      auto* canvas = new CanvasWidget(dialog);
      dialog->setLayout(layout);
      {
         auto* scroll = new QScrollArea;
         scroll->setWidget(canvas);
         layout->addWidget(scroll);
      }
      //
      auto* grid = new QWidget(dialog);
      grid->setLayout(new QGridLayout);
      layout->addWidget(grid);
      //
      canvas->setImageSize(300, 300);
      dialog->setFixedHeight(200);
      //
      {
         canvas->addLayer(); // red
         canvas->addLayer(); // green
         canvas->addLayer(); // blue
         for (auto* layer : canvas->layers()) {
            auto* data  = new CanvasLayerData;
            auto* image = new QImage(canvas->imageSize(), QImage::Format_ARGB32);
            image->fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
            data->replaceWithImage(image);
            layer->setData(data);
         }
      }
      {
         auto* button = new QPushButton("Random Red Pixel");
         QObject::connect(button, &QPushButton::clicked, [canvas]() {
            auto* layer = canvas->layers()[0];
            auto* data  = layer->data();
            auto* image = data->checkOutImage();
            auto  size  = image->size();
            int x = rand() % size.width();
            int y = rand() % size.height();
            image->setPixel({ x, y }, qRgba(255, 0, 0, 255));
            data->checkInImage(image);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QCheckBox("Show Red Layer");
         QObject::connect(button, &QCheckBox::toggled, [canvas](bool checked) {
            auto* layer = canvas->layers()[0];
            layer->setVisible(checked);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Random Green Pixel");
         QObject::connect(button, &QPushButton::clicked, [canvas]() {
            auto* layer = canvas->layers()[1];
            auto* data  = layer->data();
            auto* image = data->checkOutImage();
            auto  size  = image->size();
            int x = rand() % size.width();
            int y = rand() % size.height();
            image->setPixel({ x, y }, qRgba(0, 160, 0, 255));
            data->checkInImage(image);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QCheckBox("Show Green Layer");
         QObject::connect(button, &QCheckBox::toggled, [canvas](bool checked) {
            auto* layer = canvas->layers()[1];
            layer->setVisible(checked);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Random Blue Pixel");
         QObject::connect(button, &QPushButton::clicked, [canvas]() {
            auto* layer = canvas->layers()[2];
            auto* data  = layer->data();
            auto* image = data->checkOutImage();
            auto  size  = image->size();
            int x = rand() % size.width();
            int y = rand() % size.height();
            image->setPixel({ x, y }, qRgba(0, 0, 255, 255));
            data->checkInImage(image);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QCheckBox("Show Blue Layer");
         QObject::connect(button, &QCheckBox::toggled, [canvas](bool checked) {
            auto* layer = canvas->layers()[2];
            layer->setVisible(checked);
         });
         grid->layout()->addWidget(button);
      }
      //
      dialog->show();
   }
}