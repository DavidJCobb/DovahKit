
local TRANSPARENT     = "#00000000"
local WATER_DEPTH     = 4096
local WATER_MIN_ALPHA = 0.5

local DEFAULT_LAND = { -- executable-level defaults: a LandTexture created at run-time with no form ID
   diffuse  = "Landscape\\" .. dovah.lookup_game_ini_setting("Landscape", "sDefaultLandDiffuseTexture"),
   normal   = "Landscape\\" .. dovah.lookup_game_ini_setting("Landscape", "sDefaultLandNormalTexture"),
   material = nil, -- TODO: set this to Default Object "DLMT"
}

local world = dovah.get_form_by_id(0x3C)
if not world then
   error("No worldspace to work with!")
end

local world_info = {
   heights = {
      land  = world.default_land_height,
      water = world.default_water_height,
   },
}

local window   = ui.window.new()
local panel    = ui.widget.new()
local scroll   = ui.scrollbox.new()
local canvas   = ui.canvas.new()
local progress = ui.progress_bar.new()
progress.alignment = "center center"
window.title = string.format("Heightmap for %s", world.name)

local panel_controls = {}

do
   window:set_layout("grid")
   window:add_child(scroll, 1, 1)
   window:add_child(panel,  1, 2)
   local sb = scroll.body
   sb:set_layout("grid")
   sb:add_child(canvas)
   window:add_child(progress, 2, 1, 1, 2)
   
   panel:set_layout("down")
   do
      local check = ui.checkbox.new("Show heightmap")
      check.checked = true
      check.enabled = false
      panel:add_child(check)
      --
      panel_controls.show_heightmap = check
   end
   do
      local check = ui.checkbox.new("Show landscape paint")
      check.checked = true
      check.enabled = false
      panel:add_child(check)
      --
      panel_controls.show_paint = check
   end
   do
      local check = ui.checkbox.new("Show vertex colors")
      check.checked = true
      check.enabled = false
      panel:add_child(check)
      --
      panel_controls.show_vert_colors = check
   end
   do
      local check = ui.checkbox.new("Show water")
      check.checked = true
      check.enabled = false
      panel:add_child(check)
      --
      panel_controls.show_water = check
   end
   
   do
      local tip = ui.text.new("TIP: It's normal for rivers and other water formations to be missing chunks. In order to allow for waterfalls at oblique angles relative to the compass, Bethesda will use placed water objects instead of cell and worldspace water.")
      tip.word_wrap = true
      window:add_child(tip, 3, 1, 1, 2)
   end
   
   window:show()
end

--
-- NOTE: Bethesda's coordinate system points northeast. Southwest is 
-- negative. Given a top-down view with north matching up, this would 
-- correspond to a vertical flip.
--
function do_grid_flip(x, y)
   return x, (33 - y + 1)
end

local cells = world:get_all_cells()
local count = #cells

progress.format  = "Checking world height range..."
progress.minimum = 0
progress.maximum = count
progress.value   = 0

local extents = nil
do
   function _update(t, v, b) -- call _update(tbl, v) or _update(tbl, min, max)
      if not t.min or v < t.min then
         t.min = v
      end
      if b then
         if not t.max or b > t.max then
            t.max = b
         end
      else
         if not t.max or v > t.max then
            t.max = v
         end
      end
   end
   --
   extents = {
      x = {
         min = nil,
         max = nil,
      },
      y = {
         min = nil,
         max = nil,
      },
      z = {
         min = nil,
         max = nil,
      },
      width  = nil,
      height = nil,
   }
   --
   local z_min = nil
   local z_max = nil
   for i = 1, count do
      local cell = cells[i]
      local land = cell.landscape
      if land then
         local min = land:get_minimum_height()
         local max = land:get_maximum_height()
         _update(extents.z, min, max)
      end
      local gc = cell.grid_coords
      local gx = gc.x
      local gy = -gc.y
      _update(extents.x, gx)
      _update(extents.y, gy)
      --
      progress.value = i
   end
   --
   if not extents.x.min then
      error("Unable to find any cells in this worldspace!")
   end
   if not extents.z.min then
      error("Unable to find any landscape data in this worldspace!")
   end
   --
   extents.width  = (extents.x.max - extents.x.min) + 1
   extents.height = (extents.y.max - extents.y.min) + 1
   --
   extents.z.span = extents.z.max - extents.z.min
   if extents.z.span == 0 then
      extents.z.span = 1
   end
end
dovah.dump(extents)

progress.format  = "Preparing canvas..."
progress.minimum = 0
progress.maximum = 0
progress.value   = 0

local rasters = {
   order = {
      "height",
      "paint",
      "color",
      "water",
   },
   render = {
      height = false,
      paint  = false,
      color  = false,
      water  = false,
   },
   working = {
      height = false,
      paint  = false,
      color  = false,
      water  = false,
   },
   layers = {
   },
   layer_configs = {
      paint = { opacity = 0.65, blend_mode = "multiply" },
      color = { blend_mode = "multiply" },
   },
}

