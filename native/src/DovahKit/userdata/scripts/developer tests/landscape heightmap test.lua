
local world = dovah.get_form_by_id(0x3C)
if not world then
   error("No worldspace to work with!")
end

--
-- NOTE: Bethesda's coordinate system points northeast. Southwest is 
-- negative. Given a top-down view with north matching up, this would 
-- correspond to a vertical flip.
--
function do_grid_flip(x, y)
   return x, (33 - y + 1)
end

local bounds = {}
do
   local b  = world.bounds
   local b1 = b.min
   local b2 = b.max
   bounds = {
      x = {
         min = math.floor(b.min.x / 4096),
         max = math.ceil (b.max.x / 4096),
      },
      y = {
         min = math.floor(b.min.y / 4096),
         max = math.ceil (b.max.y / 4096),
      }
   }
   bounds.width  = bounds.x.max - bounds.x.min
   bounds.height = bounds.y.max - bounds.y.min
end

local cells = world:get_all_cells()
local count = #cells

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

progress.format  = "Preparing canvas..."
progress.minimum = 0
progress.maximum = 0
progress.value   = 0

local IMAGE_W = 32 * bounds.width
local IMAGE_H = 32 * bounds.height
canvas.width  = IMAGE_W
canvas.height = IMAGE_H
dovah.log_message("Canvas size: %dx%dpx", IMAGE_W, IMAGE_H)
local base_raster = raster.new({
   width  = IMAGE_W,
   height = IMAGE_H,
   background_color = "#000000",
})
local base_layer = canvas:append_layer()
base_layer.data = base_raster

progress.format  = "Checking world height range..."
progress.minimum = 0
progress.maximum = count
progress.value   = 0

local world_min_height = nil
local world_max_height = nil
for i = 1, count do
   local cell = cells[i]
   local land = cell.landscape
   if land then
      local min = land:get_minimum_height()
      local max = land:get_maximum_height()
      if not world_min_height or min < world_min_height then
         world_min_height = min
      end
      if not world_max_height or max > world_max_height then
         world_max_height = max
      end
   end
   progress.value = i
end

if not (world_min_height and world_max_height) then
   error("Failed to identify world height range")
end
local world_height_span = world_max_height - world_min_height
dovah.log_message("Terrain height spans the range of [%s, %s], distance %s.", world_min_height, world_max_height, world_height_span)
if world_height_span < 0 then
   error(string.format("World height span is a negative number (%s).", world_height_span))
end
if world_height_span == 0 then
   world_height_span = 1
end

--progress.format  = "Drawing cells: %p% (%v/%m)..." -- perf impact?
progress.format  = "Drawing cells..."
progress.minimum = 0
progress.maximum = count
progress.value   = 0

for i = 1, count do
   local cell = cells[i]
   local land = cell.landscape
   if land then
      local gc = cell.grid_coords
      local x  = gc.x - bounds.x.min
      local y  = gc.y - bounds.y.min
      y = bounds.height - y - 1
      x = x * 32
      y = y * 32
      --
      for u = 2, 33 do -- leftmost col overlaps with western cell, so skip it
         for v = 1, 32 do -- bottom row overlaps with southern cell, so skip it
            local height = land:get_height_at(u, v)
            local shade  = (height - world_min_height) / world_height_span
            shade = math.floor(shade * 255) -- TODO: round
            --
            u, v = do_grid_flip(u, v)
            base_raster:set_pixel(x + u, y + v, { r = shade, g = shade, b = shade })
         end
      end
   end
   progress.value = i
end