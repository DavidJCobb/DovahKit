if not Window then
   error("missing dependency")
end

local CELL_SIZE_IN_WORLD_UNITS = 4096

function is_navmesh_in_world(navmesh, world)
   local cell = navmesh.parent_cell
   if not cell then
      return false
   end
   return cell.parent_world == world
end

-- debugging
LAST_LONGEST_PATH = nil
LAST_WORLD_BOUNDS = nil

-- pp_raster-relative positions
navmesh_position_cache = {}

function render()
   local cell_size_in_pixels = 32
   
   local worldspace = dovah.get_form_by_id(0x3C)
   local navi       = nil
   dovah.for_each_form_of_type(form_types.navmesh_info_map, function(form)
      navi = form
      return true
   end)
   if not navi then
      return
   end
   
   local canvas   = Window.controls.canvas
   local progress = Window.controls.progress
   function _update_progress(text, min, max, value)
      local p = Window.controls.progress
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
   
   local cells   = worldspace:get_all_cells()
   local count   = #cells
   local extents = nil
   _update_progress("Finding worldspace bounds (checked %v/%m cells)...", 0, count, 0)
   do
      extents = {
         x = Range:new(),
         y = Range:new(),
         width  = nil,
         height = nil,
      }
      --
      for i = 1, count do
         local cell = cells[i]
         local gc = cell.grid_coords
         local gx = gc.x
         local gy = -gc.y
         extents.x:accept(gx)
         extents.y:accept(gy)
         --
         progress.value = i
      end
      --
      if extents.x:empty() then
         dovah.log_message("Unable to find any cells in this worldspace!")
         return
      end
      --
      extents.width  = (extents.x.max - extents.x.min) + 1
      extents.height = (extents.y.max - extents.y.min) + 1
   end
   LAST_WORLD_BOUNDS = extents
   
   local image_w = extents.width  * cell_size_in_pixels
   local image_h = extents.height * cell_size_in_pixels
   
   local pp_raster = raster.new({
      width  = image_w,
      height = image_h
   })
   canvas.width  = image_w
   canvas.height = image_h
   
   do
      local image = dovah.package.load_file({ path = "pre-rendered/Tamriel-Skyrim.esm.png", type = "png" })
      if image then
         local prerender = canvas:append_layer()
         prerender.data = image
         prerender.name = "Pre-rendered terrain"
      end
   end
   local layer = canvas:append_layer()
   layer.data = pp_raster
   
   local navi_data = navi:debug_copy_as_table()
   
   function map_world_position_to_canvas(pos)
      pos.y = -pos.y
      pos:div(CELL_SIZE_IN_WORLD_UNITS)
      pos:sub({ x = extents.x.min, y = extents.y.min })
      pos:mul(cell_size_in_pixels)
   end
   
   -- pp_raster-relative positions
   --local navmesh_position_cache = {}
   function get_navmesh_position(navmesh)
      local pos = navmesh_position_cache[navmesh]
      if pos then
         return pos
      end
      do
         local info = navi_data.navmesh_infos[navmesh]
         if info then
            pos = vector2.new(info.approx_location.x, info.approx_location.y)
         end
      end
      if not pos then
         local centroid = vector2.new()
         centroid = vector2.new(navmesh.approx_location.x, navmesh.approx_location.y)
         local list = navmesh.vertices
         local size = #list
         for i = 1, size do
            local vert = list[i]
            centroid:add(vector2.new(vert.x, vert.y))
         end
         if size > 0 then
            centroid:div(size)
         end
         pos = centroid
      end
      map_world_position_to_canvas(pos)
      navmesh_position_cache[navmesh] = pos
      return pos
   end
   
   do
      local list = navi_data.precomputed_paths
      local size = #list
      _update_progress("Drawing paths (%v/%m)...", 0, size, 0)
      
      local count_drawn = 0
      local longest     = 0
      
      for i = 1, size do
         local path  = list[i]
         local plen  = #path
         
         local relevant = false
         if plen > 0 then
            relevant = is_navmesh_in_world(path[1], worldspace)
         end
         if relevant then
            local draw  = raster_draw_path.new()
            local first = true
            for j = 1, plen do
               local navmesh = path[j]
               local point   = get_navmesh_position(navmesh)
               if first then
                  draw:move_to(point)
                  first = false
               else
                  draw:line_to(point)
               end
            end
            if plen > longest then
               longest = plen
               LAST_LONGEST_PATH = path
            end
            pp_raster:draw_path({
               fill_color = "#00000000",
               line_color = "#F00",
               path       = draw,
            })
            count_drawn = count_drawn + 1
         end
         
         progress.value = i
      end
      dovah.log_message("Drew %d paths. The longest had %d nodes.", count_drawn, longest)
   end
   
   -- Draw all road markers
   do
      local RoadMarker = dovah.get_form_by_id(0xF077B) -- hardcoded; TODO: find it via DOBJ?
      if RoadMarker then
         local rm_raster = nil
         
         local list = RoadMarker:get_user_forms()
         local size = #list
         _update_progress("Checking RoadMarker uses (%v/%m)...", 0, size, 0)
         for i = 1, size do
            local user = list[i]
            if  user.form_type == form_types.reference
            and user.base_form == RoadMarker
            then
               local pos = user.position
               pos = vector2.new(pos.x, pos.y)
               map_world_position_to_canvas(pos)
               
               if not rm_raster then
                  rm_raster = raster.new({
                     width  = image_w,
                     height = image_h
                  })
                  canvas:append_layer().data = rm_raster
               end
               
               local draw
               --
               draw = raster_draw_path.new()
               draw:add_ellipse({
                  center = pos,
                  radius = 7,
               })
               rm_raster:draw_path({ path = draw, fill_color = "#000" })
               --
               draw = raster_draw_path.new()
               draw:add_ellipse({
                  center = pos,
                  radius = 5,
               })
               rm_raster:draw_path({ path = draw, fill_color = "#806020" })
            end
            progress.value = i
         end
      end
   end
   
   _update_progress("Done!", 0, 100, 100)
   layer.data = pp_raster
end
Window.controls.buttons.execute:on("OnActivated", "", function()
   Window.controls.config.enabled = false
   render()
   Window.controls.config.enabled = true
end)

Window:show()