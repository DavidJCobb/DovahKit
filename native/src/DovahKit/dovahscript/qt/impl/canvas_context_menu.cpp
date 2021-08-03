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

// TODO: clean all this up; maybe make a worker/state struct for it

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

   void _text_for_entity(int index, CanvasWidgetEntity* entity, QString& plaintext) {
      QString out;
      if (entity->isLayerGroup())
         out = QObject::tr("Group %1 %2", "script canvas layer export");
      else
         out = QObject::tr("Layer %1 %2", "script canvas layer export");
      out = out.arg(index);
      {
         auto name = entity->objectName();
         if (name.isEmpty())
            out = out.arg(name);
         else
            out = out.arg(QString("(%1)").arg(name));
      }
      out += '\n';
      out += QObject::tr(" - Visible:    %1\n").arg(entity->visible() ? "Yes" : "No");
      out += QObject::tr(" - Position:   (%1, %2)\n").arg(entity->position().x()).arg(entity->position().y());
      out += QObject::tr(" - Opacity:    %1\n").arg(entity->opacity());
      out += QObject::tr(" - Blend Mode: ");
      switch (entity->compositionMode()) {
         case CanvasWidgetEntity::CompositionMode::CompositionMode_SourceOver:
            out += QObject::tr("Normal");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_ColorBurn:
            out += QObject::tr("Burn");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_ColorDodge:
            out += QObject::tr("Dodge");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Darken:
            out += QObject::tr("Darken Only");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Difference:
            out += QObject::tr("Difference");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_HardLight:
            out += QObject::tr("Hard Light");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Lighten:
            out += QObject::tr("Lighen Only");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Multiply:
            out += QObject::tr("Multiply");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Overlay:
            out += QObject::tr("Overlay");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Plus:
            out += QObject::tr("Add");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_Screen:
            out += QObject::tr("Screen");
            break;
         case CanvasWidgetEntity::CompositionMode::CompositionMode_SoftLight:
            out += QObject::tr("Soft Light");
            break;
         default:
            out += QObject::tr("Unknown (%1)").arg((int)entity->compositionMode());
            break;
      }
      out += "\n\n";
      //
      plaintext += out;
   }
   void _traverse_layers(CanvasWidgetLayerGroup* group, QVector<CanvasWidgetLayer*>& layers, QVector<CanvasWidgetLayerGroup*>& groups) {
      for (auto* entity : group->childLayers()) {
         if (entity->isLayerGroup()) {
            auto* nested = (CanvasWidgetLayerGroup*)entity;
            groups.push_back(nested);
            _traverse_layers(nested, layers, groups);
         } else {
            auto* layer = (CanvasWidgetLayer*)entity;
            layers.push_back(layer);
         }
      }
   }
   void _print_layer_structure(CanvasWidgetLayerGroup* group, QVector<CanvasWidgetLayer*>& layers, QVector<CanvasWidgetLayerGroup*>& groups, QString& plaintext, QString indent = QString()) {
      for (auto* entity : group->childLayers()) {
         int  position = -1;
         bool is_group = entity->isLayerGroup();
         CanvasWidgetLayerGroup* nested = nullptr;
         if (is_group) {
            nested = (CanvasWidgetLayerGroup*)entity;
            position = groups.indexOf(nested);
         } else {
            auto* layer = (CanvasWidgetLayer*)entity;
            position = layers.indexOf(layer);
         }
         assert(position >= 0);
         auto name = entity->objectName();
         if (!name.isEmpty())
            name = QString("(%1)").arg(name);
         //
         plaintext += QObject::tr("%1 - %2 #%3 %4\n").arg(indent).arg(is_group ? "Group" : "Layer").arg(position + 1).arg(name);
         if (nested)
            _print_layer_structure(nested, layers, groups, plaintext, indent + "   ");
      }
   }
   template<typename T> void _append_to_buffer(QByteArray& ba, T value) {
      ba.append(&value, sizeof(T));
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
      QString plaintext;
      //
      QVector<CanvasWidgetLayer*> layers;
      QVector<CanvasWidgetLayerGroup*> groups;
      for (auto* entity : canvas->layers()) {
         if (entity->isLayerGroup()) {
            auto* group = (CanvasWidgetLayerGroup*)entity;
            groups.push_back(group);
            _traverse_layers(group, layers, groups);
         } else {
            auto* layer = (CanvasWidgetLayer*)entity;
            layers.push_back(layer);
         }
      }
      {
         plaintext += QObject::tr("== Groups ==\n\n", "script canvas layer export");
         auto size = groups.size();
         for (int i = 0; i < size; ++i)
            _text_for_entity(i + 1, groups[i], plaintext);
      }
      {
         plaintext += QObject::tr("== Layers ==\n\n", "script canvas layer export");
         auto size = layers.size();
         for (int i = 0; i < size; ++i)
            _text_for_entity(i + 1, layers[i], plaintext);
      }
      plaintext += QObject::tr("== Structure ==\n", "script canvas layer export");
      for (auto* entity : canvas->layers()) {
         auto name = entity->objectName();
         if (!name.isEmpty())
            name = QString("(%1)").arg(name);
         if (entity->isLayerGroup()) {
            auto* group = (CanvasWidgetLayerGroup*) entity;
            plaintext += QObject::tr(" - Group #%1 %2\n").arg(groups.indexOf(group) + 1).arg(name);
            _print_layer_structure(group, layers, groups, plaintext, "   ");
         } else {
            auto* layer = (CanvasWidgetLayer*) entity;
            plaintext += QObject::tr(" - Layer #%1 %2\n").arg(layers.indexOf(layer) + 1).arg(name);
         }
      }
      //
      {
         {
            auto pt = plaintext.toUtf8();
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
         int size = layers.size();
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
                  QString("layer %1.png").arg(i).toUtf8(),
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