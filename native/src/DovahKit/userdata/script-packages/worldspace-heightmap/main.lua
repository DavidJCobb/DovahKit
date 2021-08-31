if not HeightmapWindow then
   error("missing dependency")
end

local CELL_SIDE_SIZE = 4096
local TRANSPARENT    = "#00000000"

-- Comprehensive but slow; haven't yet found cases where it's actually needed
local TEST_FOR_PERSISTENT_REFS = false

--

render_worldspace_height = nil
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
      
      local world_first_file = nil -- we shouldn't outline cells defined in the same file as the worldspace, because that'll be nearly all of them
      local world_filenames  = {}
      do
         local list = world:get_source_file_list()
         local file = list[1]
         if file then
            world_first_file = file.filename
         end
         for i = 2, #list do
            world_filenames[i - 1] = list[i].filename
         end
      end
      local all_maps = CellOutlineMapAllFiles:new()
      for i = 1, #world_filenames do
         all_maps:get_or_create_file(world_filenames[i])
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
            local lowest = nil
            for i = 1, #LAYER_SPEC do
               local spec = LAYER_SPEC[i]
               local name = spec.name
               local show = HeightmapWindow.state.layer_visibility[name]
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
               layer.name = name
               HeightmapWindow.state.layers[name] = layer
               if lowest then
                  if spec.blend_mode then
                     layer.blend_mode = spec.blend_mode
                  end
                  if spec.opacity then
                     layer.opacity = spec.opacity
                  end
               elseif show then
                  lowest = layer
               end
               if not show then
                  layer.visible = false
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
               water_height = a or b or -9999999
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
            do -- File list
               local files = cell:get_source_file_list()
               for j = 1, #files do
                  local file = files[j]
                  if file then
                     local name = file.filename
                     if name ~= world_first_file then
                        all_maps:accept(name, x / 32, y / 32) -- this function wants (north = up) grid coordinates, not pixel coordinates
                     end
                  end
               end
            end
            if TEST_FOR_PERSISTENT_REFS then
               --
               -- This code exists to handle a theoretical edge-case. It's not known 
               -- whether the Creation Kit would ever produce this edge-case, and the 
               -- code hasn't been useful when testing Dawnguard.esm.
               --
               -- It's possible for a file to modify a cell without actually ending up 
               -- in the cell's source file list. If the cell is an exterior cell, and 
               -- the file places a persistent reference inside of the cell, then that 
               -- persistent reference won't actually be stored as part of the cell. 
               -- Instead, the reference goes inside of what's called a "persistent 
               -- cell," which belongs to the worldspace as a whole.
               --
               -- If every normal cell is a box that encloses a small chunk of space 
               -- within the worldspace, then you can think of the "persistent cell" 
               -- as a large box that overlaps with the entire worldspace.
               --
               -- Of course, when DovahKit's backend loads a file, it "reparents" all 
               -- children of the persistent cell -- that is, it "moves" them so that 
               -- they belong to a "real" cell, based on their positions. This is the 
               -- same thing that the game does while loading.
               --
               -- As such, the only way to TRULY know what files modify a cell -- not 
               -- a cell form, but the physical region that we think of as a "cell" -- 
               -- is to check the cell's source file list, and then loop over all of 
               -- its persistent references.
               --
               local refs  = cell:get_all_persistent_refs()
               local count = #refs
               for i = 1, count do
                  local files = refs[i]:get_source_file_list()
                  for j = 1, #files do
                     local name = files[j].filename
                     if name ~= world_first_file then
                        all_maps:accept(name, x / 32, y / 32) -- this function wants (north = up) grid coordinates, not pixel coordinates
                     end
                  end
               end
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
               local shade  = math.max(0, math.min(1, (height - extents.z.min) / extents.z.span))
               shade = math.floor(shade * 255) -- TODO: round
               --
               rw.height:fill({ r = shade, g = shade, b = shade })
               rr.height:draw_raster(rw.height, x + 1, y + 1)
               --
               local color = TextureManager:get_default_color()
               color.a  = 255 -- force alpha
               color[4] = 255 -- force alpha
               rw.paint:fill(color)
               rr.paint:draw_raster(rw.paint, x + 1, y + 1)
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
         dovah.log_message("Time taken for draw: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         
         benchmark = dovah.benchmark_start()
         do -- Cell outlines
            local files = all_maps:get_filename_list()
            local count = #files
            _update_progress("Drawing outlines...", 0, count, 0)
            --
            local HUE_PER_FILE = math.ceil(360 / count)
            --
            local root_group = false
            local line_group = false
            local fill_group = false
            for i = 1, count do
               local name = files[i]
               local map  = all_maps.maps[name]
               map:set_color(string.format("hsl(%sdeg, 100%%, 50%%)", HUE_PER_FILE * i))
               map:generate_islands()
               --
               local islands = map.islands
               if #islands > 0 then
                  if not root_group then -- lazy-create these layer groups
                     root_group = canvas:append_layer_group()
                     root_group.name = "Cells by file"
                     fill_group = root_group:append_layer_group()
                     fill_group.name = "Fill"
                     line_group = root_group:append_layer_group()
                     line_group.name = "Outlines"
                     --
                     fill_group.opacity = math.max(0, math.min(1, CELL_OUTLINE_FILL_OPACITY / 255))
                  end
                  map:create_canvas_data(line_group, fill_group)
                  for j = 1, #islands do
                     local island = islands[j]
                     island:create_canvas_data()
                     island:draw()
                  end
               end
               progress.value = i
            end
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for islands: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         HeightmapWindow:import_cell_outline_data(all_maps)
         _update_progress("Done!", 0, count, count)
      end
   end
end

HeightmapWindow:show()