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

   void _dialog_1(QWidget* parent) {
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
   void _dialog_2(QWidget* parent) {
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
         auto* logo  = canvas->createLayer();
         auto* color = canvas->createLayer();
         logo->setVisible(true);
         color->setVisible(true);
         //
         {
            auto* data  = new CanvasWidgetLayerDataImage(dialog);
            auto& image = data->image();
            image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
            image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
            //
            QPainter painter(&image);
            QBrush   brush = QColor(255, 255, 255, 255);
            painter.setBrush(brush);
            painter.drawEllipse(image.rect());
            //
            logo->setData(data);
         }
         {
            auto* mask = color->createLayer();
            auto* grad = color->createLayer();
            mask->setVisible(true);
            grad->setVisible(true);
            color->setCompositionMode(QPainter::CompositionMode_Multiply);
            grad->setCompositionMode(QPainter::CompositionMode_Multiply);
            //
            {
               auto* data  = new CanvasWidgetLayerDataImage(dialog);
               auto& image = data->image();
               image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
               image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
               //
               QPainter painter(&image);
               QRect rect = image.rect();
               auto  y    = rect.height() / 2;
               rect.setHeight(y);
               rect.setTop(rect.top() + y / 2);
               painter.fillRect(rect, QColor(255, 255, 255, 255));
               //
               mask->setData(data);
            }
            {
               auto* data  = new CanvasWidgetLayerDataImage(dialog);
               auto& image = data->image();
               image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
               image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
               //
               QPainter painter(&image);
               QLinearGradient gradient(0, 0, image.width(), 0);
               gradient.setStops({
                  { 0.000, QColor::fromHsl(  0, 255, 128) },
                  { 0.128, QColor::fromHsl( 45, 255, 128) },
                  { 0.250, QColor::fromHsl( 90, 255, 128) },
                  { 0.375, QColor::fromHsl(135, 255, 128) },
                  { 0.500, QColor::fromHsl(180, 255, 128) },
                  { 0.625, QColor::fromHsl(225, 255, 128) },
                  { 0.750, QColor::fromHsl(270, 255, 128) },
                  { 0.875, QColor::fromHsl(315, 255, 128) },
                  { 1.000, QColor::fromHsl(359, 255, 128) },
               });
               painter.fillRect(image.rect(), gradient);
               //
               grad->setData(data);
            }
         }
      }
      //
      dialog->show();
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
   }
}

namespace DovahKitDebug {
   void debug_canvas_widget(QWidget* parent) {
      _dialog_1(parent);
      _dialog_2(parent);
   }
}