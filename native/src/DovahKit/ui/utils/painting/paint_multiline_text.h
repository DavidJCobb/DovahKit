#pragma once
#include <QPainter>

namespace ui::utils {
   extern void paint_multiline_text(QPainter&, const QPointF& at, Qt::Alignment, QString);
}
