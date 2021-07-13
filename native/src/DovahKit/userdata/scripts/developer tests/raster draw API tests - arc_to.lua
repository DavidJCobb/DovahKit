
local DO_HFLIP = false
local DO_VFLIP = false
local ROTATION = 25

local window = ui.window.new()
window:set_layout("ltr")

function draw_point(raster, point, color)
   raster:draw_ellipse({
      line_color = color,
      line_width = 3,
      center = point,
      radius = 1.5
   })
end

function hflip(v, about)
   v.x = 300 - v.x
end
function vflip(v, about)
   v.y = 300 - v.y
end

function rotate_about(v, about, deg)
   local w = v - about
   w = w:rotate(deg)
   w = w + about
   --
   v.x = w.x
   v.y = w.y
   if DO_HFLIP then
      hflip(v)
   end
   if DO_VFLIP then
      vflip(v)
   end
end

do -- arcTo test (contained angle)
   local widget = ui.canvas.new()
   window:add_child(widget)

   widget.width  = 300
   widget.height = 300
   
   local raster = raster.new({ width = 300, height = 300 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:fill("#FFF")
   
   local center  = vector2.new(150, 150)
   local point_a = vector2.new(230,  95)
   local point_b = vector2.new( 90, 205)
   local point_c = vector2.new( 20,  95)
   local radius  = 50
   
   local diag_center    = vector2.new(102.5, 131.5)
   local diag_tangent_1 = vector2.new(133.4, 170.8)
   local diag_tangent_2 = vector2.new( 60.3, 158.4)
   
   rotate_about(point_a,        center, ROTATION)
   rotate_about(point_b,        center, ROTATION)
   rotate_about(point_c,        center, ROTATION)
   rotate_about(diag_tangent_1, center, ROTATION)
   rotate_about(diag_tangent_2, center, ROTATION)
   rotate_about(diag_center,    center, ROTATION)
   
   do -- diagnostic
      local path = raster_draw_path.new()
      path:move_to(point_a.x, point_a.y)
      path:line_to(point_c.x, point_c.y)
      raster:draw_path({
         path       = path,
         line_width = 1,
         line_color = "#800000A0"
      })
      draw_point(raster, point_b, "#000000") -- b
      --
      -- Values seen in Visual Studio debugger:
      --
      draw_point(raster, diag_tangent_1, "#8000FF") -- tan1
      draw_point(raster, diag_tangent_2, "#808000") -- tan2
      draw_point(raster, diag_center,    "#80C0A0") -- center
      raster:draw_ellipse({
         fill_color = "#AAAA0060",
         radius = 50,
         center = diag_center,
      })
      raster:draw_rect({
         fill_color = "#AAAA0020",
         from = diag_center - vector2.new(radius, radius),
         w = radius * 2,
         h = radius * 2,
      })
   end
   
   local path = raster_draw_path.new()
   path:move_to(point_a)
   path:arc_to (point_b, point_c, radius)
   path:line_to(point_c)
   
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
   widget.height = 300
   
   local raster = raster.new({ width = 300, height = 300 })
   local layer  = widget:append_layer()
   layer.data = raster
   
   raster:fill("#FFF")
   
   local center  = vector2.new(150, 150)
   local point_a = vector2.new(180, 165)
   local point_b = vector2.new(180, 205)
   local point_c = vector2.new(110, 205)
   local radius  = 130
   
   rotate_about(point_a, center, ROTATION)
   rotate_about(point_b, center, ROTATION)
   rotate_about(point_c, center, ROTATION)
   
   do -- diagnostic
      local path = raster_draw_path.new()
      path:move_to(point_a.x, point_a.y)
      path:line_to(point_c.x, point_c.y)
      raster:draw_path({
         path       = path,
         line_width = 1,
         line_color = "#800000A0"
      })
   end
   
   local path = raster_draw_path.new()
   path:move_to(point_a)
   path:arc_to (point_b, point_c, radius)
   path:line_to(point_c)
   
   raster:draw_path({
      path       = path,
      line_width = 1,
      line_color = "#000000FF"
   })
end

window:show()