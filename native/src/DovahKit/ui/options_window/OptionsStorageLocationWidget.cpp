#include "./OptionsStorageLocationWidget.h"
#include <QBoxLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

// for opening the file/folder
#include <QApplication> // literally just for the beep function
#include <QDesktopServices>
#include <QUrl>
#include "helpers/arrays/make.h"

//
// This widget isn't super robust and it isn't meant to be. It doesn't gracefully handle edge-cases 
// like the path being empty, the targeted file/folder not existing, et cetera. The file extension 
// checks are just mild paranoia; the widget as a whole is not rigorous.
//

namespace {
   constexpr const bool block_anything_weird = true;
   
   // lists below copied from the DovahscriptSaveButton widget code

   constexpr const auto executable_binary_extensions = cobb::arrays::make(
      "appx", // UWP app or installer thereof
      "cpl", // Control Panel extension
      "dll", // Dynamic link library
      "exe", // Executable
      "gadget",
      "inf1",
      "ins", // Internet Settings
      "inx", // InstallShield script (compiled)
      "isu", // InstallShield script (uninstall)
      "jar", // Java archive (can be executable)
      "job", // Windows task scheduler job file
      "lnk", // Shortcut
      "msc",
      "msi", // Microsoft installer
      "msix", // Microsoft installer (new)
      "msp", // Microsoft installer (patch)
      "mst", // Microsoft installer (transform)
      "paf", // Portable application installer
      "pif", // Program Information File: a DOS-era program options format that Windows extended
      "scr", // Screensaver executable
      "sys", // System-level configuration and device driver files
      "u3p"//,
   );
   constexpr const auto executable_script_extensions = cobb::arrays::make(
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
      "wsh"//, // Windows script preferences
   );
}

OptionsStorageLocationWidget::OptionsStorageLocationWidget(QWidget* parent) : QWidget(parent) {
   auto* groupbox = this->_subwidgets.groupbox = new QGroupBox(this);
   {
      auto* layout = new QVBoxLayout(this);
      layout->addWidget(groupbox);
      layout->setContentsMargins(0, 0, 0, 0);
   }
   this->setContentsMargins(0, 0, 0, 0);

   auto* layout_main  = new QVBoxLayout(groupbox);
   auto* description  = this->_subwidgets.description = new QLabel(groupbox);
   description->setWordWrap(true);
   description->setVisible(false);
   layout_main->addWidget(description);

   auto* strip        = new QWidget(groupbox);
   auto* layout_cross = new QHBoxLayout(strip);
   layout_cross->setContentsMargins(0, 0, 0, 0);
   layout_main->addWidget(strip);

   auto* textbox = this->_subwidgets.path = new QLineEdit(strip);
   textbox->setReadOnly(true);
   layout_cross->addWidget(textbox);

   auto* button_folder = this->_subwidgets.open_folder = new QPushButton(strip);
   button_folder->setAccessibleName(tr("Open containing folder"));
   {
      QIcon icon;
      icon.addFile(QString::fromUtf8(":/icons/open-folder-16.png"), QSize(), QIcon::Normal, QIcon::Off);
      button_folder->setIcon(icon);
   }
   layout_cross->addWidget(button_folder);

   auto* button_file = this->_subwidgets.open_file = new QPushButton(strip);
   button_file->setAccessibleName(tr("Open this file"));
   {
      QIcon icon;
      icon.addFile(QString::fromUtf8(":/icons/dogeared-page-16.png"), QSize(), QIcon::Normal, QIcon::Off);
      button_file->setIcon(icon);
   }
   layout_cross->addWidget(button_file);

   button_folder->setEnabled(false);
   button_file->setEnabled(false);

   QObject::connect(button_folder, &QPushButton::clicked, this, [this]() {
      QDir path = this->_path;
      if (this->_isFile)
         path.cdUp();

      QDesktopServices::openUrl(QUrl::fromLocalFile(path.absolutePath() + '/'));
   });
   QObject::connect(button_file, &QPushButton::clicked, this, [this]() {
      if (!this->_isFile)
         return;

      auto path = this->_path.absolutePath();
      if constexpr (block_anything_weird) {
         auto filename = this->_path.dirName();
         auto i = filename.lastIndexOf('.');
         if (i >= 0) {
            auto extension = filename.mid(i + 1);

            bool sus = false;
            for (const auto* test : executable_binary_extensions) {
               if (extension.compare(test, Qt::CaseInsensitive) == 0) {
                  sus = true;
                  break;
               }
            }
            if (!sus) {
               for (const auto* test : executable_script_extensions) {
                  if (extension.compare(test, Qt::CaseInsensitive) == 0) {
                     sus = true;
                     break;
                  }
               }
            }
            if (sus) {
               QApplication::beep();
               return;
            }
         }
      }
      QDesktopServices::openUrl(QUrl::fromLocalFile(path));
   });
}

void OptionsStorageLocationWidget::setName(QString v) {
   if (v == this->_name)
      return;
   this->_name = v;
   this->_subwidgets.groupbox->setTitle(v);
}
void OptionsStorageLocationWidget::setDescription(QString v) {
   if (v == this->_description)
      return;
   this->_description = v;
   this->_subwidgets.description->setText(v);
   this->_subwidgets.description->setVisible(!v.isEmpty());
}
void OptionsStorageLocationWidget::setFilePath(QDir v) {
   v.makeAbsolute();
   if (v == this->_path && this->_isFile)
      return;
   this->_path = v;
   this->_subwidgets.path->setText(v.absolutePath());
   this->_setIsFile(true);

   this->_subwidgets.open_file->setEnabled(true);
   v.cdUp();
   this->_subwidgets.open_folder->setEnabled(true);
}
void OptionsStorageLocationWidget::setFolderPath(QDir v) {
   v.makeAbsolute();
   if (v == this->_path && !this->_isFile)
      return;
   this->_path = v;
   this->_subwidgets.path->setText(v.absolutePath());
   this->_setIsFile(false);

   this->_subwidgets.open_file->setEnabled(false);
   this->_subwidgets.open_folder->setEnabled(true);
}

QString OptionsStorageLocationWidget::name() const {
   return this->_name;
}
QString OptionsStorageLocationWidget::description() const {
   return this->_description;
}
bool OptionsStorageLocationWidget::isFile() const {
   return this->_isFile;
}
bool OptionsStorageLocationWidget::isFolder() const {
   return !this->isFile();
}
QDir OptionsStorageLocationWidget::path() const {
   return this->_path;
}

void OptionsStorageLocationWidget::_setIsFile(bool v) {
   if (this->_isFile == v)
      return;
   this->_isFile = v;
   this->_subwidgets.open_file->setVisible(v);

   auto* button_folder = this->_subwidgets.open_folder;
   if (v) {
      button_folder->setAccessibleName(tr("Open containing folder"));
   } else {
      button_folder->setAccessibleName(tr("Open folder"));
   }
}