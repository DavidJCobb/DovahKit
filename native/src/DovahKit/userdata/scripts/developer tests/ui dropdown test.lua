local window = ui.window.new()
local picker = ui.dropdown.new()
local button = ui.button.new("Toggle Sort")

local readout = ui.text.new()

picker:append_item("Ari (1)")
picker:append_item("Chris (2)")
picker:append_item("Brianna (3)")
picker:append_item("Quigley (4)")
picker:append_item("Lucrezia (5)")

window:set_layout("grid")
window:add_child(picker, 1, 1)
window:add_child(button, 1, 2)
window:add_child(readout, 2, 1, 1, 2)

button:on("OnActivated", "", function()
   picker.sorted = not picker.sorted
end)

--
-- Unit test: Lua should always work with logical indices, not visible 
-- indices. This means that even if you enable sorting, such that Quigley 
-- becomes the fifth item visible in the dropdown, picking him should have 
-- Lua detect him as selected index 4 of 5. Event listeners that receive 
-- combobox indices should behave similarly.
--
picker:on("OnChanged", "readout", function(index)
   readout.text = "E: " .. tostring(index) .. " / W: " .. tostring(picker.selected_index)
end)

window:show()