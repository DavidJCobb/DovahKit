
local window = ui.window.new()
local widget = ui.canvas.new()
window:set_layout("grid")
window:add_child(widget)

local l_logo  = widget:append_layer()
local l_color = widget:append_layer_group()
local l_mask  = l_color:append_layer()
local l_grad  = l_color:append_layer()
local l_text  = widget:append_layer()

l_grad.blend_mode = "multiply"
l_color.blend_mode = "multiply"

widget.width  = 300
widget.height = 300

do -- Create "logo" layer
   local data = raster.new({ width = 300, height = 300 })
   l_logo.data = data
   --
   local diameter  = 300
   local center    = 150
   local radius_sq = (diameter / 2) * (diameter / 2)
   for x = 1, 300 do
      local d = math.acos((x - 150) / 150)
      local a = 150 - math.abs(math.floor(math.sin(d) * 150))
      local b = 300 - a
      a = a + 1
      b = b - 1
      for y = a, b do
         data:set_pixel(x, y, "#FFFFFF")
      end
      if x % 50 == 0 then -- this is surprisingly slow!
         dovah.log_message("Generating logo layer... (%s%%) [y: %s - %s]", math.floor(x / 300 * 100), a, b)
      end
   end
   dovah.log_message("Generated \"logo\" layer.")
end
do -- Create "mask" layer
   local data = raster.new({ width = 300, height = 100, background_color = "#FFFFFF" })
   l_mask.data = data
   l_mask.y    = 100
   --
   dovah.log_message("Generated \"mask\" layer.")
end
do -- Create "grad" layer
   local data = raster.new({ width = 300, height = 300 })
   l_grad.data = data
   --
   for x = 1, 300 do
      local h = math.floor((x / 300) * 359)
      for y = 1, 300 do
         data:set_pixel(x, y, "hsl(" .. h .. ", 100%, 50%)")
      end
   end
   dovah.log_message("Generated \"grad\" layer.")
end
do -- Create "text" layer
   local data = ui.canvas_text_data.new()
   data.color = "#000000"
   data.text  = "Hello!"
   l_text.data = data
   l_text.x = 50
   l_text.y = 50
end

-- Controls
do
   local list = ui.widget.new()
   list:set_layout("down")
   window:add_child(list, 1, 2)
   --
   do
      local layer = l_color
      local b = ui.checkbox.new("Grad + Mask")
      b.checked = true
      b:on("OnChanged", "", function()
         layer.visible = not layer.visible
      end)
      list:add_child(b)
   end
   do
      local layer = l_grad
      local b = ui.checkbox.new("Grad")
      b.checked = true
      b:on("OnChanged", "", function()
         layer.visible = not layer.visible
      end)
      list:add_child(b)
   end
   do
      local layer = l_mask
      local b = ui.checkbox.new("Mask")
      b.checked = true
      b:on("OnChanged", "", function()
         layer.visible = not layer.visible
      end)
      list:add_child(b)
   end
   do
      local layer = l_logo
      local b = ui.checkbox.new("Logo")
      b.checked = true
      b:on("OnChanged", "", function()
         layer.visible = not layer.visible
      end)
      list:add_child(b)
   end
   do
      local layer = l_mask
      local b = ui.button.new("Mask Opacity")
      b:on("OnActivated", "", function()
         if layer.opacity < 0.6 then
            layer.opacity = 1
         else
            layer.opacity = 0.5
         end
      end)
      list:add_child(b)
   end
   do
      local layer = l_mask
      local b = ui.button.new("Mask Pos")
      b:on("OnActivated", "", function()
         local x = layer.x
         if x == 0 then
            layer.x = 150
         else
            layer.x = 0
         end
      end)
      list:add_child(b)
   end
   do
      local layer = l_mask
      local data  = layer.data
      local b = ui.button.new("Mask Data")
      b:on("OnActivated", "", function()
         if layer.data then
            layer.data = nil
         else
            layer.data = data
         end
      end)
      list:add_child(b)
   end
   do
      local layer = l_color
      local b = ui.dropdown.new()
      --
      b:append_item("Add")
      b:append_item("Burn")
      b:append_item("Darken Only")
      b:append_item("Difference")
      b:append_item("Dodge")
      b:append_item("Hard Light")
      b:append_item("Lighten Only")
      b:append_item("Multiply")
      b.selected_index = #b.items
      b:append_item("Normal")
      b:append_item("Overlay")
      b:append_item("Screen")
      b:append_item("Soft Light")
      --
      b:on("OnChanged", "", function(i)
         local item = b.items[i]
         layer.blend_mode = item.text
      end)
      list:add_child(b)
   end
end

window:show()