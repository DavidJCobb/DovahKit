if not HeightmapWindow then
   error("missing dependency")
end

local CELL_SIDE_SIZE = 4096
local TRANSPARENT    = "#00000000"

-- Comprehensive but slow; haven't yet found cases where it's actually needed
local TEST_FOR_PERSISTENT_REFS = false

--

render_refr_locations = nil
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
   
   render_refr_locations = function(world, base_forms)
      local world_info = nil
      do
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
      local all_maps = CellOutlineMapAllFiles:new()
      for i = 1, #base_forms do
         all_maps:get_or_create_map(base_forms[i])
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
         
         function _do_grid_flip(x, y)
            return x, (33 - y + 1)
         end
         
         local benchmark = dovah.benchmark_start()
         for i = 1, count do
            local cell = cells[i]
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
            local refs = cell:get_all_refs()
            for i = 1, #refs do
               local form = refs[i]
               if not form.disabled then
                  local base = form.base_form
                  if false then -- if the base form is of interest
                     local p = form.position
                     local y = -p.y - (extents.y.min * 4096)
                     all_maps:accept(base, p.x, y)
                  end
               end
            end
            --
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for draw: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         
         benchmark = dovah.benchmark_start()
         do -- Cell outlines
            local files = all_maps:get_base_form_list()
            local count = #files
            _update_progress("Drawing outlines...", 0, count, 0)
            --
            local HUE_PER_FILE = math.ceil(360 / count)
            --
            local root_group = false
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
                     root_group.name = "Ref locations"
                  end
                  map:create_canvas_data(root_group)
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