
local window = ui.window.new()
local tabbox = ui.tabbox.new()

local errors = {}
function render(ext)
   local canvas = ui.canvas.new()
   do
      local layer  = canvas:append_layer()
      local raster = dovah.package.load_file("YOUR BURDENS." .. ext)
      if not raster then
         error("dovah.package.load_file produced no result")
      end
      local t = dovah.type(raster)
      if t ~= "raster" then
         error("dovah.package.load_file produced an unexpected type: " .. tostring(t))
      end
      --
      layer.data = raster
      --
      canvas.width  = raster.width
      canvas.height = raster.height
   end
   --
   local tab = tabbox:add_tab()
   tab.tab_name = ext
   tab:set_layout("grid")
   tab:add_child(canvas)
end

window.title = "Package test - load files"
window:set_layout("down")
window:add_child(tabbox)

do
   local button = ui.button.new("Test binary")
   window:add_child(button)
   button:on("OnActivated", "jpeg skim", function()
      local res = dovah.package.load_file({ path = "YOUR BURDENS.jpg", type = "binary" })
      if not res then
         error("binary test: dovah.package.load_file produced no result")
      end
      local t = dovah.type(res)
      if t ~= "binary_view" then
         error("binary test: dovah.package.load_file produced an unexpected type: " .. tostring(t))
      end
      --
      local result, offset, message, detail = jpeg_skim(res)
      dovah.log_message("JPEG Skim result: %s", result)
      if not result then
         dovah.log_message("%s at %s\n - %s", message, offset, detail)
      end
   end)
end

pcall(render, "bmp")
pcall(render, "jpg")
pcall(render, "png")

window:show()