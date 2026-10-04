#pragma once
#include <string>
#include <string_view>
#include "helpers/compile_time_strings/cs.h"

namespace dovah::utils::asset_paths {
   namespace impl {
      template<typename T>
      concept constructible_from_string_like = std::is_constructible_v<T, const std::string&> && std::is_constructible_v<T, const std::string_view&>;
   }

   template<const cobb::cs folder_name, impl::constructible_from_string_like Out = std::string>
   extern Out scope_to_folder(const std::string_view path);

   enum class top_level_asset_folder {
      meshes,
      sound,
      textures,
   };

   template<top_level_asset_folder Folder, impl::constructible_from_string_like Out = std::string>
   extern Out scope_to_folder(const std::string_view path) {
      switch (Folder) {
         using enum top_level_asset_folder;
         case meshes:   return scope_to_folder<cobb::cs("meshes"),  Out>(path);
         case sound:    return scope_to_folder<cobb::cs("sound"),   Out>(path);
         case textures: return scope_to_folder<cobb::cs("texture"), Out>(path);
      }
      return Out(path);
   }

   #pragma region for convenience
      template<impl::constructible_from_string_like Out = std::string>
      extern Out scope_to_meshes_folder(const std::string_view path) {
         return scope_to_folder<top_level_asset_folder::meshes, Out>(path);
      }
      template<impl::constructible_from_string_like Out = std::string>
      extern Out scope_to_textures_folder(const std::string_view path) {
         return scope_to_folder<top_level_asset_folder::textures, Out>(path);
      }
   #pragma endregion
}

#include "./scope_to_folder.inl"