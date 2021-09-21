#pragma once
#include <QString>
#include "../../lua.h"

namespace dovahscript::api_helpers {
   struct load_resource_params {
      enum class source {
         game_assets,
         script_package,
      };

      source  load_from = source::game_assets;
      QString path;
      QString type; // overall type (e.g. "binary", "image", "raster", "text") or format (e.g. "dds", "png")
   };

   extern load_resource_params pull_load_resource_params(lua_State* L, int arg_index);

   extern int load_and_push_resource(lua_State* L, load_resource_params);
}