
local layer = nil
do
   local canvas = ui.canvas.new()
   layer = canvas:append_layer()
end
collectgarbage("collect")
collectgarbage("collect")

dovah.log_message(layer)
dovah.log_message(layer.visible)
dovah.log_message(layer.blend_mode)
dovah.log_message(layer.canvas)