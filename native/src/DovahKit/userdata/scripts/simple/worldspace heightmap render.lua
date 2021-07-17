
local TRANSPARENT     = "#00000000"
local WATER_DEPTH     = 4096
local WATER_MIN_ALPHA = 0.5

local DEFAULT_LAND = { -- executable-level defaults: a LandTexture created at run-time with no form ID
   diffuse  = "Landscape\\" .. dovah.lookup_game_ini_setting("Landscape", "sDefaultLandDiffuseTexture"),
   normal   = "Landscape\\" .. dovah.lookup_game_ini_setting("Landscape", "sDefaultLandNormalTexture"),
   material = nil, -- TODO: set this to Default Object "DLMT"
}

-- List of internal names for layers. The render process refers to layers 
-- directly and will expect them to exist, but UI setup processes and 
-- similar rely on this struct to create, arrange, and configure layers, 
-- as well as for associated UI widgets e.g. toggle checkboxes.
--
-- Additionally, we can configure a limited range of layer properties such 
-- as the opacity and blend mode, and we can control layer order as well 
-- (first = lowest). If we want to configure additional properties, we 
-- must update the code that creates layers and the code that toggles a 
-- layer's visibility.
local LAYER_SPEC = {
   {
      name       = "height",
      check_text = "Show heightmap",
   },
   {
      name       = "paint",
      blend_mode = "multiply",
      opacity    = 0.65,
      check_text = "Show landscape paint",
   },
   {
      name       = "color",
      blend_mode = "multiply",
      check_text = "Show vertex colors",
   },
   {
      name       = "water",
      check_text = "Show water",
   },
}

---

local FileOutlineWidget = {}
setmetatable(FileOutlineWidget, { __index = FileOutlineWidget })
do -- FileOutlineWidget contents
   function FileOutlineWidget:new(options)
      options = options or {}
      local instance = setmetatable({}, self)
      --
      instance.widget = ui.groupbox.new(options.name)
      instance.color  = options.color or "#FF0000"
      instance._controls = {
         root  = instance.widget,
         color = nil,
      }
      instance.widget:on("OnToggled", "", function(checked)
         instance:onToggled(checked)
      end)
      --
      return instance
   end
   function FileOutlineWidget:onToggled(checked)
      -- TODO
   end
end

