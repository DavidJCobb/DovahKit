local window = ui.window.new()
local scroll = ui.scrollbox.new()
local canvas = ui.canvas.new()
canvas.width  = 3800
canvas.height = 3000
window:set_layout("down")
window:add_child(scroll)
window.max_width  = 1000
window.max_height =  800
do
   local body = scroll.body
   body:set_layout("down")
   body:add_child(canvas)
end

local layer = canvas:append_layer()
local image = raster.new({ width = 3800, height = 3000 })
layer.data = image

window:show()

dovah.log_message("Prepping gradient...")
image:draw_rect({
   from = {    1,    1 },
   to   = { 3800, 3000 },
   fill_gradient = {
      type  = "linear",
      angle = 90,
      stops = {
         { 0, "#FF0000FF" },
         { 1, "#FF000000" },
      },
   },
})
dovah.log_message("Gradient ready...")

dovah.log_message("Doing RGB-only fill...")
local bench = dovah.benchmark_start()
image:fill_rgb("#0080FF")
dovah.benchmark_stop(bench)
dovah.log_message("Fill done. Time: %s ms (%s us)", bench:milliseconds(), bench:microseconds())