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
      
      local all_maps      = RefrMap:new()
      local base_form_set = {}
      for i = 1, #base_forms do
         local form = base_forms[i]
         if form then
            base_form_set[form] = true
            all_maps:get_or_create_map(form)
         end
      end
      
      local cells   = world:get_all_cells()
      local count   = #cells
      local extents = nil
      
      _update_progress("Checking world height range...", 0, count, 0)
      do
         extents = {
            x = Range:new(),
            y = Range:new(),
            z = Range:new(),
            width  = nil,
            height = nil,
         }
         for i = 1, count do
            local cell = cells[i]
            local gc   = cell.grid_coords
            local gx   = gc.x
            local gy   = -gc.y
            extents.x:accept(gx)
            extents.y:accept(gy)
            --
            progress.value = i
         end
         if not extents.x.min then
            dovah.log_message("Unable to find any cells in this worldspace!")
            return
         end
         --
         extents.width  = (extents.x.max - extents.x.min) + 1
         extents.height = (extents.y.max - extents.y.min) + 1
         if extents.z.max then
            extents.z.span = extents.z.max - extents.z.min
            if extents.z.span == 0 then
               extents.z.span = 1
            end
         else
            extents.z.span = nil
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
            local prior = HeightmapWindow.state.layers["terrain"]
            if prior then
               HeightmapWindow.state.layers["terrain"] = nil
               canvas:remove_layer(prior)
            end
            local world_first_file = nil
            do
               local list = world:get_source_file_list()
               local file = list[1]
               if file then
                  world_first_file = file.filename
               end
            end
            --
            local name  = string.format("pre-rendered/%s-%s.png", world.editor_id, world_first_file or "nil")
            local image = dovah.package.load_file({ path = name, type = "png" })
            if image then
               local layer = canvas:append_layer()
               layer.data = image
               layer.name = "Pre-rendered terrain"
               HeightmapWindow.state.layers["terrain"] = layer
               --
               if image.width ~= IMAGE_W or image.height ~= IMAGE_H then
                  warn("World size doesn't match the prerendered terrain image!")
               end
            else
               dovah.log_message("Failed to load prerendered terrain for this worldspace from file:\n%s", name)
            end
         end
      end
      --
      _update_progress("Finding references...", 0, count, 0)
      if #base_forms > 0 then
         local benchmark = dovah.benchmark_start()
         for i = 1, count do
            local cell = cells[i]
            --
            local refs = cell:get_all_refs()
            for i = 1, #refs do
               local form = refs[i]
               if not form.disabled then
                  local base = form.base_form
                  if base_form_set[base] then
                     local p = form.position
                     local x = p.x - (extents.x.min * 4096)
                     local y = -p.y - (extents.y.min * 4096)
                     all_maps:accept(base, x, y)
                  end
               end
            end
            --
            progress.value = i
         end
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for search: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         
         benchmark = dovah.benchmark_start()
         do -- Cell outlines
            local files = all_maps:get_base_form_list()
            local count = #files
            _update_progress("Drawing outlines...", 0, count, 0)
            --
            local root_group = false
            for i = 1, count do
               local name = files[i]
               local map  = all_maps.maps[name]
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
last_rendered_maps = all_maps
         dovah.benchmark_stop(benchmark)
         dovah.log_message("Time taken for islands: %s milliseconds (%s microseconds)", benchmark:milliseconds(), benchmark:microseconds())
         HeightmapWindow:import_cell_outline_data(all_maps)
         _update_progress("Done!", 0, count, count)
      end
   end
end

HeightmapWindow:show()