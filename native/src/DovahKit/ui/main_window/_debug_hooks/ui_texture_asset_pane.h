#pragma once
#include "_base.h"
#include <QLabel>

namespace DovahKitDebug {
   namespace features {
      struct ui_texture_asset_pane : debug_feature {
         static constexpr const char* name = "DKTextureAssetPane test";
         static void execute(QWidget* from);
      };
   }
}
