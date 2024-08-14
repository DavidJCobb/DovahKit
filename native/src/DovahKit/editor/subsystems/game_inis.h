#pragma once
#include "../../helpers/qt/ini.h"
#include "../../dovah/data/game.h"

namespace editor::game_inis {
   extern cobb::qt::ini::File& get_skyrim();
   extern cobb::qt::ini::File& get_skyrim_prefs();

   extern void load_inis(dovah::game); // loaded settings are reset to defaults, and then loaded again; the reset does not emit value-changed signals
}