
local TRANSPARENT = "#00000000"
local WATER_DEPTH = 4096
local WATER_MURK  = 1

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
local scroll   = ui.scrollbox.new()
local canvas   = ui.canvas.new()
local progress = ui.progress_bar.new()
progress.alignment = "center center"
window.title = string.format("Heightmap for %s", world.name)
do
   window:set_layout("grid")
   window:add_child(scroll)
   local sb = scroll.body
   sb:set_layout("grid")
   sb:add_child(canvas)
   window:add_child(progress, 2, 1)
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

local IMAGE_W = 32 * extents.width
local IMAGE_H = 32 * extents.height
canvas.width  = IMAGE_W
canvas.height = IMAGE_H
dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
local base_raster
local base_water_raster
do
   base_raster = raster.new({
      width  = IMAGE_W,
      height = IMAGE_H,
      background_color = "#000000",
   })
   local base_layer = canvas:append_layer()
   base_layer.data = base_raster
   --
   base_water_raster = raster.new({
      width  = IMAGE_W,
      height = IMAGE_H,
      background_color = TRANSPARENT,
   })
   local water_layer = canvas:append_layer()
   water_layer.data = base_water_raster
end


progress.format  = "Drawing cells..."
progress.minimum = 0
progress.maximum = count
progress.value   = 0

local working_raster = raster.new({ width = 32, height = 32 })
local working_water  = raster.new({ width = 32, height = 32 })

for i = 1, count do
   local cell = cells[i]
   --
   local water_height = nil
   do
      local a = cell.water_height
      local b = world_info.heights.water
      if a then
         water_height = math.max(a, b)
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
      working_water:fill(TRANSPARENT)
      --
      for u = 2, 33 do -- leftmost col overlaps with western cell, so skip it
         for v = 1, 32 do -- bottom row overlaps with southern cell, so skip it
            local height = land:get_height_at(u, v)
            local shade  = (height - extents.z.min) / extents.z.span
            shade = math.floor(shade * 255) -- TODO: round
            --
            local a, b = do_grid_flip(u, v)
            a = a - 1 -- account for skipped col
            b = b - 1 -- account for skipped row
            working_raster:set_pixel(a, b, { r = shade, g = shade, b = shade })
            --
            do -- water
               local diff = water_height - height
               if diff > 0 then
                  local opacity = math.ceil(math.min(1, diff / WATER_DEPTH) * 255)
                  working_water:set_pixel(a, b, { r = 80, g = 160, b = 255, a = opacity })
               end
            end
         end
      end
      --
      base_raster:draw_raster(working_raster, x + 1, y + 1)
      base_raster:draw_raster(working_water,  x + 1, y + 1)
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
      working_raster:fill({ r = shade, g = shade, b = shade })
      --
      base_raster:draw_raster(working_raster, x + 1, y + 1)
      --
      if height < water_height then -- water
         local diff = water_height - height
         local opacity = math.ceil(math.min(1, diff / WATER_DEPTH) * 255)
         working_water:fill({ r = 80, g = 160, b = 255, a = opacity })
         base_raster:draw_raster(working_water,   x + 1, y + 1)
      end
   end
   progress.value = i
end