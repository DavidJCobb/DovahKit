#pragma once
#include "../_base.h"

namespace DovahKitDebug::features::widgets {
   struct texture_asset_pane : debug_feature {
      static constexpr const char* name = "DKTextureAssetPane";
      static void execute(QWidget* from);
   };
}
