local item = nil
do
   local dropdown = ui.dropdown.new()
   dropdown:append_item("Test!")
   item = dropdown.items[1]
end
collectgarbage("collect")
collectgarbage("collect") -- has to be twice

--
-- The parent has gone out of scope, and we have forced its userdata 
-- to be garbage-collected.
--

local window = ui.window.new()
local button = ui.button.new("Check child")
local bclear = ui.button.new("Clear child")
window:set_layout("grid")
window:add_child(button)
window:add_child(bclear)

window.name = "Visible window"
button.name = "Visible button"
bclear.name = "Clear button"

button:on("OnActivated", "", function()
   --
   -- If we have failed to properly manage widget lifetimes, then the 
   -- model item will have been deleted by the time this runs, with 
   -- the Lua VM left holding a dangling pointer; that should cause 
   -- this to crash.
   --
   -- If, on the other hand, we did our job, then the model item should 
   -- still exist and this should be perfectly safe.
   --
   dovah.log_message(item.text)
end)
bclear:on("OnActivated", "", function()
   item = nil
   collectgarbage("collect")
   collectgarbage("collect") -- has to be twice
   --
   -- Should result in the widget's deletion.
   --
end)

window:show()