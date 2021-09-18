local widget = ui.widget.new()

local window = ui.window.new()
local button = ui.button.new("collect")
button:on("OnActivated", "", function()
   collectgarbage("collect")
   collectgarbage("collect")
   dovah.log_message("GC'd")
end)

window:set_layout("grid")
window:add_child(button)
window:show()

widget = nil