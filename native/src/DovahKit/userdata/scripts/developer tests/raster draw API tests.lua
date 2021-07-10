
local window = ui.window.new()
local widget = ui.canvas.new()
window:set_layout("ltr")
window:add_child(widget)

widget.width  = 128
widget.height = 128

function draw_point(raster, x, y, color)
   raster:draw_ellipse({
      line_color = color,
      line_width = 3,
      x = x,
      y = y,
      radius = 1.5
   })
end

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
      line_join  = "round",
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
            { 0.000, "hsl(  0deg, 100%, 50%)" },
            { 0.125, "hsl( 45deg, 100%, 50%)" },
            { 0.250, "hsl( 90deg, 100%, 50%)" },
            { 0.375, "hsl(135deg, 100%, 50%)" },
            { 0.500, "hsl(180deg, 100%, 50%)" },
            { 0.625, "hsl(225deg, 100%, 50%)" },
            { 0.750, "hsl(270deg, 100%, 50%)" },
            { 0.875, "hsl(315deg, 100%, 50%)" },
            { 1.000, "hsl(360deg, 100%, 50%)" },
         }
      }
   })
   
   raster:draw_ellipse({
      center = { 64, 64 },
      angle  = 135,
      radii  = { 20, 40 },
      line_width = 2,
      line_color = "#F00",
   })
   
   raster:draw_line({
      from = { 16, 16 },
      to   = { 80, 80 },
      line_color = "#0FF",
      line_width = 3
   })
   
   raster:scale(2.0, 3.0)
   raster:flip("v")
   
   widget.width  = raster.width
   widget.height = raster.height
end

do
   local widget = ui.canvas.new()
   window:add_child(widget)

   widget.width  = 128
   widget.height = 128
   
   local raster = raster.new({ width = 128, height = 128 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:fill("#202020")
   
   local path = raster_draw_path.new()
   path:move_to( 8,   8)
   path:line_to(16,  16)
   path:line_to(16,  64)
   path:line_to(64,  64)
   path:line_to( 0, 128)
   path:line_to( 8,   8)
   
   raster:draw_path({
      path       = path,
      line_width = 5,
      line_color = "#FF0000FF",
      fill_color = "#FF000060"
   })
end

do -- arcTo test (contained angle)
   local widget = ui.canvas.new()
   window:add_child(widget)

   widget.width  = 300
   widget.height = 150
   
   local raster = raster.new({ width = 300, height = 150 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:fill("#FFF")
   
   do -- diagnostic
      local path = raster_draw_path.new()
      path:move_to(230, 20)
      path:line_to( 20, 20)
      raster:draw_path({
         path       = path,
         line_width = 1,
         line_color = "#800000A0"
      })
      draw_point(raster, 90, 130, "#000000") -- b
      --
      -- Values seen in Visual Studio debugger:
      --
      draw_point(raster, 133.4,  95.8, "#8000FF") -- tan1
      draw_point(raster,  60.3,  83.4, "#808000") -- tan2
      draw_point(raster, 102.5,  56.5, "#80C0A0") -- center
      raster:draw_ellipse({
         fill_color = "#AAAA0060",
         radius = 50,
         x = 102.5,
         y =  56.5,
      })
      raster:draw_rect({
         fill_color = "#AAAA0020",
         from = { 52.5, 6.5 },
         w = 100,
         h = 100,
      })
   end
   
   local path = raster_draw_path.new()
   path:move_to(230, 20)
   path:arc_to({ 90, 130 }, { 20, 20 }, 50)
   path:line_to( 20, 20)
   
   raster:draw_path({
      path       = path,
      line_width = 1,
      line_color = "#000000FF"
   })
end
do -- arcTo test (excess angle)
   local widget = ui.canvas.new()
   window:add_child(widget)

   widget.width  = 300
   widget.height = 150
   
   local raster = raster.new({ width = 300, height = 150 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:fill("#FFF")
   
   do -- diagnostic
      local path = raster_draw_path.new()
      path:move_to(180, 90)
      path:line_to(110, 130)
      raster:draw_path({
         path       = path,
         line_width = 1,
         line_color = "#800000A0"
      })
   end
   
   local path = raster_draw_path.new()
   path:move_to(180, 90)
   path:arc_to({ 180, 130 }, { 110, 130 }, 130)
      -- tan_u:  180, 260
      -- tan_v:  310, 130
      -- center: 310, 260
   path:line_to(110, 130)
   
   raster:draw_path({
      path       = path,
      line_width = 1,
      line_color = "#000000FF"
   })
end

window:show()