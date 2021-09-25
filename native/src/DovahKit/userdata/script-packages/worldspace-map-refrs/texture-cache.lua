local TRANSPARENT  = "#00000000"
local DEFAULT_LAND = { -- executable-level defaults: a LandTexture created at run-time with no form ID
   diffuse  = nil,
   normal   = nil,
   material = nil, -- TODO: set this to Default Object "DLMT"
}
do
   local s = dovah.lookup_game_ini_setting("Skyrim.ini", "Landscape", "sDefaultLandDiffuseTexture")
   if s then
      DEFAULT_LAND.diffuse = "Landscape\\" .. s.current_value
   else
      error("Unable to get the default land diffuse texture. Failed to access game INI settings.")
   end
   s = dovah.lookup_game_ini_setting("Skyrim.ini", "Landscape", "sDefaultLandNormalTexture")
   if s then
      DEFAULT_LAND.normal = "Landscape\\" .. s.current_value
   else
      error("Unable to get the default land diffuse texture. Failed to access game INI settings.")
   end
end

TextureManager = {
   map = {},
}
do -- TextureManager contents
   function _try_get_smallest_mip(dds)
      local image = dds.images[1]
      if not image then
         return
      end
      local mips = image.mipmaps
      local last = #mips
      local data = mips[last]
      if data then
         return data:copy_to_raster()
      end
   end
   function _resize_dds(dds)
      local ras = _try_get_smallest_mip(dds)
      if not ras then
         ras = dds:copy_to_raster()
      end
      ras:resize(1, 1)
      return ras
   end
   function _adjust_color(color)
      return
   end
   
   function TextureManager:get_color(land_texture)
      local ts = land_texture.texture_set
      if not ts then
         return TRANSPARENT
      end
      local path = ts.textures.diffuse
      if not path or path == "" then
         return TRANSPARENT
      end
      if self.map[path] then
         return self.map[path]
      end
      --
      local dds <close> = dovah.load_game_asset("textures/" .. path)
      if dovah.type(dds) ~= "dds_resource" then
         self.map[path] = TRANSPARENT
         return TRANSPARENT
      end
      local raster <close> = _resize_dds(dds)
      --
      local color = raster:get_pixel(1, 1)
      _adjust_color(color)
      color.a  = 255
      color[4] = 255 -- alpha in land textures means something else (parallax?)
      self.map[path] = color
      --dovah.log_message("Color for %s: (%s, %s, %s, %s)", path, color.r, color.g, color.b, color.a)
      return color
   end
   function TextureManager:get_default_color()
      if self.default_color then
         return self.default_color
      end
      local path = DEFAULT_LAND.diffuse
      --
      local dds <close> = dovah.load_game_asset("textures/" .. path)
      if dovah.type(dds) ~= "dds_resource" then
         self.default_color = TRANSPARENT
         return TRANSPARENT
      end
      local raster <close> = dds:copy_to_raster()
      raster:resize(1, 1)
      --
      local color = raster:get_pixel(1, 1)
      _adjust_color(color)
      color.a  = 255
      color[4] = 255 -- alpha in land textures means something else (parallax?)
      self.default_color = color
      return color
   end
end