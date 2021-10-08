#include "DovahscriptSaveButton.h"
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTextStream>
#include "../../../../DirectXTex/DirectXTex.h"
#include "../../../helpers/intrusive_windows_defines.h"

namespace {
   constexpr auto executable_binary_extensions = std::array{
      "cpl", // Control Panel extension
      "dll", // Dynamic link library
      "exe", // Executable
      "gadget",
      "inf1",
      "ins", // Internet Settings
      "inx", // InstallShield script (compiled)
      "isu", // InstallShield script (uninstall)
      "job", // Windows task scheduler job file
      "lnk", // Shortcut
      "msc",
      "msi", // Microsoft installer
      "msp", // Microsoft installer (patch)
      "mst", // Microsoft installer (transform)
      "paf", // Portable application installer
      "pif", // Program Information File: a DOS-era program options format that Windows extended
      "scr", // Screensaver executable
      "sys", // System-level configuration and device driver files
      "u3p",
   };
   constexpr auto executable_script_extensions = std::array{
      "bat", // Batch file: a list of CMD instructions
      "com", // Command file
      "cmd",
      "hta", // HyperText Application: an HTML file capable of using system-level COM/ActiveX objects
      "jse", // JScript file
      "ps1", // PowerShell file
      "reg", // Registry data
      "rgs", // Registry script
      "sct",
      "shb",
      "shs", // Shell scrap object
      "vb",  // Visual Basic
      "vbe",
      "vbs",
      "vbscript",
      "ws",  // Windows script
      "wsf", // Windows script file
      "wsh", // Windows script preferences
   };
}

DovahscriptSaveButton::DovahscriptSaveButton(QWidget* parent) : QPushButton(parent) {
   this->setText(tr("Save file..."));
   QObject::connect(this, &QPushButton::clicked, this, &DovahscriptSaveButton::requestSave);
}

void DovahscriptSaveButton::requestSave() {
   if (!this->hasContent()) {
      QMessageBox::information(this, tr("Error"), tr("There isn't any data to save."));
      return;
   }
   QString path;
   QString extension;
   {
      QString filter;
      if (this->state.filename.isEmpty()) {
         const auto& fn = this->state.filename;
         int i = fn.lastIndexOf('.');
         if (i >= 0 && i < fn.size() - 1) {
            filter = QString("*.%1").arg(fn.mid(i));
         }
      }
      path = QStandardPaths::locate(QStandardPaths::StandardLocation::DocumentsLocation, QString(), QStandardPaths::LocateDirectory);
      if (!this->state.filename.isEmpty()) {
         if (!path.isEmpty()) {
            if (!path.endsWith('/'))
               path += '/';
            path += this->state.filename;
         }
      }
      path = QFileDialog::getSaveFileName(this,
         tr("Save file"),
         path,
         filter
      );
      if (path.isEmpty())
         return;
      int i = path.lastIndexOf('.');
      int j = path.lastIndexOf('/');
      if (i > j) {
         extension = path.mid(i + 1);
      }
   }
   QSaveFile file(path);
   if (!file.open(QIODevice::OpenModeFlag::WriteOnly)) {
      QMessageBox::critical(this, tr("Error"), tr("Unable to open the file for writing."));
      return;
   }
   if (this->contentIsText()) {
      QTextStream stream(&file);
      stream.setCodec("UTF-8");
      stream << this->state.content.text;
      file.commit();
   } else {
      auto& res  = this->state.content.resource;
      auto  type = res->resource_type();
      switch (type) {
         using _ = decltype(type);
         case _::binary:
         case _::undefined:
            {
               const auto data = res->get_binary_widget_side();
               file.write(data);
            }
            break;
         case _::dds:
            [[fallthrough]];
         case _::raster:
            if (extension.compare("dds", Qt::CaseInsensitive) == 0) {
               if (type == _::dds) {
                  auto* info = res->get_dds_metadata();
                  auto* data = res->get_dds_raw_data();
                  //
                  DirectX::Blob blob;
                  HRESULT result = SaveToDDSMemory(data->GetImages(), data->GetImageCount(), *info, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, blob);
                  if (FAILED(result)) {
                     QMessageBox::critical(this, tr("Error"), tr("Encountered an error when attempting to save the DDS file."));
                     return;
                  }
                  file.write((const char*)blob.GetBufferPointer(), qint64(blob.GetBufferSize()));
               } else {
                  auto image = res->get_raster_widget_side();
                  //
                  DirectX::Image raw;
                  raw.width      = image.width();
                  raw.height     = image.height();
                  raw.format     = DXGI_FORMAT_B8G8R8A8_UNORM;
                  raw.rowPitch   = 4 * raw.width;
                  raw.slicePitch = raw.rowPitch * raw.height;
                  raw.pixels     = (uint8_t*)image.constBits();
                  //
                  DirectX::Blob blob;
                  HRESULT result = SaveToDDSMemory(raw, DirectX::DDS_FLAGS::DDS_FLAGS_NONE, blob);
                  if (FAILED(result)) {
                     QMessageBox::critical(this, tr("Error"), tr("Encountered an error when attempting to save the DDS file."));
                     return;
                  }
                  file.write((const char*)blob.GetBufferPointer(), qint64(blob.GetBufferSize()));
               }
            } else {
               QImage image;
               if (type == _::dds) {
                  image = res->get_dds_layer(0, 0, 0);
               } else {
                  image = res->get_raster_widget_side();
               }
               auto result = image.save(&file, extension.toUtf8().constData());
               if (!result) {
                  QMessageBox::critical(this, tr("Error"), tr("Unable to save the image in this format. %1").arg(file.errorString()));
                  return;
               }
            }
            break;
      }
      file.commit();
   }
   QMessageBox::information(this, tr("File saved"), tr("Your file has been saved to:\n\n%1").arg(path));
}

void DovahscriptSaveButton::setContent(DSRH resource) {
   this->state.content.text.clear();
   this->state.content.resource = resource;
}
void DovahscriptSaveButton::setContent(const QString& text) {
   if (this->state.content.text == text)
      return;
   this->state.content.text     = text;
   this->state.content.resource = nullptr;
}

void DovahscriptSaveButton::setDesiredFilename(QString name) {
   int i = name.lastIndexOf('/');
   int j = name.lastIndexOf('\\');
   i = std::max(i, j);
   if (i >= 0) {
      name = name.mid(i + 1);
   }
   j = name.lastIndexOf('.');
   if (j >= 0) {
      QString extension = name.mid(j + 1).toLower();
      QString replace;
      for (const auto* test : executable_binary_extensions) {
         if (extension == test) {
            replace = "bin";
            break;
         }
      }
      if (replace.isEmpty()) {
         for (const auto* test : executable_script_extensions) {
            if (extension == test) {
               replace = "txt";
               break;
            }
         }
      }
      //
      if (!replace.isEmpty()) {
         name = name.mid(0, j + 1) + replace;
      }
   }
   this->state.filename = name;
}

void DovahscriptSaveButton::setText(const QString& label) {
   if (label == this->text())
      return;
   this->state.text = label;
   if (label.isEmpty()) {
      QPushButton::setText(tr("Save file..."));
   } else {
      QPushButton::setText(tr("Save file: %1").arg(label));
   }
}