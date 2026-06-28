#include "./icon_for_file_extension.h"
#include <QMimeDatabase>
#ifdef _WIN32
   #include "helpers/windows.h"
#endif

namespace {
   #ifdef _WIN32
      QIcon _get_icon_from_windows(const QString& extension) {
         constexpr std::array sizes       = { 0, SHGFI_SMALLICON, SHGFI_LARGEICON, SHGFI_SHELLICONSIZE };
         constexpr std::array state_flags = {
            std::pair{ QIcon::Normal,   0 },
            std::pair{ QIcon::Selected, SHGFI_SELECTED },
         };
         
         QIcon icon;
         
         auto extension_win = extension.toStdWString();
         extension_win.insert(extension_win.begin(), decltype(extension_win)::value_type('.'));
         
         SHFILEINFO info = {};
         for (auto s : sizes) {
            for (const auto [qt_state, w32_state] : state_flags) {
               HRESULT hr = SHGetFileInfoW(extension_win.c_str(), FILE_ATTRIBUTE_NORMAL, &info, sizeof(info), SHGFI_ICON | SHGFI_USEFILEATTRIBUTES | s | w32_state);
               if (!SUCCEEDED(hr))
                  continue;
               if (info.hIcon == NULL)
                  continue;
               auto pm = QPixmap::fromImage(QImage::fromHICON(info.hIcon));
               DestroyIcon(info.hIcon);
               if (pm.isNull())
                  continue;
               icon.addPixmap(pm, qt_state);
            }
         }
         
         return icon;
      }
   #endif
}

namespace dovahkit::qt::utils {
   extern QIcon icon_for_file_extension(QString extension) {
      QIcon icon;
      #ifdef _WIN32
         if (icon.isNull()) {
            icon = _get_icon_from_windows(extension);
            if (!icon.isNull())
               return icon;
         }
      #endif
      const auto types = QMimeDatabase{}.mimeTypesForFileName(QString("x.%1").arg(extension));
      for (const auto& type : types) {
         icon = QIcon::fromTheme(type.iconName());
         if (!icon.isNull())
            break;
      }
      return icon;
   }
}