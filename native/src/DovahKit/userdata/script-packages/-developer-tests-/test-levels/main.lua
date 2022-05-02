
local window = ui.window.new()
local tabbox = ui.tabbox.new()

function show(image, name)
   local canvas = ui.canvas.new()
   do
      local layer = canvas:append_layer()
      layer.data  = image
      --
      canvas.width  = image.width
      canvas.height = image.height
   end
   --
   local tab    = tabbox:add_tab()
   local scroll = ui.scrollbox.new()
   tab.tab_name = name
   tab:set_layout("grid")
   tab:add_child(scroll)
   scroll.body:set_layout("grid")
   scroll.body:add_child(canvas)
end

window.title = "Package test - levels"
window:set_layout("down")
window:add_child(tabbox)

local moon = dovah.package.load_file("Moon_Farside_LRO.jpg")
local edit = dovah.package.load_file("Moon_Farside_LRO.jpg")
edit:levels({
   from  = { 0, 120 },
   to    = { min = 0, max = 255 },
   gamma = 0.4,
})

show(moon, "Before")
show(edit, "After")

window:show()