local HeightmapWindow = {
   controls = {
      window   = ui.window.new(),
      scroll   = ui.scrollbox.new(),
      canvas   = ui.canvas.new(),
      progress = ui.progress_bar.new(),
      --
      config   = ui.widget.new(),
      bottom   = ui.widget.new(),
      --
      options = {
         world = false,
         layer_visibility = {}, -- list of checkboxes
         execute = false,
      },
   },
   state = {
      layers = {},
      layer_visibility = {},
   },
}
do -- HeightmapWindow contents
   do -- Initial config for HeightmapWindow.
      for i = 1, #LAYER_SPEC do
         local spec = LAYER_SPEC[i]
         local name = spec.name
         HeightmapWindow.state.layer_visibility[name] = true
      end
   end
   do -- Create widgets and layout for HeightmapWindow.
      local window = HeightmapWindow.controls.window
      window.title = "Heightmap"
      window:set_layout("grid")
      --
      local scroll = HeightmapWindow.controls.scroll
      local config = HeightmapWindow.controls.config
      window:add_child(scroll, 1, 1)
      window:add_child(config, 1, 2)
      window:set_layout_stretch_at("col", 1, 1)
      window:set_layout_stretch_at("col", 2, 0)
      do
         local sb = scroll.body
         sb:set_layout("grid")
         sb:add_child(HeightmapWindow.controls.canvas)
      end
      --
      do
         local bottom = HeightmapWindow.controls.bottom
         window:add_child(bottom, 2, 1, 1, 2)
         bottom:set_layout("down")
         bottom.layout_margins = 0
         --
         local progress = HeightmapWindow.controls.progress
         bottom:add_child(progress)
         progress.alignment = "center center"
         --
         do
            local tip = ui.text.new("TIP: It's normal for rivers and other water formations to be missing chunks. In order to allow for waterfalls at oblique angles relative to the compass, Bethesda will use placed water objects instead of cell and worldspace water.")
            tip.word_wrap = true
            bottom:add_child(tip)
         end
      end
      --
      do -- Create options
         config:set_layout("down")
         config.layout_margins = 0
         --
         local oc = HeightmapWindow.controls.options
         local lv = oc.layer_visibility
         do
            local picker = ui.formpicker.new()
            picker.form_types   = form_types.worldspace
            picker.default_form = dovah.get_form_by_id(0x3C)
            picker.allow_none   = false
            config:add_child(picker)
            --
            oc.world = picker
         end
         for i = 1, #LAYER_SPEC do
            local spec = LAYER_SPEC[i]
            local name = spec.name
            local text = spec.check_text
            --
            lv[name] = ui.checkbox.new(text)
            lv[name].checked = true
            lv[name]:on("OnToggled", "", function(checked)
               HeightmapWindow:set_layer_visibility(name, checked)
            end)
            config:add_child(lv[name])
         end
         do
            local button = ui.button.new("Render")
            config:add_child(button)
            --
            oc.execute = button
         end
         --
         config:add_spacer("v")
      end
   end
   function HeightmapWindow:clear_canvas()
      local canvas = self.controls.canvas
      canvas.width  = 1
      canvas.height = 1
      local layers = {}
      do
         local source = canvas.layers
         local count  = #source
         for i = 1, count do
            layers[i] = source[i]
         end
      end
      for i = 1, #layers do
         layers[i]:delete()
      end
      self.state.layers = {}
   end
   function HeightmapWindow:set_is_locked(state)
      self.controls.config.enabled = not state
   end
   function HeightmapWindow:set_layer_visibility(name_to_alter, state)
      local lowest = nil
      for i = 1, #LAYER_SPEC do
         local spec  = LAYER_SPEC[i]
         local name  = spec.name
         local layer = HeightmapWindow.state.layers[name]
         local show  = HeightmapWindow.state.layer_visibility[name]
         if name == name_to_alter then
            if layer then -- user can configure settings before rendering i.e. before layers exist
               layer.visible = state
            end
            HeightmapWindow.state.layer_visibility[name] = state
            show = state
         end
         if layer then -- user can configure settings before rendering i.e. before layers exist
            if show and not lowest then
               lowest = layer
               layer.blend_mode = "normal"
               layer.opacity    = 1
            else
               if spec.blend_mode then
                  layer.blend_mode = spec.blend_mode
               end
               if spec.opacity then
                  layer.opacity = spec.opacity
               end
            end
         end
      end
   end
   function HeightmapWindow:show()
      self.controls.window:show()
   end
end

