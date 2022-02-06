#include "load_nif.h"
#include <QByteArray>
#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include "nif/file.h"

namespace DovahKitDebug::features {
   /*static*/ void load_nif::execute(QWidget* from) {
      auto name = QFileDialog::getOpenFileName(from, QObject::tr("Select BSA file", "debug"), "", "NetImmerse Format model (*.nif)");
      if (name.isEmpty())
         return;
      auto file = QFile(name);
      if (!file.open(QIODevice::OpenModeFlag::ReadOnly)) {
         qDebug() << "Unable to open file. " << file.errorString();
         return;
      }
      QByteArray data = file.readAll();
      //
      nifDK::file f;
      f.read((void*)data.constData(), data.size());
      __debugbreak();
      qDebug() << "load_nif debug tool done";
   }
}