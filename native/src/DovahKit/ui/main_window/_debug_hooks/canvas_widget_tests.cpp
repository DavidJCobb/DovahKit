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
      auto* child = canvas->layers()[which_color];
      assert(!child->isLayerGroup());
      auto* layer = (CanvasWidgetLayer*)child;
      //
      auto* data  = qobject_cast<CanvasWidgetLayerDataImage*>(layer->data());
      assert(data);
      auto image = data->image();
      auto size  = image.size();
      int x = rand() % size.width();
      int y = rand() % size.height();
      //
      colors[which_color] = 255;
      //
      image.setPixel({ x, y }, qRgba(colors[0], colors[1], colors[2], 255));
      data->setImage(image);
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
         for (auto* child : canvas->layers()) {
            assert(!child->isLayerGroup());
            auto* layer = (CanvasWidgetLayer*)child;
            auto* data  = new CanvasWidgetLayerDataImage(dialog);
            auto  image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
            image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
            data->setImage(image);
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
         scroll->setFixedHeight(159);
         layout->addWidget(scroll);
      }
      //
      auto* grid = new QWidget(dialog);
      grid->setLayout(new QGridLayout);
      layout->addWidget(grid);
      //
      canvas->setImageSize(300, 300);
      //dialog->setFixedHeight(200);
      //
      {
         auto* logo  = canvas->createLayer();
         auto* color = canvas->createLayerGroup();
         logo->setVisible(true);
         color->setVisible(true);
         logo->setObjectName("logo");
         color->setObjectName("color");
         //
         {
            auto* data  = new CanvasWidgetLayerDataImage(dialog);
            auto  image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
            image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
            //
            QPainter painter(&image);
            QBrush   brush = QColor(255, 255, 255, 255);
            painter.setBrush(brush);
            painter.drawEllipse(image.rect());
            //
            data->setImage(image);
            logo->setData(data);
         }
         {
            auto* mask = color->createLayer();
            auto* grad = color->createLayer();
            mask->setVisible(true);
            grad->setVisible(true);
            mask->setObjectName("mask");
            grad->setObjectName("grad");
            color->setCompositionMode(QPainter::CompositionMode_Multiply);
            grad->setCompositionMode(QPainter::CompositionMode_Multiply);
            //
            {
               auto* data  = new CanvasWidgetLayerDataImage(dialog);
               auto  image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
               image.fill(qRgba(0, 0, 0, 0)); // the constructor doesn't actually initialize or clear any image data
               //
               QPainter painter(&image);
               QRect rect = image.rect();
               auto  y    = rect.height() / 2;
               rect.setTop(y / 2);
               rect.setHeight(y);
               painter.fillRect(rect, QColor(255, 255, 255, 255));
               //
               data->setImage(image);
               mask->setData(data);
            }
            {
               auto* data  = new CanvasWidgetLayerDataImage(dialog);
               auto  image = QImage(canvas->imageSize(), QImage::Format_ARGB32);
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
               data->setImage(image);
               grad->setData(data);
            }
         }
      }
      {
         auto* button = new QPushButton("Move Mask to -50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("mask");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / -2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Mask to 0% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("mask");
            assert(layer);
            auto pos = layer->position();
            pos.setX(0);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Mask to 50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("mask");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / 2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Grad to -50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("grad");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / -2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Grad to 0% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("grad");
            assert(layer);
            auto pos = layer->position();
            pos.setX(0);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Grad to 50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("grad");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / 2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Shape to -50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("logo");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / -2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Shape to 0% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("logo");
            assert(layer);
            auto pos = layer->position();
            pos.setX(0);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      {
         auto* button = new QPushButton("Move Shape to 50% X");
         QObject::connect(button, &QPushButton::clicked, [canvas](bool checked) {
            auto* layer = canvas->findChild<CanvasWidgetLayer*>("logo");
            assert(layer);
            auto pos = layer->position();
            pos.setX(canvas->imageWidth() / 2);
            layer->setPosition(pos);
         });
         grid->layout()->addWidget(button);
      }
      //
      dialog->show();
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
   }
}

namespace DovahKitDebug::features {
   /*static*/ void debug_canvas_widget::execute(QWidget* parent) {
      _dialog_1(parent);
      _dialog_2(parent);
   }
}