local TextureManager = {
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
      local dds <close> = dovah.lookup_game_asset("textures/" .. path)
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
      local dds <close> = dovah.lookup_game_asset("textures/" .. path)
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

local render_worldspace_height = nil
do
   local canvas   = HeightmapWindow.controls.canvas
   local progress = HeightmapWindow.controls.progress
   function _update_progress(text, min, max, value)
      local p = HeightmapWindow.controls.progress
      if text then
         p.format = text
      end
      if min then
         p.minimum = min
      end
      if max then
         p.maximum = max
      end
      if value then
         p.value = value
      end
   end
   
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

   render_worldspace_height = function()
      local world = nil
      local world_info = nil
      do
         world = HeightmapWindow.controls.options.world.form
         if not world then
            dovah.log_message("No worldspace to work with!")
            return
         end
         world_info = {
            heights = {
               land  = world.default_land_height,
               water = world.default_water_height,
            },
         }
      end
      
      local cells   = world:get_all_cells()
      local count   = #cells
      local extents = nil
      local rasters = nil
      
      _update_progress("Checking world height range...", 0, count, 0)
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
            dovah.log_message("Unable to find any cells in this worldspace!")
            return
         end
         if not extents.z.min then
            dovah.log_message("Unable to find any landscape data in this worldspace!")
            return
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
      
      _update_progress("Preparing canvas...", 0, 0, 0)
      do
         rasters = {
            render  = {},
            working = {},
         }
         --
         local IMAGE_W = 32 * extents.width
         local IMAGE_H = 32 * extents.height
         canvas.width  = IMAGE_W
         canvas.height = IMAGE_H
         dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
         do
            for i = 1, #LAYER_SPEC do
               local spec = LAYER_SPEC[i]
               local name = spec.name
               --
               rasters.working[name] = raster.new({
                  width  = 32,
                  height = 32,
                  background_color = TRANSPARENT,
               })
               --
               rasters.render[name] = raster.new({
                  width  = IMAGE_W,
                  height = IMAGE_H,
                  background_color = TRANSPARENT,
               })
               local layer = canvas:append_layer()
               layer.data = rasters.render[name]
               HeightmapWindow.state.layers[name] = layer
               if spec.blend_mode then
                  layer.blend_mode = spec.blend_mode
               end
               if spec.opacity then
                  layer.opacity = spec.opacity
               end
            end
         end
      end
      
      _update_progress("Drawing cells...", 0, count, 0)
      do
         local rw = rasters.working
         local rr = rasters.render
         
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
         function _do_grid_flip(x, y)
            return x, (33 - y + 1)
         end
         function _for_each_quad(land, functor)
            local quads = land.quads
            functor("bottom_left",  quads.bottom_left)
            functor("bottom_right", quads.bottom_right)
            functor("top_left",     quads.top_left)
            functor("top_right",    quads.top_right)
         end
         
         local benchmark = dovah.benchmark_start()
         for i = 1, count do
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
               rw.water:fill(TRANSPARENT)
               --
               do
                  for u = 2, 33 do -- leftmost col overlaps with western cell, so skip it
                     for v = 1, 32 do -- bottom row overlaps with southern cell, so skip it
                        local vcolor = land:get_color_at(u, v)
                        local height = land:get_height_at(u, v)
                        local shade  = (height - extents.z.min) / extents.z.span
                        shade = math.floor(shade * 255) -- TODO: round
                        --
                        local a, b = _do_grid_flip(u, v)
                        a = a - 1 -- account for skipped col
                        b = b - 1 -- account for skipped row
                        rw.height:set_pixel(a, b, { r = shade, g = shade, b = shade })
                        rw.color:set_pixel(a, b, vcolor)
                        --
                        if cell.has_water then -- water
                           local alpha = _alpha_from_height(height, water_height)
                           if alpha > 0 then
                              rw.water:set_pixel(a, b, { r = 80, g = 160, b = 255, a = alpha })
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
                  _draw_quad = function(which, quad)
                     local vx = QUAD_BOUNDS[which].vx
                     local vy = QUAD_BOUNDS[which].vy
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
                        local a, b = _do_grid_flip(vx.min, vy.min)
                        local c, d = _do_grid_flip(vx.max, vy.max)
                        a = a - 1
                        d = d - 1
                        rw.paint:draw_rect({
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
                                 local a, b = _do_grid_flip(u, v)
                                 a = a - 1 -- account for skipped col
                                 b = b - 1 -- account for skipped row
                                 color.a  = math.max(0, math.min(255, math.ceil(opacity * 255))) -- force alpha
                                 color[4] = color.a -- force alpha
                                 rw.paint:blend_pixel(a, b, color) -- set_pixel overwrites; blend_pixel blends
                              end
                           end
                        end
                     end)
                  end
               end
               _for_each_quad(land, _draw_quad)
               --
               rr.height:draw_raster(rw.height, x + 1, y + 1)
               rr.color:draw_raster (rw.color,  x + 1, y + 1)
               rr.water:draw_raster (rw.water,  x + 1, y + 1)
               rr.paint:draw_raster (rw.paint,  x + 1, y + 1)
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
               rw.height:fill({ r = shade, g = shade, b = shade })
               rr.height:draw_raster(rw.height, x + 1, y + 1)
               --
               if cell.has_water and height < water_height then -- water
                  local alpha = _alpha_from_height(height, water_height)
                  if alpha > 0 then
                     rw.water:fill({ r = 80, g = 160, b = 255, a = alpha })
                     rr.water:draw_raster(rw.water,  x + 1, y + 1)
                  end
               end
            end
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
      end
   end
end

HeightmapWindow.controls.options.execute:on("OnActivated", "render", function()
   HeightmapWindow:set_is_locked(true)
   HeightmapWindow:clear_canvas()
   render_worldspace_height()
   HeightmapWindow:set_is_locked(false)
end)
HeightmapWindow:show()
