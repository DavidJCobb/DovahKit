#include "canvas_context_menu.h"
#include "../../../ui/generic/CanvasWidget.h"
#include <QAction>
#include <QBuffer>
#include <QByteArray>
#include <QFile>
#include <QFileDialog>
#include <QImage>
#include <QMenu>
#include <QMessageBox>
#include <QStandardPaths>
#include "../../../../miniz/miniz.h"
#include "DovahscriptCanvasExporter.h"

namespace {
   void _flatten_canvas(CanvasWidget* canvas) {
      auto path = QFileDialog::getSaveFileName(
         canvas->window(),
         QObject::tr("Save script image"),
         QStandardPaths::locate(QStandardPaths::StandardLocation::DocumentsLocation, QString(), QStandardPaths::LocateDirectory),
         "Images (*.png *.xpm *.jpg)"
      );
      if (path.isEmpty())
         return;
      //
      auto size = canvas->imageSize();
      auto out  = QImage(size.width(), size.height(), QImage::Format::Format_ARGB32_Premultiplied);
      {
         QPainter painter(&out);
         canvas->drawTo(painter, out.rect());
      }
      if (!out.save(path)) {
         QMessageBox::critical(
            canvas->window(),
            QObject::tr("Error"),
            QObject::tr("The image was not saved.")
         );
      }
   }

   namespace {
      size_t _miniz_write_to_qfile(void* pOpaque, mz_uint64 file_ofs, const void* pBuf, size_t n) {
         auto* file = (QFile*) pOpaque;
         file->write((const char*)pBuf, n);
         return n;
      }
   }
   void _export_canvas(CanvasWidget* canvas) {
      auto path = QFileDialog::getSaveFileName(
         canvas->window(),
         QObject::tr("Save script layers as ZIP"),
         QStandardPaths::locate(QStandardPaths::StandardLocation::DocumentsLocation, QString(), QStandardPaths::LocateDirectory),
         "ZIP Archive (*.zip)"
      );
      if (path.isEmpty())
         return;
      //
      QFile file(path);
      if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) {
         QMessageBox::critical(
            canvas->window(),
            QObject::tr("Error"),
            QObject::tr("Failed to open a file for writing. The archive was not saved.")
         );
         return;
      }
      mz_zip_archive zip;
      memset(&zip, 0, sizeof(zip));
      zip.m_pWrite     = &_miniz_write_to_qfile;
      zip.m_pIO_opaque = (void*)&file;
      if (!mz_zip_writer_init(&zip, 0)) {
         QMessageBox::critical(
            canvas->window(),
            QObject::tr("Error"),
            QObject::tr("The archive was not saved.")
         );
         return;
      }
      //
      DovahscriptCanvasExporter exporter;
      exporter.process(canvas);
      //
      {
         {
            auto pt = exporter.index_file.toUtf8();
            mz_bool status = mz_zip_writer_add_mem(
               &zip,
               "info.txt",
               pt.constData(),
               pt.size(),
               5
            );
            if (!status) {
               file.remove();
               QMessageBox::critical(
                  canvas->window(),
                  QObject::tr("Error"),
                  QObject::tr("The archive was not saved.")
               );
               return;
            }
         }
         //
         auto& layers = exporter.layers;
         int   size   = layers.size();
         for(int i = 0; i < size; ++i) {
            auto* layer = layers[i];
            auto* data  = layer->data();
            if (!data)
               continue;
            QRect  rect = data->rect();
            QImage out  = layer->render(rect, { 0, 0 });
            //
            QByteArray bytes;
            {
               QBuffer buffer(&bytes);
               buffer.open(QIODevice::WriteOnly);
               out.save(&buffer, "PNG");
            }
            QString comment = "";
            {
               auto cd = comment.toUtf8();
               mz_bool status = mz_zip_writer_add_mem(
                  &zip,
                  QString("layer %1.png").arg(i + 1).toUtf8(),
                  bytes.constData(),
                  bytes.size(),
                  5
               );
               if (!status) {
                  file.remove();
                  QMessageBox::critical(
                     canvas->window(),
                     QObject::tr("Error"),
                     QObject::tr("The archive was not saved.")
                  );
                  return;
               }
            }
         }
      }
      mz_zip_writer_finalize_archive(&zip);
      mz_zip_end(&zip);
   }
}

namespace dovahscript::impl {
   extern void set_up_canvas_context_menu(CanvasWidget* canvas) {
      canvas->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);
      QObject::connect(canvas, &QWidget::customContextMenuRequested, [canvas](const QPoint& pos) {
         QMenu menu(canvas);
         auto* action_flatten = new QAction(QObject::tr("Save image...",  "scripted canvas context"), &menu);
         auto* action_export  = new QAction(QObject::tr("Save layers...", "scripted canvas context"), &menu);
         menu.addAction(action_flatten);
         menu.addAction(action_export);
         //
         QObject::connect(action_flatten, &QAction::triggered, [canvas]() {
            _flatten_canvas(canvas);
         });
         QObject::connect(action_export, &QAction::triggered, [canvas]() {
            _export_canvas(canvas);
         });
         //
         menu.exec(canvas->mapToGlobal(pos));
      });
   }
}