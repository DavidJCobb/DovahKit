#pragma once
#include <filesystem>
#include <memory>
#include "../sound_definition.h"

namespace dovahkit::subsystems::audio::utils {
   // Path is relative to the Data directory.
   extern std::shared_ptr<sound_definition> load_asset_as_sound_definition(std::filesystem::path);
}