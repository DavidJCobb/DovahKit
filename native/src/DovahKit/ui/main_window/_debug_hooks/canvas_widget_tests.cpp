#include "canvas_widget_tests.h"
#include <array>
#include <QBoxLayout>
#include <QCheckBox>
#include <QDialog>
#include <QPushButton>
#include <QScrollArea>
#include "../../generic/CanvasWidget.h"

namespace {
   void _max_random_pixel(CanvasWidget* canvas, int which_color) {
      std::array<int, 3> colors = { 0, 0, 0 };
      //
      auto* layer = canvas->layers()[which_color];
      auto* data  = qobject_cast<CanvasWidgetLayerDataImage*>(layer->data());
      assert(data);
      auto& image = data->image();
      auto  size  = image.size();
      int x = rand() % size.width();
      int y = rand() % size.height();
      //
      colors[which_color] = 255;
      //
      image.setPixel({ x, y }, qRgba(colors[0], colors[1], colors[2], 255));
   }
}

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
         canvas->createLayer(); // red
         canvas->createLayer(); // green
         canvas->createLayer(); // blue
         for (auto* layer : canvas->layers()) {
            auto* data  = new CanvasWidgetLayerDataImage(dialog);
            auto& image = data->image();
            image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
            image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
            layer->setData(data);
         }
      }
      {
         auto* button = new QPushButton("Random Red Pixel");
         QObject::connect(button, &QPushButton::clicked, [canvas]() {
            _max_random_pixel(canvas, 0);
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
            _max_random_pixel(canvas, 1);
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
            _max_random_pixel(canvas, 2);
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
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
   }
}