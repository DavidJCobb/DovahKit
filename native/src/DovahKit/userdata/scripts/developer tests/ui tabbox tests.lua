local window = ui.window.new()
local tabbox = ui.tabbox.new()

window:set_layout("grid")
window:add_child(tabbox)

do
   local tab = tabbox:add_tab("First tab")
   tab:set_layout("grid")
   tab:add_child(ui.text.new("Hello, world!"))
end
do
   local tab = tabbox:add_tab("Second tab")
   tab:set_layout("grid")
   tab:add_child(ui.text.new("G'day, world!"))
end

local readout = ui.text.new("...")
window:add_child(readout, 2, 1)

tabbox:on("OnSelectionChanged", "readout", function(tab)
   dovah.log_message(tab)
   readout.text = tab.tab_name
   tab.tab_name = tab.tab_name .. "x"
end)

window:show()