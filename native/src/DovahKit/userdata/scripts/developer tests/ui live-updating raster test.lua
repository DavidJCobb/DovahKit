local IMAGE_SIZE = 16

local window = ui.window.new()
local view   = ui.table_view.new()

window:set_layout("down")
window:add_child(view)

view.show_row_headers = false

local image = raster.new({
   width  = IMAGE_SIZE,
   height = IMAGE_SIZE,
   background_color = "#000"
})

view:append_row({
   icon = image,
   text = "Test"
})

do
   math.randomseed()
   --
   local button = ui.button.new("Random Pixel")
   button:on("OnActivated", "", function()
      local x = math.random(1, IMAGE_SIZE)
      local y = math.random(1, IMAGE_SIZE)
      local hue = math.random(0, 359)
      image:set_pixel(x, y, "hsl(" .. hue .. "deg, 100%, 50%)")
      --
      dovah.log_message("Setting (%d, %d) to hue %d...", x, y, hue)
   end)
   --
   window:add_child(button)
end

window:show()