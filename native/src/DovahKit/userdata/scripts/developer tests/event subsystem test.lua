local window = ui.window.new()
local button = ui.button.new("I have been clicked 0 times")

window:set_layout("grid")
window:add_child(button)

local count = 0
button:on("OnActivated", "", function()
   count = count + 1
   button.text = "I have been clicked " .. count .. " times"
end)

window:show()