local IMAGE_W = 32 * extents.width
local IMAGE_H = 32 * extents.height
canvas.width  = IMAGE_W
canvas.height = IMAGE_H
dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
do
   for i = 1, #rasters.order do
      local k = rasters.order[i]
      --
      rasters.working[k] = raster.new({
         width  = 32,
         height = 32,
         background_color = TRANSPARENT,
      })
      --
      rasters.render[k] = raster.new({
         width  = IMAGE_W,
         height = IMAGE_H,
         background_color = TRANSPARENT,
      })
      local layer = canvas:append_layer()
      layer.data = rasters.render[k]
      rasters.layers[k] = layer
      --
      local cfg = rasters.layer_configs[k]
      if cfg then
         local l, w = next(cfg)
         while l do
            layer[l] = w
            l, w = next(cfg, l)
         end
      end
   end
   
   local k, v
   --
   k, v = next(rasters.render, nil)
   while k do
      if not v then
         rasters.render[k] = raster.new({
            width  = IMAGE_W,
            height = IMAGE_H,
            background_color = TRANSPARENT,
         })
         local layer = canvas:append_layer()
         layer.data = rasters.render[k]
         rasters.layers[k] = layer
         --
         rasters.working[k] = raster.new({
            width  = 32,
            height = 32,
            background_color = TRANSPARENT,
         })
      end
      --
      k, v = next(rasters.render, k)
   end
end

progress.format  = "Drawing cells..."
progress.minimum = 0
progress.maximum = count
progress.value   = 0

local TextureManager = {
   map = {},
}
do
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

function _alpha_from_height(land, water)
   local diff = water - land
   if diff > 0 then
      local span  = 1 - WATER_MIN_ALPHA
      local alpha = (math.min(1, diff / WATER_DEPTH) * span) + WATER_MIN_ALPHA
      alpha = math.min(1, alpha)
      return math.ceil(alpha * 255)
   end
   return 0
end
function _for_each_quad(land, functor)
   local quads = land.quad
   functor(quad.bottom_left)
   functor(quad.bottom_right)
   functor(quad.top_left)
   functor(quad.top_right)
end

local LayerToggles = {
   --
   -- Enabling this system will allow the user to enable and disable layers while the heightmap 
   -- is drawing; however, it will also slow down drawing (significantly, in the debug build) 
   -- because we have to synchronize with the main thread (i.e. hit a lock) once per checkbox 
   -- that we test.
   --
   enabled = false,
   --
   checkboxes = {
      height = panel_controls.show_heightmap,
      paint  = panel_controls.show_paint,
      color  = panel_controls.show_vert_colors,
      water  = panel_controls.show_water
   },
   states = {},
}
if LayerToggles.enabled then
   local k, v = next(LayerToggles.checkboxes)
   while k do
      LayerToggles.states[k] = v.checked
      v.enabled = true
      k, v = next(LayerToggles.checkboxes, k)
   end
end
function LayerToggles:refresh()
   if not self.enabled then
      return
   end
   local k, v = next(self.checkboxes)
   while k do
      rasters.layers[k].visible = v.checked
      k, v = next(self.checkboxes, k)
   end
end
function LayerToggles:switch_to_listeners()
   if not self.enabled then
      local k, v = next(self.checkboxes)
      while k do
         v.enabled = true
         k, v = next(self.checkboxes, k)
      end
   end
   --
   function _update_layer_settings(name_to_alter, state)
      local names  = rasters.order
      local lowest = nil
      for i = 1, #names do
         local name  = names[i]
         local layer = rasters.layers[name]
         local vis
         if name == name_to_alter then
            layer.visible = state
            vis = state
         else
            vis = layer.visible
         end
         if vis and not lowest then
            lowest = layer
            layer.blend_mode = "normal"
            layer.opacity    = 1
         else
            local prefs = rasters.layer_configs[name] or {}
            layer.blend_mode = prefs.blend_mode or "normal"
            layer.opacity    = prefs.opacity    or 1
         end
      end
   end
   --
   panel_controls.show_heightmap:on("OnToggled", "", function(checked)
      _update_layer_settings("height", checked)
   end)
   --
   panel_controls.show_paint:on("OnToggled", "", function(checked)
      _update_layer_settings("paint", checked)
   end)
   --
   panel_controls.show_vert_colors:on("OnToggled", "", function(checked)
      _update_layer_settings("color", checked)
   end)
   --
   panel_controls.show_water:on("OnToggled", "", function(checked)
      _update_layer_settings("water", checked)
   end)
end

