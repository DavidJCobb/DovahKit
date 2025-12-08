#include "paint_ellipse_tests.h"
#include <algorithm>
#include <array>
#include <QDialog>
#include <QGridLayout>
#include <QImage>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>

namespace {
   void _configure_painter(QPainter& painter) {
      QPen pen;
      pen.setColor(Qt::GlobalColor::transparent);
      pen.setWidthF(1.0F);
      pen.setJoinStyle(Qt::PenJoinStyle::MiterJoin);
      //
      QBrush brush(Qt::SolidPattern);
      brush.setColor(QColor(0x88, 0x88, 0x88));
      //
      painter.setPen(pen);
      painter.setBrush(brush);
      painter.setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing, true);
   }
}

namespace DovahKitDebug::features::qt {
   /*static*/ void paint_ellipse_tests::execute(QWidget* parent) {
      auto* dialog = new QDialog(parent);
      auto* layout = new QGridLayout(dialog);
      dialog->setLayout(layout);
      QObject::connect(dialog, &QDialog::finished, dialog, &QObject::deleteLater);
      //
      layout->addWidget(new QLabel("QPainter::drawEllipse"),    0, 0);
      layout->addWidget(new QLabel("QPainterPath::addEllipse"), 1, 0);
      layout->addWidget(new QLabel("QPainterPath::addEllipse (downscaled)"), 2, 0);
      layout->addWidget(new QLabel("QPainterPath::addEllipse (upscaled)"), 3, 0);
      {
         constexpr std::array radii = { 1, 3, 5, 7, 9, 13, 15 };
         constexpr qreal span = *std::max_element(radii.begin(), radii.end()) * 2;
         constexpr int   gap  = 5;
         constexpr int   w    = (span + gap) * radii.size();
         constexpr int   h    =  50;
         //
         std::array<QImage, 4> images;
         for (auto& image : images) {
            image = QImage(w, h, QImage::Format::Format_ARGB32);
            //
            QPainter painter(&image);
            _configure_painter(painter);
            int x = 0;
            for (auto radius : radii) {
               auto rect = QRect(x, h - 12, span, 12);
               painter.drawText(rect, Qt::AlignHCenter, QString::number(radius));
               x += span + gap;
            }
         }
         //
         {
            QPainter painter(&images[0]);
            _configure_painter(painter);
            int x = 0;
            for (auto radius : radii) {
               qreal cx = x + (span / 2);
               qreal cy = h / 2;
               painter.drawEllipse(QPointF{ cx, cy }, radius, radius);
               x += span + gap;
            }
         }
         {
            QPainter painter(&images[1]);
            QPainterPath path;
            _configure_painter(painter);
            int x = 0;
            for (auto radius : radii) {
               qreal cx = x + (span / 2);
               qreal cy = h / 2;
               path.closeSubpath();
               path.addEllipse(QPointF{ cx, cy }, radius, radius);
               path.closeSubpath();
               x += span + gap;
            }
            path = path.translated(QPointF{ 0, 0 });
            painter.drawPath(path);
         }
         {
            QPainter painter(&images[2]);
            QPainterPath path;
            _configure_painter(painter);
            painter.scale(2, 2);
            int x = 0;
            for (auto radius : radii) {
               qreal cx = x + (span / 2);
               qreal cy = h / 2;
               path.addEllipse(QPointF{ cx, cy } / 2, radius / 2, radius / 2);
               x += span + gap;
            }
            path = path.translated(QPointF{ 0, 0 });
            painter.drawPath(path);
         }
         {
            QPainter painter(&images[3]);
            QPainterPath path;
            _configure_painter(painter);
            painter.scale(.5, .5);
            int x = 0;
            for (auto radius : radii) {
               qreal cx = x + (span / 2);
               qreal cy = h / 2;
               path.addEllipse(QPointF{ cx, cy } * 2, radius * 2, radius * 2);
               x += span + gap;
            }
            path = path.translated(QPointF{ 0, 0 });
            painter.drawPath(path);
         }
         //
         for (size_t i = 0; i < images.size(); ++i) {
            auto& image = images[i];
            auto* label = new QLabel(dialog);
            label->setPixmap(QPixmap::fromImage(image));
            layout->addWidget(label, i, 1);
         }
      }
      //
      dialog->show();
   }
}