
local window = ui.window.new()
local widget = ui.canvas.new()
window:set_layout("grid")
window:add_child(widget)

widget.width  = 128
widget.height = 128

do
   local raster = raster.new({ width = 128, height = 128 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:draw_rect({
      x = 8,
      y = 8,
      w = 112,
      height = 112,
      fill_color = "#F00"
   })
   raster:draw_rect({
      from = { 20, 20 },
      to   = { x = 108, y = 108 },
      line_color = "#FFF",
      line_width = 8,
   })
   
   raster:draw_rect({
      from = { 0, 32 },
      w = 128,
      h =  64,
      fill_gradient = {
         type    = "linear",
         angle   = 45,
         stretch = true,
         stops   = {
            { 0.000, "hsl(  0deg, 50%, 50%)" },
            { 0.125, "hsl( 45deg, 50%, 50%)" },
            { 0.250, "hsl( 90deg, 50%, 50%)" },
            { 0.375, "hsl(135deg, 50%, 50%)" },
            { 0.500, "hsl(180deg, 50%, 50%)" },
            { 0.625, "hsl(225deg, 50%, 50%)" },
            { 0.750, "hsl(270deg, 50%, 50%)" },
            { 0.875, "hsl(315deg, 50%, 50%)" },
            { 1.000, "hsl(360deg, 50%, 50%)" },
         }
      }
   })
   
   raster:draw_line({
      from = { 16, 16 },
      to   = { 80, 80 },
      line_color = "#0FF",
      line_width = 3
   })
end

window:show()