local bench = benchmark.new()
for i = 1, count do
   LayerToggles:refresh()
   local cell = cells[i]
   --
   local water_height = nil
   do
      local a = cell.water_height -- nil if we're defaulting to the world water height
      local b = world_info.heights.water
      if a then
         water_height = a
      else
         water_height = b or -9999999
      end
   end
   --
   local x = nil
   local y = nil
   do
      local gc = cell.grid_coords
      x  =  gc.x - extents.x.min
      y  = -gc.y - extents.y.min -- invert Y so that north is up
      x = x * 32
      y = y * 32
   end
   --
   local land = cell.landscape
   if land and land.enable_vertex_heights then
      rasters.working.water:fill(TRANSPARENT)
      --
      do
         for u = 2, 33 do -- leftmost col overlaps with western cell, so skip it
            for v = 1, 32 do -- bottom row overlaps with southern cell, so skip it
               local vcolor = land:get_color_at(u, v)
               local height = land:get_height_at(u, v)
               local shade  = (height - extents.z.min) / extents.z.span
               shade = math.floor(shade * 255) -- TODO: round
               --
               local a, b = do_grid_flip(u, v)
               a = a - 1 -- account for skipped col
               b = b - 1 -- account for skipped row
               rasters.working.height:set_pixel(a, b, { r = shade, g = shade, b = shade })
               rasters.working.color:set_pixel(a, b, vcolor)
               --
               if cell.has_water then -- water
                  local alpha = _alpha_from_height(height, water_height)
                  if alpha > 0 then
                     rasters.working.water:set_pixel(a, b, { r = 80, g = 160, b = 255, a = alpha })
                  end
               end
            end
         end
      end
      --
      rasters.working.paint:fill(TRANSPARENT)
      local quads = land.quads
      local _draw_quad = false
      do
         local QUAD_BOUNDS = {
            top_left = {
               vx = { min =  2, max = 17 },
               vy = { min = 18, max = 32 },
            },
            top_right = {
               vx = { min = 18, max = 33 },
               vy = { min = 18, max = 32 },
            },
            bottom_left = {
               vx = { min =  2, max = 17 },
               vy = { min =  1, max = 17 },
            },
            bottom_right = {
               vx = { min = 18, max = 33 },
               vy = { min =  1, max = 17 },
            },
         }
         _draw_quad = function(which, quad_list)
            local vx   = QUAD_BOUNDS[which].vx
            local vy   = QUAD_BOUNDS[which].vy
            local quad = quad_list[which]
            --
            do
               local ltex  = quad.default_texture
               local color = nil
               if ltex then
                  color = TextureManager:get_color(ltex)
               else
                  color = TextureManager:get_default_color()
               end
               color.a  = 255 -- force alpha
               color[4] = 255 -- force alpha
               local a, b = do_grid_flip(vx.min, vy.min)
               local c, d = do_grid_flip(vx.max, vy.max)
               a = a - 1
               --b = b - 1
               --c = c - 1
               d = d - 1
               rasters.working.paint:draw_rect({
                  from = { a, d },
                  to   = { c, b },
                  fill_color = color,
               })
            end
            quad:for_each_alpha_layer(function(layer)
               local ltex  = layer.texture
               local color = nil
               if ltex then
                  color = TextureManager:get_color(ltex)
               else
                  -- YES, blend layers can be null and therefore use the default texture too
                  color = TextureManager:get_default_color()
               end
               for u = vx.min, vx.max do
                  for v = vy.min, vy.max do
                     local opacity = layer:get_opacity_at_cell_position(u, v)
                     if opacity > 0 then
                        local a, b = do_grid_flip(u, v)
                        a = a - 1 -- account for skipped col
                        b = b - 1 -- account for skipped row
                        color.a  = math.max(0, math.min(255, math.ceil(opacity * 255))) -- force alpha
                        color[4] = color.a -- force alpha
                        rasters.working.paint:blend_pixel(a, b, color) -- set_pixel overwrites; blend_pixel blends
                     end
                  end
               end
            end)
         end
      end
      _draw_quad("top_left",     quads)
      _draw_quad("top_right",    quads)
      _draw_quad("bottom_left",  quads)
      _draw_quad("bottom_right", quads)
      --
      rasters.render.height:draw_raster(rasters.working.height, x + 1, y + 1)
      rasters.render.color:draw_raster (rasters.working.color,  x + 1, y + 1)
      rasters.render.water:draw_raster (rasters.working.water,  x + 1, y + 1)
      rasters.render.paint:draw_raster (rasters.working.paint,  x + 1, y + 1)
   else
      --
      -- The Creation Kit won't always encode a valid heightmap for a cell if the 
      -- cell's terrain exactly matches the worldspace's default land height -- 
      -- including the portions that overlap adjacent cells.
      --
      -- As of this writing, DovahKit can't remember whether a cell had a heightmap 
      -- or not, post-load, but it *can* remember whether the landscape was *flagged* 
      -- as having a heightmap, which is what we test here.
      --
      local height = world_info.heights.land
      local shade  = (height - extents.z.min) / extents.z.span
      shade = math.floor(shade * 255) -- TODO: round
      --
      rasters.working.height:fill({ r = shade, g = shade, b = shade })
      --
      rasters.render.height:draw_raster(rasters.working.height, x + 1, y + 1)
      --
      if cell.has_water and height < water_height then -- water
         local alpha = _alpha_from_height(height, water_height)
         if alpha > 0 then
            rasters.working.water:fill({ r = 80, g = 160, b = 255, a = alpha })
            rasters.render.water:draw_raster(rasters.working.water,  x + 1, y + 1)
         end
      end
   end
   progress.value = i
end

bench:stop()
dovah.log_message("Time taken: %s", bench:time_to_string())

LayerToggles:switch_to_listeners()